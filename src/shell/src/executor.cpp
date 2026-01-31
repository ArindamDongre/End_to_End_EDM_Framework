#include "../include/executor.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <cstring> // for std::strcmp
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Job {
    int id = 0;
    pid_t pgid = 0;
    std::vector<pid_t> pids;
    std::string command;
    enum class State { Running, Stopped } state = State::Running;
    bool background = false;
};

std::vector<Job> jobs;
int next_job_id = 1;
pid_t shell_pgid = 0;
int shell_terminal = -1;
bool shell_interactive = false;

std::vector<char *> build_argv(const SimpleCommand &cmd) {
    std::vector<char *> argv;
    argv.reserve(cmd.argv.size() + 1);
    for (const auto &arg : cmd.argv) {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);
    return argv;
}

bool setup_redirections(const SimpleCommand &cmd) {
    if (!cmd.input_redirection.empty()) {
        int fd = open(cmd.input_redirection.c_str(), O_RDONLY);
        if (fd < 0) {
            perror(cmd.input_redirection.c_str());
            return false;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2");
            close(fd);
            return false;
        }
        close(fd);
    }

    if (!cmd.output_redirection.empty()) {
        int fd = open(cmd.output_redirection.c_str(),
                      O_WRONLY | O_CREAT | O_TRUNC,
                      0666);
        if (fd < 0) {
            perror(cmd.output_redirection.c_str());
            return false;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            close(fd);
            return false;
        }
        close(fd);
    }

    return true;
}

void prepare_child_signals() {
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
}

void run_child(const SimpleCommand &cmd,
               int inherit_stdin_fd,
               int inherit_stdout_fd) {
    if (inherit_stdin_fd >= 0) {
        if (dup2(inherit_stdin_fd, STDIN_FILENO) < 0) {
            perror("dup2");
            _exit(126);
        }
        close(inherit_stdin_fd);
    }

    if (inherit_stdout_fd >= 0) {
        if (dup2(inherit_stdout_fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            _exit(126);
        }
        close(inherit_stdout_fd);
    }

    if (!setup_redirections(cmd)) {
        _exit(126);
    }

    auto argv = build_argv(cmd);
    if (argv[0] != nullptr) {
        // Handle some builtins specially when they appear in pipelines/background.
        // They should NOT affect the parent shell, just behave like commands.

        if (std::strcmp(argv[0], "cd") == 0) {
            // In a pipeline: behave like a command that does nothing and exits.
            // This avoids execvp("cd") failure and matches the observable behavior:
            //   `cd .. | ls` -> only `ls` matters; shell cwd doesn't change.
            _exit(0);
        }

        if (std::strcmp(argv[0], "exit") == 0) {
            // In a pipeline: just terminate this child, do NOT exit the shell.
            // `exit | ls` -> acts like a process that immediately exits,
            // `ls` still runs.
            _exit(0);
        }
    }

    execvp(argv[0], argv.data());
    perror("execvp");
    _exit(127);
}

void terminate_group(pid_t pgid) {
    if (pgid <= 0) {
        return;
    }
    kill(-pgid, SIGTERM);
    int status = 0;
    while (waitpid(-pgid, &status, WNOHANG) > 0) {
    }
}

Job *find_job_by_id(int id) {
    for (auto &job : jobs) {
        if (job.id == id) {
            return &job;
        }
    }
    return nullptr;
}

Job *find_job_by_pid(pid_t pid) {
    for (auto &job : jobs) {
        if (std::find(job.pids.begin(), job.pids.end(), pid) != job.pids.end()) {
            return &job;
        }
    }
    return nullptr;
}

void remove_job(Job &job) {
    jobs.erase(std::remove_if(jobs.begin(),
                              jobs.end(),
                              [&](const Job &j) { return j.id == job.id; }),
               jobs.end());
}

std::string describe_command(const Command &cmd) {
    if (!cmd.original_line.empty()) {
        return cmd.original_line;
    }
    std::string combined;
    for (size_t i = 0; i < cmd.commands.size(); ++i) {
        if (i) {
            combined += " | ";
        }
        if (!cmd.commands[i].argv.empty()) {
            combined += cmd.commands[i].argv[0];
        }
    }
    return combined;
}

void handle_child_status(pid_t pid, int status) {
    Job *job = find_job_by_pid(pid);
    if (!job) {
        return;
    }

    if (WIFSTOPPED(status)) {
        job->state = Job::State::Stopped;
        job->background = false;
    } else if (WIFCONTINUED(status)) {
        job->state = Job::State::Running;
    } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        auto &pids = job->pids;
        pids.erase(std::remove(pids.begin(), pids.end(), pid), pids.end());
        if (pids.empty()) {
            if (job->background) {
                std::cout << "[" << job->id << "] Done " << job->command << "\n";
            }
            remove_job(*job);
        }
    }
}

void give_terminal_to(pid_t pgid) {
    if (shell_interactive) {
        tcsetpgrp(shell_terminal, pgid);
    }
}

