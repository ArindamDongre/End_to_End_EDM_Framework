#include "vm_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STACK_SIZE 1024
#define MAX_STEPS 500000

#define MAX_VARS 256

typedef struct {
    char name[32];
    Object *value;
} Var;

static Var vars[MAX_VARS];
static int var_count = 0;


/* =========================
   Heap / GC
   ========================= */

static Object* heap_alloc(VM *vm, int value) {
    Object *o = malloc(sizeof(Object));
    o->value = value;
    o->marked = false;
    o->next = vm->heap;
    vm->heap = o;
    return o;
}

static void mark(Object *o) {
    if (!o || o->marked) return;
    o->marked = true;
}

void gc_collect(VM *vm) {
    // mark stack roots
    for (int i = 0; i < vm->sp; i++)
        mark(vm->stack[i]);

    for (int i = 0; i < var_count; i++) {
        if (vars[i].value) {
             mark(vars[i].value);
        }
    }

    // sweep
    Object **p = &vm->heap;
    while (*p) {
        if (!(*p)->marked) {
            Object *dead = *p;
            *p = dead->next;
            free(dead);
        } else {
            (*p)->marked = false;
            p = &(*p)->next;
        }
    }
}

// Add this function at the end or near gc_collect
void vm_destroy(VM *vm) {
    if (!vm) return;
    
    // Free all objects on the heap
    Object *curr = vm->heap;
    while (curr) {
        Object *next = curr->next;
        free(curr);
        curr = next;
    }
    vm->heap = NULL;
    vm->sp = 0;
}

static Object* get_var(const char *name) {
    for (int i = 0; i < var_count; i++)
        if (strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    return NULL;
}

static void set_var(const char *name, Object *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            vars[i].value = value;
            return;
        }
    }
    strcpy(vars[var_count].name, name);
    vars[var_count].value = value;
    var_count++;
}


void vm_report_leaks(VM *vm) {
    int count = 0;
    int bytes = 0;
    for (Object *o = vm->heap; o; o = o->next) {
        count++;
        bytes += sizeof(Object);
    }
    printf("\n--- Leak Report ---\n");
    printf("Leaked objects: %d\n", count);
    printf("Leaked bytes:   %d\n", bytes);
    printf("-------------------\n");
}

/* =========================
   Stack helpers
   ========================= */

static void push(VM *vm, Object *o) {
    vm->stack[vm->sp++] = o;
}

static Object* pop(VM *vm) {
    return vm->stack[--vm->sp];
}

/* =========================
   State printing
   ========================= */

void vm_print_state(VM *vm) {
    printf("PC = %d\n", vm->pc);
    printf("Stack:\n");
    for (int i = 0; i < vm->sp; i++) {
        printf("  [%d] %d\n", i, vm->stack[i]->value);
    }
}

/* =========================
   Single step
   ========================= */

bool vm_step(VM *vm) {
    if (vm->pc >= vm->ir->size) return false;

    if (vm->breakpoints[vm->pc]) {
        printf("Breakpoint hit at IR[%d]\n", vm->pc);
        return false;
    }

    if (++vm->steps > MAX_STEPS) {
        printf("\nVM halted: possible infinite loop\n");
        return false;
    }

    IRInstr instr = vm->ir->instructions[vm->pc++];

    switch (instr.op) {

        case IR_LOAD_CONST:
            push(vm, heap_alloc(vm, instr.value));
            break;

        case IR_ADD: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value + b->value));
            break;
        }

        case IR_SUB: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value - b->value));
            break;
        }

        case IR_MUL: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value * b->value));
            break;
        }

        case IR_DIV: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            if (b->value == 0) {
                printf("Runtime Error: division by zero\n");
                exit(1);
            }
            push(vm, heap_alloc(vm, a->value / b->value));
            break;
        }

        case IR_EQ: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value == b->value));
            break;
        }
        case IR_NE: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value != b->value));
            break;
        }
        case IR_LT: {
            Object *b = pop(vm); // Second operand is at top of stack
            Object *a = pop(vm); // First operand is below it
            push(vm, heap_alloc(vm, a->value < b->value));
            break;
        }
        case IR_GT: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value > b->value));
            break;
        }
        case IR_LE: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value <= b->value));
            break;
        }
        case IR_GE: {
            Object *b = pop(vm);
            Object *a = pop(vm);
            push(vm, heap_alloc(vm, a->value >= b->value));
            break;
        }

        case IR_JMP:
            vm->pc = instr.value;
            break;

        case IR_JZ: {
            Object *v = pop(vm);
            if (v->value == 0)
                vm->pc = instr.value;
            break;
        }

        case IR_LOAD_VAR:
            Object *v = get_var(instr.name);
            if (!v) v = heap_alloc(vm, 0);
            push(vm, v);
            break;

        case IR_STORE_VAR: {
            Object *v = pop(vm);
            set_var(instr.name, v);
            break;
        }

        case IR_LABEL:
            break;

        default:
            printf("Unknown opcode\n");
            exit(1);
    }

    return true;
}

