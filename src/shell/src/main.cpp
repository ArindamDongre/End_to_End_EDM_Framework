#include <iostream>
#include <string>
#include <signal.h>
#include <vector>
#include <fstream>   // for std::ifstream, std::ofstream (persistent history)
#include <cstdlib>   // for std::getenv (to get $HOME)
#include <unistd.h>
#include <limits.h> // for PATH_MAX
#include <map>
#include "../include/command.hpp"
#include "../include/parser.hpp"
#include "../include/builtins.hpp"
#include "../include/executor.hpp"
#include "../../core/program.h"
using namespace std;    

static vector<string> history;
static string history_path;

map<int, Program*> program_table;
int next_pid = 1;

// Decide where to store history file
static string get_history_path() {
    const char *home = getenv("HOME");
    if (home && home[0] != '\0') {
        return string(home) + "/.mysh_history";
    }
    // Fallback: current directory if HOME is not set
    return ".mysh_history";
}

// Load existing history from file (if it exists)
static void load_history() {
    history_path = get_history_path();

    ifstream in(history_path);
    if (!in) {
        // No existing history file, that's fine
        return;
    }

    string line;
    while (getline(in, line)) {
        if (!line.empty()) {
            history.push_back(line);
        }
    }
}

// Append a single command line to the history file
static void append_history_line(const string &line) {
    if (history_path.empty()) {
        history_path = get_history_path();
    }

    ofstream out(history_path, std::ios::app);
    if (!out) {
        // Cannot open file, silently ignore (do not crash shell)
        return;
    }

    out << line << '\n';
    // no need to flush explicitly; destructor will flush/close
}

static void print_command_debug(const Command &cmd) {
    cout << "----- Parsed Command -----\n";

    cout << "Total commands: " << cmd.commands.size() << "\n";
    for (size_t idx = 0; idx < cmd.commands.size(); ++idx) {
        const SimpleCommand &simple = cmd.commands[idx];
        cout << "Command " << idx << " argv:";
        for (const auto &arg : simple.argv) {
            cout << " [" << arg << "]";
        }
        cout << "\n";
        if (!simple.input_redirection.empty()) {
            cout << "  input  < " << simple.input_redirection << "\n";
        }
        if (!simple.output_redirection.empty()) {
            cout << "  output > " << simple.output_redirection << "\n";
        }
    }

    cout << "Background: " << (cmd.background ? "YES" : "NO") << "\n";
    cout << "--------------------------\n";
}

int main() {
    load_history(); // load history from previous sessions

    string line;

    signal(SIGINT, SIG_IGN);
    initialize_executor();

    while (true) {

        // Print current working directory to test cd builtin
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) {
            cout << "[cwd] " << cwd << "\n";
        }

        cout << "mysh> ";
        cout.flush();

        if (!getline(cin, line)) {
            cout << "\n";
            break;
        }

        if (line.empty()) {
            continue;
        }

        history.push_back(line);
        append_history_line(line);

        Command cmd;
        string error;
        if (!parse_command(line, cmd, error)) {
            cerr << "Parse error: " << error << "\n";
            continue;
        }

        // History builtin: print command history
        if (!cmd.commands.empty() &&
            !cmd.commands.front().argv.empty() &&
            cmd.commands.front().argv[0] == "history") {
            for (size_t i = 0; i < history.size(); ++i) {
                cout << (i + 1) << "  " << history[i] << "\n";
            }
            continue;
        }

        // Program control commands (our new OS layer)
        if (!cmd.commands.empty() &&
            !cmd.commands.front().argv.empty()) {

            vector<string>& args = cmd.commands.front().argv;

            if (handle_program_commands(args)) {
                // handled by program manager
                continue;
            }
        }

        // Builtins first
        if (is_builtin(cmd)) {
            bool keep_running = run_builtin(cmd);
            if (!keep_running) {
                // exit command
                break;
            }
            // builtin handled, go to next prompt
            reap_background_jobs();
            continue;
        }

        print_command_debug(cmd);

        if (!execute_command(cmd)) {
            break;
        }

        reap_background_jobs();
    }

    return 0;
}
