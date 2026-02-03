#ifndef IR_H
#define IR_H

#include "ast.h"

typedef enum {
    IR_LOAD_CONST,
    IR_LOAD_VAR,
    IR_STORE_VAR,
    IR_ADD, IR_SUB, IR_MUL, IR_DIV,
    IR_EQ, IR_NE, IR_LT, IR_GT, IR_LE, IR_GE,
    IR_JMP,
    IR_JZ,
    IR_LABEL
} IROp;

typedef struct {
    IROp op;
    int value;
    char name[32];
    int line;
} IRInstr;

typedef struct {
    IRInstr *instructions;
    int size;
    int capacity;
} IR;

/* IR functions */
IR* ir_create();
void ir_emit(IR *p, IRInstr instr);
IRInstr make_instr(IROp op, int value, const char* name, int line);
void ir_dump(IR *p);
void ir_free(IR *p);

void ir_resolve_labels(IR *ir);

/* Variable store (VM interface) */
int ir_get_var(const char *name);
void ir_set_var(const char *name, int value);
void ir_print_vars();
void ir_reset_vars();


#endif
