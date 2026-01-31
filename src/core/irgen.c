#include "irgen.h"
#include <string.h>

/* Helpers */
static void gen_expr(IR *ir, ASTNode *n);
static void gen_stmt(IR *ir, ASTNode *n);

/* Helper: Build an IR instruction */
static IRInstr make_instr(IROpcode op,
                          int arg_int,
                          const char *arg_name,
                          int line)
{
    IRInstr instr;
    instr.op = op;
    instr.arg_int = arg_int;

    if (arg_name)
        strncpy(instr.arg_name, arg_name, sizeof(instr.arg_name));
    else
        instr.arg_name[0] = '\0';

    instr.line = line;
    return instr;
}

/* Entry: Generate IR from AST */
IR *irgen_generate(ASTNode *root)
{
    IR *ir = ir_create();

    ASTNode *stmt = root->left;
    while (stmt)
    {
        gen_stmt(ir, stmt);
        stmt = stmt->next;
    }

    /* End program */
    ir_emit(ir, make_instr(IR_HALT, 0, NULL, 0));
    return ir;
}

/* ---------------- STATEMENTS ---------------- */

static void gen_stmt(IR *ir, ASTNode *n)
{
    if (!n) return;

    switch (n->type)
    {

    case AST_VAR_DECL:
        /* var x; OR var x = expr; */
        if (n->right)
        {
            gen_expr(ir, n->right);
            ir_emit(ir, make_instr(IR_STORE_VAR, 0, n->name, 0));
        }
        break;

    case AST_ASSIGN:
        /* x = expr; */
        gen_expr(ir, n->right);
        ir_emit(ir, make_instr(IR_STORE_VAR, 0, n->left->name, 0));
        break;

    default:
        break;
    }
}

/* ---------------- EXPRESSIONS ---------------- */

static void gen_expr(IR *ir, ASTNode *n)
{
    if (!n) return;

    switch (n->type)
    {

    case AST_INT:
        ir_emit(ir, make_instr(IR_LOAD_CONST, n->value, NULL, 0));
        break;

    case AST_IDENT:
        ir_emit(ir, make_instr(IR_LOAD_VAR, 0, n->name, 0));
        break;

    case AST_BINOP:
        gen_expr(ir, n->left);
        gen_expr(ir, n->right);

        if (n->op == '+')
            ir_emit(ir, make_instr(IR_ADD, 0, NULL, 0));
        else if (n->op == '-')
            ir_emit(ir, make_instr(IR_SUB, 0, NULL, 0));
        else if (n->op == '*')
            ir_emit(ir, make_instr(IR_MUL, 0, NULL, 0));
        else if (n->op == '/')
            ir_emit(ir, make_instr(IR_DIV, 0, NULL, 0));

        break;

    default:
        break;
    }
}
