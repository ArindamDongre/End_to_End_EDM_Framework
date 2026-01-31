#ifndef IR_H
#define IR_H

#include <stdio.h>

typedef enum {
    IR_LOAD_CONST,
    IR_LOAD_VAR,
    IR_STORE_VAR,

    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,

    IR_EQ,
    IR_NE,
    IR_LT,
    IR_GT,
    IR_LE,
    IR_GE,

    IR_HALT
} IROpcode;

typedef struct {
    IROpcode op;

    int arg_int;
    char arg_name[32];

    int line;
} IRInstr;

typedef struct IR {
    IRInstr *code;
    int count;
    int capacity;
} IR;

IR *ir_create();
void ir_emit(IR *p, IRInstr instr);
void ir_dump(IR *p);

/* ✅ ADD THIS */
void ir_free(IR *p);

#endif
