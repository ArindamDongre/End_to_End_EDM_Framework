#include "../include/builtins.hpp"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <map>

/* ✅ FIX 1: Wrap C headers in extern "C" so the C++ shell can see the C compiler data */
extern "C" {
#include "../../core/program.h"
#include "../../core/compiler.h"
#include "../../core/ir.h"
#include "../../debugger/vm_debug.h"
}

#include "../include/executor.hpp"

using namespace std;

bool is_builtin(const Command &cmd) {
    if (cmd.commands.size() != 1) return false;

    const SimpleCommand &simple = cmd.commands.front();
    if (simple.argv.empty()) return false;

    const string &name = simple.argv[0];

    return (name == "cd" ||
            name == "exit" ||
            name == "jobs" ||
            name == "fg" ||
            name == "bg" ||
            name == "export" ||

            // ✅ Program builtins
            name == "submit" ||
            name == "list" ||
            name == "run" ||
            name == "kill" ||

            // ✅ Add Memory Management Commands
            name == "memstat" ||
            name == "gc" ||
            name == "leaks" ||

            // ✅ Phase2 new commands
            name == "compile" ||
            name == "ir");
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
                                [](unsigned char ch) { return isdigit(ch); })) {
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
    if (name.empty()) return false;

    unsigned char c0 = static_cast<unsigned char>(name[0]);
    if (!isalpha(c0) && c0 != '_') return false;

    for (unsigned char ch : name) {
        if (!isalnum(ch) && ch != '_') return false;
    }
    return true;
}

} // namespace

bool is_number(const string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool run_builtin(const Command &cmd) {
    if (cmd.commands.empty() || cmd.commands.front().argv.empty()) {
        return true;
    }

    const SimpleCommand &simple = cmd.commands.front();
    const string &name = simple.argv[0];

    // ---------------- EXPORT ----------------
    if (name == "export") {
        const auto &argv = simple.argv;

        if (argv.size() == 1) {
            for (char **env = environ; *env != nullptr; ++env) {
                cout << *env << "\n";
            }
            return true;
        }

        for (size_t i = 1; i < argv.size(); ++i) {
            const string &arg = argv[i];

            string var, value;
            auto pos = arg.find('=');

            if (pos == string::npos) {
                var = arg;
                const char *existing = getenv(var.c_str());
                value = existing ? string(existing) : "";
            } else {
                var = arg.substr(0, pos);
                value = arg.substr(pos + 1);
            }

            if (!is_valid_var_name(var)) {
                cerr << "export: `" << var
                     << "': not a valid identifier\n";
                continue;
            }

            if (setenv(var.c_str(), value.c_str(), 1) != 0) {
                perror("export");
            }
        }

        return true;
    }

    // ---------------- EXIT ----------------
    if (name == "exit") {
        return false;
    }

    // ---------------- JOBS ----------------
    if (name == "jobs") {
        print_jobs();
        return true;
    }

    // ---------------- FG ----------------
    if (name == "fg") {
        int job_id = parse_job_id(simple.argv, "fg");
        if (job_id < 0) return true;
        if (!foreground_job(job_id)) cerr << "fg: no such job\n";
        return true;
    }

    // ---------------- BG ----------------
    if (name == "bg") {
        int job_id = parse_job_id(simple.argv, "bg");
        if (job_id < 0) return true;
        if (!background_job(job_id)) cerr << "bg: no such job\n";
        return true;
    }

    // ---------------- CD ----------------
    if (name == "cd") {
        const char *path = nullptr;

        if (simple.argv.size() < 2) {
            path = getenv("HOME");
            if (!path) {
                cerr << "cd: HOME not set\n";
                return true;
            }
        } else if (simple.argv.size() == 2) {
            path = simple.argv[1].c_str();
        } else {
            cerr << "cd: too many arguments\n";
            return true;
        }

        if (chdir(path) != 0) perror("cd");
        return true;
    }

    return true;
}

/* ============================================================
    PROGRAM COMMANDS (submit/list/run/kill/compile/ir)
   ============================================================ */

extern std::map<int, Program*> program_table;
extern int next_pid;

bool handle_program_commands(std::vector<std::string>& args) {

    // ---------------- SUBMIT ----------------
    if (args[0] == "submit") {
        if (args.size() < 2) {
            cout << "Usage: submit <file>\n";
            return true;
        }

        Program* p = program_create(next_pid++, args[1].c_str());
        program_table[p->pid] = p;

        cout << "PID = " << p->pid << "\n";
        return true;
    }

    // ---------------- LIST ----------------
    if (args[0] == "list") {
        for (auto& [pid, p] : program_table) {
            program_print(p);
        }
        return true;
    }

    // ---------------- COMPILE ----------------
    if (args[0] == "compile") {
        if (args.size() < 2) {
            cout << "Usage: compile <pid>\n";
            return true;
        }

        int pid = stoi(args[1]);

        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program with PID " << pid << "\n";
            return true;
        }

        Program* p = program_table[pid];

        cout << "Compiling program " << pid << "...\n";

        if (!compile_program(p)) {
            cout << "Compilation failed.\n";
            return true;
        }

        cout << "Compilation successful.\n";
        return true;
    }

    // ---------------- IR ----------------
    if (args[0] == "ir") {
        if (args.size() < 2) {
            cout << "Usage: ir <pid>" << endl;
            return true;
        }
        int pid = stoi(args[1]);
        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program with PID " << pid << endl;
            return true;
        }

        Program* p = program_table[pid];
        
        /* ✅ FIX 2: Correct pointer check for IR */
        if (p->ir == nullptr) {
            cout << "Program not compiled yet. Run: compile " << pid << endl;
        } else {
            cout << "IR for Program " << pid << ":" << endl;
            ir_dump(p->ir); 
        }
        return true;
    }

