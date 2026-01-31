#ifndef PROGRAM_H
#define PROGRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "ir.h"
#include "ast.h"

typedef enum {
    PROGRAM_SUBMITTED,
    PROGRAM_READY,
    PROGRAM_RUNNING,
    PROGRAM_PAUSED,
    PROGRAM_TERMINATED
} ProgramState;

typedef struct Program {
    int pid;
    ProgramState state;
    char *source_path;

    ASTNode *ast;
    IR *ir;
} Program;

Program *program_create(int pid, const char *source_path);
void program_destroy(Program *p);

const char *program_state_str(ProgramState state);
void program_print(Program *p);

#ifdef __cplusplus
}
#endif

#endif
