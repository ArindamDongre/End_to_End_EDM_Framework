#pragma once

#include "command.hpp"

// Returns true if this command should be handled as a builtin
// instead of being executed via fork/exec.
bool is_builtin(const Command &cmd);

// Runs the builtin command.
// Returns:
//   - true  => shell should continue running
//   - false => shell should exit (for "exit")
bool run_builtin(const Command &cmd);

bool handle_program_commands(std::vector<std::string>& args);
