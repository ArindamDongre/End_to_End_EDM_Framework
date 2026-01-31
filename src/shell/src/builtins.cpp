#include "../include/builtins.hpp"
#include <iostream>  // cout, cerr
#include <unistd.h>  // chdir, environ
#include <cstdlib>   // getenv
#include <cctype>
#include <algorithm>
#include <map>
#include "../../core/program.h"                 
#include "../include/executor.hpp"

bool is_builtin(const Command &cmd) {
    // Only treat as builtin if it's a *single* simple command (no pipes)
    if (cmd.commands.size() != 1) {
        return false;
    }

    const SimpleCommand &simple = cmd.commands.front();
    if (simple.argv.empty()) {
        return false;
    }

    const string &name = simple.argv[0];
    return (name == "cd" ||
            name == "exit" ||
            name == "jobs" ||
            name == "fg" ||
            name == "bg" ||
            name == "export"
            );
}


namespace {

int parse_job_id(const vector<string> &argv, const string &cmd_name) {
    int job_id = latest_job_id();

    if (argv.size() == 1) {
        if (job_id < 0) {
            cerr << cmd_name << ": no current job\n";
        }
        return job_id;
    }

    string spec = argv[1];
    if (!spec.empty() && spec[0] == '%') {
        spec.erase(spec.begin());
    }

    if (spec.empty() || !all_of(spec.begin(), spec.end(),
                                [](unsigned char ch) { return std::isdigit(ch); })) {
        cerr << cmd_name << ": invalid job id\n";
        return -1;
    }

    try {
        job_id = stoi(spec);
    } catch (...) {
        cerr << cmd_name << ": invalid job id\n";
        return -1;
    }

    return job_id;
}

bool is_valid_var_name(const std::string &name) {
    if (name.empty()) {
        return false;
    }
    unsigned char c0 = static_cast<unsigned char>(name[0]);
    if (!std::isalpha(c0) && c0 != '_') {
        return false;
    }
    for (unsigned char ch : name) {
        if (!std::isalnum(ch) && ch != '_') {
            return false;
        }
    }
    return true;
}

} // namespace

bool run_builtin(const Command &cmd) {
    if (cmd.commands.empty() || cmd.commands.front().argv.empty()) {
        // nothing to do
        return true; // keep shell running
    }

    const SimpleCommand &simple = cmd.commands.front();
    const string &name = simple.argv[0];

    if (name == "export") {
        const auto &argv = simple.argv;

        // Case 1: no arguments -> print environment
        if (argv.size() == 1) {
            for (char **env = environ; *env != nullptr; ++env) {
                std::cout << *env << "\n";
            }
            return true;
        }

        // Case 2+: one or more arguments
        for (size_t i = 1; i < argv.size(); ++i) {
            const std::string &arg = argv[i];

            std::string var;
            std::string value;
            auto pos = arg.find('=');

            if (pos == std::string::npos) {
                // `export VAR`
                var = arg;
                const char *existing = std::getenv(var.c_str());
                value = existing ? std::string(existing) : std::string{};
            } else {
                // `export VAR=VALUE`
                var = arg.substr(0, pos);
                value = arg.substr(pos + 1);
            }

            if (!is_valid_var_name(var)) {
                std::cerr << "export: `" << var
                        << "': not a valid identifier\n";
                continue;
            }

            if (setenv(var.c_str(), value.c_str(), 1) != 0) {
                perror("export");
            }
        }

        return true;
    }


    if (name == "exit") {
        // Tell main loop to exit
        return false;
    }

    if (name == "jobs") {
        print_jobs();
        return true;
    }

    if (name == "fg") {
        int job_id = parse_job_id(simple.argv, "fg");
        if (job_id < 0) {
            return true;
        }
        if (!foreground_job(job_id)) {
            cerr << "fg: no such job\n";
        }
        return true;
    }

    if (name == "bg") {
        int job_id = parse_job_id(simple.argv, "bg");
        if (job_id < 0) {
            return true;
        }
        if (!background_job(job_id)) {
            cerr << "bg: no such job\n";
        }
        return true;
    }

    if (name == "cd") {
        // cd [dir]
        const char *path = nullptr;

        if (simple.argv.size() < 2) {
            // No argument: use HOME
            path = getenv("HOME");
            if (!path) {
                cerr << "cd: HOME not set\n";
                return true;
            }
        } else if (simple.argv.size() == 2) {
            // One argument: cd <dir>
            path = simple.argv[1].c_str();
        } else {
            // More than one argument: cd a b c  -> error
            cerr << "cd: too many arguments\n";
            return true;
        }

        if (chdir(path) != 0) {
            // chdir returns -1 on error
            perror("cd");
        }

        return true;
    }

    // Should not reach here for now
    return true;
}

extern std::map<int, Program*> program_table;
extern int next_pid;

bool handle_program_commands(std::vector<std::string>& args) {
    if (args[0] == "submit") {
        if (args.size() < 2) {
            std::cout << "Usage: submit <file>\n";
            return true;
        }
        Program* p = program_create(next_pid++, args[1].c_str());
        program_table[p->pid] = p;
        std::cout << "PID = " << p->pid << "\n";
        return true;
    }

    if (args[0] == "list") {
        for (auto& [pid, p] : program_table) {
            program_print(p);
        }
        return true;
    }

    if (args[0] == "run") {
        if (args.size() < 2) {
            cout << "Usage: run <pid>\n";
            return true;
        }

        int pid = stoi(args[1]);

        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program with PID " << pid << "\n";
            return true;
        }

        Program* p = program_table[pid];
        p->state = PROGRAM_RUNNING;
        cout << "Running program " << pid << "\n";
        return true;
    }


    if (args[0] == "kill") {
        if (args.size() < 2) {
            cout << "Usage: run <pid>\n";
            return true;
        }

        int pid = stoi(args[1]);

        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program with PID " << pid << "\n";
            return true;
        }

        program_destroy(program_table[pid]);
        program_table.erase(pid);
        cout << "Killed " << pid << "\n";
        return true;
    }

    return false;
}

