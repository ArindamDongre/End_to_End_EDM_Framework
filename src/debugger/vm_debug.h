#ifndef VM_DEBUG_H
#define VM_DEBUG_H

#include <stdbool.h>
#include "../core/ir.h"

typedef struct Object {
    int value;
    bool marked;
    struct Object *next;
} Object;

typedef struct VM{
    IR *ir;
    int pc;

    Object *stack[1024];
    int sp;

    Object *heap;

    bool breakpoints[10000];
    int steps;
} VM;

void vm_init(VM *vm, IR *ir);
bool vm_step(VM *vm);
void vm_executor(VM *vm);
void vm_debug(VM *vm);
void vm_print_state(VM *vm);
void vm_report_leaks(VM *vm);
void gc_collect(VM *vm);
void vm_destroy(VM *vm); // New function to clean up VM memory

#endif
