#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"

IR* ir_create() {
    IR *ir = (IR*)malloc(sizeof(IR));
    ir->capacity = 32;
    ir->size = 0;
    ir->instructions = (IRInstr*)malloc(sizeof(IRInstr) * ir->capacity);
    return ir;
}

void ir_emit(IR *p, IRInstr instr) {
    if (p->size >= p->capacity) {
        p->capacity *= 2;
        p->instructions = (IRInstr*)realloc(p->instructions, sizeof(IRInstr) * p->capacity);
    }
    p->instructions[p->size++] = instr;
}

IRInstr make_instr(IROp op, int value, const char* name) {
    IRInstr instr;
    memset(&instr, 0, sizeof(IRInstr));
    instr.op = op;
    instr.value = value;
    if (name) {
        strncpy(instr.name, name, 31);
    }
    return instr;
}

void ir_dump(IR *p) {
    if (!p || p->size == 0) {
        printf("  (No instructions generated)\n");
        return;
    }
    for (int i = 0; i < p->size; i++) {
        IRInstr instr = p->instructions[i];
        printf("[%03d] ", i);
        switch(instr.op) {
            case IR_LOAD_CONST: printf("LOAD_CONST %d\n", instr.value); break;
            case IR_LOAD_VAR:   printf("LOAD_VAR   %s\n", instr.name); break;
            case IR_STORE_VAR:  printf("STORE_VAR  %s\n", instr.name); break;
            case IR_ADD:        printf("ADD\n"); break;
            case IR_SUB:        printf("SUB\n"); break;
            case IR_MUL:        printf("MUL\n"); break;
            case IR_DIV:        printf("DIV\n"); break;
            case IR_EQ:         printf("CMP_EQ\n"); break;
            case IR_NE:         printf("CMP_NE\n"); break;
            case IR_LT:         printf("CMP_LT\n"); break;
            case IR_GT:         printf("CMP_GT\n"); break;
            case IR_LE:         printf("CMP_LE\n"); break;
            case IR_GE:         printf("CMP_GE\n"); break;
            default:            printf("UNKNOWN_OP\n"); break;
        }
    }
}

void ir_free(IR *p) {
    if (p) {
        free(p->instructions);
        free(p);
    }
}