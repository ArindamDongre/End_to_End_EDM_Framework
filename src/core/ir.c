#include "ir.h"
#include <stdlib.h>
#include <string.h>

IR *ir_create()
{
    IR *p = (IR *)malloc(sizeof(IR));
    p->count = 0;
    p->capacity = 8;
    p->code = malloc(sizeof(IRInstr) * p->capacity);
    return p;
}

void ir_emit(IR *p, IRInstr instr)
{
    if (p->count >= p->capacity)
    {
        p->capacity *= 2;
        p->code = realloc(p->code, sizeof(IRInstr) * p->capacity);
    }
    p->code[p->count++] = instr;
}

void ir_dump(IR *p)
{
    printf("\n====== IR OUTPUT ======\n");

    for (int i = 0; i < p->count; i++)
    {
        IRInstr in = p->code[i];

        printf("%d: ", i);

        switch (in.op)
        {
        case IR_LOAD_CONST:
            printf("LOAD_CONST %d\n", in.arg_int);
            break;

        case IR_LOAD_VAR:
            printf("LOAD_VAR %s\n", in.arg_name);
            break;

        case IR_STORE_VAR:
            printf("STORE_VAR %s\n", in.arg_name);
            break;

        case IR_ADD:
            printf("ADD\n");
            break;
        case IR_SUB:
            printf("SUB\n");
            break;
        case IR_MUL:
            printf("MUL\n");
            break;
        case IR_DIV:
            printf("DIV\n");
            break;

        case IR_HALT:
            printf("HALT\n");
            break;

        default:
            printf("UNKNOWN\n");
        }
    }
}

/* ✅ REQUIRED FIX */
void ir_free(IR *p)
{
    if (!p) return;

    if (p->code)
        free(p->code);

    free(p);
}
