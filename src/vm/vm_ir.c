// vm_ir.c
#include "vm_ir.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/ir.h"

#define STACK_SIZE 1024
#define MAX_VARS   256
#define MAX_LABELS 256

// --------------------
// VM State
// --------------------

static int stack[STACK_SIZE];
static int sp = 0;

typedef struct {
    char name[32];
    int value;
} Var;

static Var vars[MAX_VARS];
static int var_count = 0;

static int label_pc[MAX_LABELS];

// --------------------
// Stack helpers
// --------------------

static void push(int v) {
    stack[sp++] = v;
}

static int pop() {
    return stack[--sp];
}

// --------------------
// Variable helpers
// --------------------

static int get_var(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    }
    return 0; // default
}

static void set_var(const char *name, int value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            vars[i].value = value;
            return;
        }
    }
    // new variable
    strcpy(vars[var_count].name, name);
    vars[var_count].value = value;
    var_count++;
}

// --------------------
// VM entry point
// --------------------

void vm_execute(IR *ir) {
    if (!ir) return;

    // reset state
    sp = 0;
    var_count = 0;

    // --------------------
    // Build label table
    // --------------------
    for (int i = 0; i < ir->size; i++) {
        if (ir->instructions[i].op == IR_LABEL) {
            label_pc[ir->instructions[i].value] = i;
        }
    }

    // --------------------
    // Execute
    // --------------------
    int pc = 0;

    int steps = 0;
    int MAX_STEPS = 500000;

    while (pc < ir->size) {
        if (++steps > MAX_STEPS) {
            printf("\nVM halted: possible infinite loop\n");
            break;
        }

        IRInstr instr = ir->instructions[pc];

        switch (instr.op) {

            case IR_LOAD_CONST:
                push(instr.value);
                break;

            case IR_LOAD_VAR:
                push(get_var(instr.name));
                break;

            case IR_STORE_VAR:
                set_var(instr.name, pop());
                break;

            case IR_ADD: {
                int b = pop();
                int a = pop();
                push(a + b);
                break;
            }

            case IR_SUB: {
                int b = pop();
                int a = pop();
                push(a - b);
                break;
            }

            case IR_MUL: {
                int b = pop();
                int a = pop();
                push(a * b);
                break;
            }

            case IR_DIV: {
                int b = pop();
                int a = pop();
                push(a / b);
                break;
            }

            case IR_EQ: {
                int b = pop();
                int a = pop();
                push(a == b);
                break;
            }

            case IR_NE: {
                int b = pop();
                int a = pop();
                push(a != b);
                break;
            }

            case IR_LT: {
                int b = pop();
                int a = pop();
                push(a < b);
                break;
            }

            case IR_GT: {
                int b = pop();
                int a = pop();
                push(a > b);
                break;
            }

            case IR_LE: {
                int b = pop();
                int a = pop();
                push(a <= b);
                break;
            }

            case IR_GE: {
                int b = pop();
                int a = pop();
                push(a >= b);
                break;
            }

            case IR_JMP:
                pc = label_pc[instr.value];
                continue;

            case IR_JZ: {
                int v = pop();
                if (v == 0) {
                    pc = label_pc[instr.value];
                    continue;
                }
                break;
            }

            case IR_LABEL:
                // no-op
                break;

            default:
                printf("Unknown IR opcode\n");
                exit(1);
        }

        pc++;
    }

    // --------------------
    // Dump final variables
    // --------------------
    printf("\n--- VM Final State ---\n");
    for (int i = 0; i < var_count; i++) {
        printf("%s = %d\n", vars[i].name, vars[i].value);
    }
    printf("----------------------\n");
}