void wait_for_job(int job_id) {
    Job *job = find_job_by_id(job_id);
    if (!job) {
        return;
    }

    while (true) {
        int status = 0;
        pid_t pid = waitpid(-job->pgid, &status, WUNTRACED);
        if (pid < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        handle_child_status(pid, status);

        Job *current = find_job_by_id(job_id);
        if (!current) {
            break;
        }
        if (current->state == Job::State::Stopped) {
            std::cout << "\n[" << current->id << "] Stopped " << current->command << "\n";
            break;
        }
    }
}

struct SpawnResult {
    pid_t pgid = 0;
    std::vector<pid_t> pids;
};

bool spawn_pipeline(const std::vector<SimpleCommand> &commands, SpawnResult &result) {
    result = SpawnResult{};
    int prev_read_fd = -1;
    pid_t pgid = 0;

    for (size_t i = 0; i < commands.size(); ++i) {
        int pipefd[2] = {-1, -1};
        const bool need_pipe = (i + 1 < commands.size());
        if (need_pipe && pipe(pipefd) < 0) {
            perror("pipe");
            if (prev_read_fd >= 0) {
                close(prev_read_fd);
            }
            terminate_group(pgid);
            return false;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            if (pipefd[0] >= 0) {
                close(pipefd[0]);
            }
            if (pipefd[1] >= 0) {
                close(pipefd[1]);
            }
            if (prev_read_fd >= 0) {
                close(prev_read_fd);
            }
            terminate_group(pgid);
            return false;
        }

        if (pid == 0) {
            pid_t child_pgid = (pgid == 0) ? getpid() : pgid;
            setpgid(0, child_pgid);
            prepare_child_signals();

            if (need_pipe) {
                close(pipefd[0]);
            }

            run_child(commands[i],
                      prev_read_fd,
                      need_pipe ? pipefd[1] : -1);
        }

        if (pipefd[1] >= 0) {
            close(pipefd[1]);
        }

        if (pgid == 0) {
            pgid = pid;
        }
        setpgid(pid, pgid);

        result.pids.push_back(pid);

        if (prev_read_fd >= 0) {
            close(prev_read_fd);
        }
        prev_read_fd = pipefd[0];
    }

    if (prev_read_fd >= 0) {
        close(prev_read_fd);
    }

    result.pgid = pgid;
    return true;
}

int register_job(pid_t pgid,
                 const std::vector<pid_t> &pids,
                 const std::string &command,
                 bool background) {
    Job job;
    job.id = next_job_id++;
    job.pgid = pgid;
    job.pids = pids;
    job.command = command;
    job.state = Job::State::Running;
    job.background = background;
    jobs.push_back(job);
    return job.id;
}

bool put_job_in_foreground_internal(int job_id, bool cont) {
    Job *job = find_job_by_id(job_id);
    if (!job) {
        return false;
    }
    job->background = false;

    give_terminal_to(job->pgid);

    if (cont) {
        if (kill(-job->pgid, SIGCONT) < 0) {
            perror("SIGCONT");
        }
    }

    job->state = Job::State::Running;
    wait_for_job(job_id);
    give_terminal_to(shell_pgid);

    return true;
}

} // namespace

void initialize_executor() {
    shell_terminal = STDIN_FILENO;
    shell_interactive = isatty(shell_terminal);
    shell_pgid = getpid();

    if (!shell_interactive) {
        return;
    }

    pid_t fg = tcgetpgrp(shell_terminal);
    while (fg != shell_pgid) {
        if (fg > 0) {
            kill(-fg, SIGTTIN);
        }
        fg = tcgetpgrp(shell_terminal);
    }

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("setpgid");
    }
    give_terminal_to(shell_pgid);
}

bool execute_command(const Command &cmd) {
    if (cmd.commands.empty()) {
        return true;
    }

    SpawnResult result;
    if (!spawn_pipeline(cmd.commands, result)) {
        return true;
    }

    std::string label = describe_command(cmd);
    int job_id = register_job(result.pgid, result.pids, label, cmd.background);

    if (cmd.background) {
        std::cout << "[" << job_id << "] " << result.pgid << " " << label << "\n";
        return true;
    }

    put_job_in_foreground_internal(job_id, false);
    return true;
}

void reap_background_jobs() {
    int status = 0;
    while (true) {
        pid_t pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (pid <= 0) {
            break;
        }
        handle_child_status(pid, status);
    }
}

void print_jobs() {
    for (const auto &job : jobs) {
        std::cout << "[" << job.id << "] "
                  << (job.state == Job::State::Running ? "Running" : "Stopped")
                  << " " << job.command << "\n";
    }
}

bool foreground_job(int job_id) {
    Job *job = find_job_by_id(job_id);
    if (!job) {
        return false;
    }
    job->background = false;
    return put_job_in_foreground_internal(job_id, true);
}

bool background_job(int job_id) {
    Job *job = find_job_by_id(job_id);
    if (!job) {
        return false;
    }
    job->state = Job::State::Running;
    job->background = true;
    if (kill(-job->pgid, SIGCONT) < 0) {
        perror("SIGCONT");
        return false;
    }
    std::cout << "[" << job->id << "] " << job->pgid << " " << job->command << "\n";
    return true;
}

int latest_job_id() {
    if (jobs.empty()) {
        return -1;
    }
    return jobs.back().id;
}
