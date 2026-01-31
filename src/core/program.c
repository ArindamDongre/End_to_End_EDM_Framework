#include "program.h"
#include <stdlib.h>
#include <string.h>

Program *program_create(int pid, const char *source_path)
{
    Program *p = malloc(sizeof(Program));

    p->pid = pid;
    p->state = PROGRAM_SUBMITTED;

    p->source_path = strdup(source_path);

    p->ast = NULL;
    p->ir = NULL;

    return p;
}

void program_destroy(Program *p)
{
    if (!p) return;

    if (p->ast)
        ast_free(p->ast);

    if (p->ir)
        ir_free(p->ir);

    if (p->source_path)
        free(p->source_path);

    free(p);
}

const char *program_state_str(ProgramState state)
{
    switch (state)
    {
    case PROGRAM_SUBMITTED: return "SUBMITTED";
    case PROGRAM_READY:     return "READY";
    case PROGRAM_RUNNING:   return "RUNNING";
    case PROGRAM_PAUSED:    return "PAUSED";
    case PROGRAM_TERMINATED:return "TERMINATED";
    default:                return "UNKNOWN";
    }
}

void program_print(Program *p)
{
    printf("Program PID=%d State=%s Source=%s\n",
           p->pid,
           program_state_str(p->state),
           p->source_path);
}
