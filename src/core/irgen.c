#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "irgen.h"
#include "ast.h"
#include "ir.h"

static int next_label = 0;
static int new_label() { return next_label++; }

static void emit_label(IR *ir, int id) {
    ir_emit(ir, make_instr(IR_LABEL, id, NULL));
}

static void emit_jmp(IR *ir, int id) {
    ir_emit(ir, make_instr(IR_JMP, id, NULL));
}

static void emit_jz(IR *ir, int id) {
    ir_emit(ir, make_instr(IR_JZ, id, NULL));
}

static void gen_stmt(IR *ir, ASTNode *n);

static void gen_expr(IR *ir, ASTNode *n) {
    if (!n) return;
    switch(n->type) {
        case AST_INT:
            ir_emit(ir, make_instr(IR_LOAD_CONST, n->value, NULL));
            break;
        case AST_IDENT:
            ir_emit(ir, make_instr(IR_LOAD_VAR, 0, n->name));
            break;
        case AST_BINOP:
            gen_expr(ir, n->left);
            gen_expr(ir, n->right);
            switch(n->op) {
                case AST_OP_ADD: ir_emit(ir, make_instr(IR_ADD, 0, NULL)); break;
                case AST_OP_SUB: ir_emit(ir, make_instr(IR_SUB, 0, NULL)); break;
                case AST_OP_MUL: ir_emit(ir, make_instr(IR_MUL, 0, NULL)); break;
                case AST_OP_DIV: ir_emit(ir, make_instr(IR_DIV, 0, NULL)); break;
                case AST_OP_EQ:  ir_emit(ir, make_instr(IR_EQ,  0, NULL)); break;
                case AST_OP_NE:  ir_emit(ir, make_instr(IR_NE,  0, NULL)); break;
                case AST_OP_LT:  ir_emit(ir, make_instr(IR_LT,  0, NULL)); break;
                case AST_OP_GT:  ir_emit(ir, make_instr(IR_GT,  0, NULL)); break;
                case AST_OP_LE:  ir_emit(ir, make_instr(IR_LE,  0, NULL)); break;
                case AST_OP_GE:  ir_emit(ir, make_instr(IR_GE,  0, NULL)); break;
            }
            break;
        default: break;
    }
}

static void gen_stmt(IR *ir, ASTNode *n) {
    if (!n) return;
    
    // Iterate through linked list of nodes
    for (ASTNode *curr = n; curr; curr = curr->next) {
        switch(curr->type) {
            case AST_VAR_DECL:
                if (curr->left) gen_expr(ir, curr->left);
                else ir_emit(ir, make_instr(IR_LOAD_CONST, 0, NULL));
                ir_emit(ir, make_instr(IR_STORE_VAR, 0, curr->name));
                break;
            case AST_ASSIGN:
                gen_expr(ir, curr->right);
                // LHS of assign is an AST_IDENT, get name from it
                ir_emit(ir, make_instr(IR_STORE_VAR, 0, curr->left->name));
                break;
            case AST_BLOCK:
                gen_stmt(ir, curr->left); // Recurse into block
                break;
            case AST_IF: {
                int else_lbl = new_label();
                int end_lbl = new_label();

                gen_expr(ir, curr->left);
                emit_jz(ir, else_lbl);
                gen_stmt(ir, curr->right);
                emit_jmp(ir, end_lbl);
                emit_label(ir, else_lbl);
                if (curr->third) gen_stmt(ir, curr->third);
                emit_label(ir, end_lbl);
                break;
            }
            case AST_WHILE: {
                int start = new_label();
                int end = new_label();

                emit_label(ir, start);
                gen_expr(ir, curr->left);
                emit_jz(ir, end);
                gen_stmt(ir, curr->right);
                emit_jmp(ir, start);
                emit_label(ir, end);
                break;
            }
            case AST_FOR: {
                int start = new_label();
                int end = new_label();

                gen_stmt(ir, curr->left);    // init
                emit_label(ir, start);
                gen_expr(ir, curr->right);  // condition
                emit_jz(ir, end);
                gen_stmt(ir, curr->third);  // body (already includes step)
                emit_jmp(ir, start);
                emit_label(ir, end);
                break;
            }
            default: break;
        }
    }
}

IR* generate_ir(ASTNode *root) {
    IR *ir = ir_create();
    if (!root) return ir;

    if (root->type == AST_BLOCK) {
        gen_stmt(ir, root->left);
    } else {
        gen_stmt(ir, root);
    }
    return ir;
}