// ---------------- RUN ----------------
    if (args[0] == "run") {
        if (args.size() < 2) { cout << "Usage: run <pid>\n"; return true; }
        int pid = stoi(args[1]);
        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program with PID " << pid << "\n";
            return true;
        }

        Program* p = program_table[pid];

        // Compile if needed
        if (p->ir == nullptr) {
            if (!compile_program(p)) { cout << "Compilation failed.\n"; return true; }
        }

        // ✅ Initialize Persistent VM if needed
        if (p->vm == nullptr) {
            p->vm = (VM*)malloc(sizeof(VM));
            vm_init(p->vm, p->ir);
        } else {
            // Optional: Reset VM if re-running
            // vm_init(p->vm, p->ir); 
        }

        p->state = PROGRAM_RUNNING;
        cout << "Running program " << pid << "\n";

        // ✅ Use the persistent VM instance
        vm_executor(p->vm);
        
        // Mark as terminated or paused depending on implementation
        p->state = PROGRAM_TERMINATED; 
        return true;
    }

    // ---------------- MEMSTAT / LEAKS ----------------
    if (args[0] == "memstat" || args[0] == "leaks") {
        if (args.size() < 2) { cout << "Usage: " << args[0] << " <pid>\n"; return true; }
        int pid = stoi(args[1]);
        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program.\n";
            return true;
        }
        
        Program* p = program_table[pid];
        if (p->vm == nullptr) {
            cout << "Program has not been run yet (no memory state).\n";
        } else {
            vm_report_leaks(p->vm);
        }
        return true;
    }

    // ---------------- GC ----------------
    if (args[0] == "gc") {
        if (args.size() < 2) { cout << "Usage: gc <pid>\n"; return true; }
        int pid = stoi(args[1]);
        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program.\n";
            return true;
        }

        Program* p = program_table[pid];
        if (p->vm == nullptr) {
            cout << "Program has not been run yet.\n";
        } else {
            cout << "Running Garbage Collector on PID " << pid << "...\n";
            gc_collect(p->vm);
            vm_report_leaks(p->vm); // Report status after GC
        }
        return true;
    }

    // ---------------- KILL ----------------
    if (args[0] == "kill") {
        if (args.size() < 2) {
            cout << "Usage: kill <pid>\n";
            return true;
        }

        if (!is_number(args[1])) {
            cout << "PID must be a number\n";
            return true;
        }


        int pid = stoi(args[1]);

        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program with PID " << pid << "\n";
            return true;
        }

        // ✅ Fix: Use the correct function name defined in program.h/c
        program_destroy(program_table[pid]); 
        program_table.erase(pid);

        cout << "Killed " << pid << "\n";
        return true;
    }

// ---------------- DEBUG ----------------
    if (args[0] == "debug") {
        if (args.size() < 2) {
            cout << "Usage: debug <pid>\n";
            return true;
        }

        int pid = stoi(args[1]);
        if (program_table.find(pid) == program_table.end()) {
            cout << "No such program with PID " << pid << "\n";
            return true;
        }

        Program* p = program_table[pid];

        // 1. Compile if needed
        if (p->ir == nullptr) {
            if (!compile_program(p)) return true;
        }
            
        // 2. Prepare VM (Create if null, OR RESET if existing)
        if (p->vm == nullptr) {
            p->vm = (VM*)malloc(sizeof(VM));
        }
        
        // ✅ CRITICAL FIX: Always reset the VM when starting a debug session
        // This ensures PC starts at 0 and Stack is clear.
        vm_init(p->vm, p->ir); 

        // 3. State Transition: RUNNING -> PAUSED
        p->state = PROGRAM_PAUSED;
        cout << "Attached to PID " << pid << " [PAUSED]\n";

        // 4. Enter Debug Loop
        vm_debug(p->vm);

        // 5. State Transition on Exit
        if (p->vm->pc >= p->ir->size) {
            p->state = PROGRAM_TERMINATED;
            cout << "PID " << pid << " terminated.\n";
        } else {
            p->state = PROGRAM_PAUSED;
            cout << "PID " << pid << " is paused.\n";
        }

        return true;
    }

    return false;
}