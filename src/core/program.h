#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdio.h>

/* ---------- Program State ---------- */

typedef enum {
    PROGRAM_SUBMITTED,
    PROGRAM_READY,
    PROGRAM_RUNNING,
    PROGRAM_PAUSED,
    PROGRAM_TERMINATED
} ProgramState;

/* ---------- Forward Declarations ---------- */
/* These will be defined in other modules later */

typedef struct AST AST;
typedef struct IR IR;
typedef struct VM VM;
typedef struct Debugger Debugger;
typedef struct GCHeap GCHeap;

/* ---------- Program Control Block ---------- */
/* This is your mini OS process structure */

typedef struct Program {
    int pid;
    ProgramState state;
    char* source_path;

    /* Compiler / Language side */
    AST* ast;
    IR* ir;

    /* Runtime side */
    VM* vm;
    Debugger* dbg;
    GCHeap* heap;
} Program;

/* ---------- Core API ---------- */

/* Create a new program entry (used by shell on submit) */
Program* program_create(int pid, const char* source_path);

/* Destroy program and free all resources */
void program_destroy(Program* p);

/* State helpers */
const char* program_state_str(ProgramState state);

/* Debug utility */
void program_print(Program* p);

#endif
