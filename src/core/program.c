#include "program.h"
#include <stdlib.h>
#include <string.h>

Program* program_create(int pid, const char* source_path) {
    Program* p = (Program*)malloc(sizeof(Program));
    p->pid = pid;
    p->state = PROGRAM_SUBMITTED;
    p->source_path = strdup(source_path);

    p->ast = NULL;
    p->ir = NULL;
    p->vm = NULL;
    p->dbg = NULL;
    p->heap = NULL;

    return p;
}

void program_destroy(Program* p) {
    if (!p) return;

    /* Later: free ast, ir, vm, heap properly */
    free(p->source_path);
    free(p);
}

const char* program_state_str(ProgramState state) {
    switch (state) {
        case PROGRAM_SUBMITTED: return "SUBMITTED";
        case PROGRAM_READY: return "READY";
        case PROGRAM_RUNNING: return "RUNNING";
        case PROGRAM_PAUSED: return "PAUSED";
        case PROGRAM_TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

void program_print(Program* p) {
    if (!p) return;
    printf("PID=%d | STATE=%s | FILE=%s\n",
           p->pid,
           program_state_str(p->state),
           p->source_path);
}
