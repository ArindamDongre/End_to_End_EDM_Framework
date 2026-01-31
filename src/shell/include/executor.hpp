#pragma once

#include "command.hpp"
#include <string>

// Executes the parsed command (non-builtin).
// Returns true on success, false if a fatal error occurred and the shell
// should terminate (e.g., unrecoverable failure).
bool execute_command(const Command &cmd);

// Initialize job-control/terminal state; call once at startup.
void initialize_executor();

// Process background job status changes.
void reap_background_jobs();

// Job-control helpers for builtins.
void print_jobs();
bool foreground_job(int job_id);
bool background_job(int job_id);
int latest_job_id();