/* =========================
   Full run
   ========================= */

void vm_executor(VM *vm) {
    while (vm_step(vm)) {}
    gc_collect(vm);
    vm_report_leaks(vm);
}

/* =========================
   Debugger
   ========================= */

void vm_debug(VM *vm) {
    char cmd[64];

    printf("Entering VM debugger. Type 'help' for commands.\n");

    while (1) {
        printf("(dbg) ");
        if (!fgets(cmd, sizeof(cmd), stdin)) break;

        // 1. HELP
        if (!strcmp(cmd, "help\n")) {
            printf("Available commands:\n");
            printf("  step          Execute one instruction\n");
            printf("  continue      Resume execution until completion or breakpoint\n");
            printf("  break <line>  Set breakpoint at source line\n");
            printf("  state         Print current registers and stack\n");
            printf("  memstat       Show memory usage statistics\n");
            printf("  gc            Run garbage collector\n");
            printf("  quit          Exit debugger (program remains in current state)\n");
        }
        // 2. STEP
        else if (!strcmp(cmd, "step\n")) {
            if (!vm_step(vm)) {
                printf("Program finished or halted.\n");
                break;
            }
        }
        // 3. CONTINUE
        else if (!strcmp(cmd, "continue\n")) {
            vm_executor(vm);
            // If executor returns, it means we hit a breakpoint or finished
            if (vm->pc >= vm->ir->size) {
                 printf("Program finished execution.\n");
                 break; 
            }
        }
        // 4. STATE
        else if (!strcmp(cmd, "state\n")) {
            vm_print_state(vm);
        }
        // 5. BREAKPOINT (Source Level)
        else if (!strncmp(cmd, "break ", 6)) {
            int target_line = atoi(cmd + 6);
            int found = 0;
            // Scan IR for the first instruction matching the line
            for(int i=0; i<vm->ir->size; i++) {
                if(vm->ir->instructions[i].line == target_line) {
                    vm->breakpoints[i] = true;
                    printf("Breakpoint set at line %d (IP=%d)\n", target_line, i);
                    found = 1;
                    break; // Only set on the first instruction of that line
                }
            }
            if (!found) printf("No instruction found for line %d\n", target_line);
        }
        // ✅ 6. MEMSTAT / LEAKS (New Integration)
        else if (!strcmp(cmd, "memstat\n") || !strcmp(cmd, "leaks\n")) {
            vm_report_leaks(vm);
        }
        // ✅ 7. GC (New Integration)
        else if (!strcmp(cmd, "gc\n")) {
            printf("Running Garbage Collector...\n");
            gc_collect(vm);
            printf("GC Complete.\n");
            vm_report_leaks(vm);
        }
        // 8. QUIT
        else if (!strcmp(cmd, "quit\n")) {
            break;
        }
        else {
            printf("Unknown command. Type 'help'.\n");
        }
    }
}

/* =========================
   Init
   ========================= */

void vm_init(VM *vm, IR *ir) {
    memset(vm, 0, sizeof(VM));
    vm->ir = ir;
    var_count = 0;
}
