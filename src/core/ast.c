#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* The actual definition of the global AST root */
ASTNode *ast_root = NULL;

static ASTNode *new_node(ASTNodeType type) {
    ASTNode *n = (ASTNode*)malloc(sizeof(ASTNode));
    if (!n) return NULL;
    memset(n, 0, sizeof(ASTNode));
    n->type = type;
    return n;
}

ASTNode *ast_make_int(int v) {
    ASTNode *n = new_node(AST_INT);
    n->value = v;
    return n;
}

ASTNode *ast_make_ident(char *name) {
    ASTNode *n = new_node(AST_IDENT);
    strncpy(n->name, name, 31);
    free(name);
    return n;
}

ASTNode *ast_make_binop(ASTOp op, ASTNode *lhs, ASTNode *rhs) {
    ASTNode *n = new_node(AST_BINOP);
    n->op = op;
    n->left = lhs;
    n->right = rhs;
    return n;
}

ASTNode *ast_make_assign(ASTNode *lhs, ASTNode *rhs) {
    ASTNode *n = new_node(AST_ASSIGN);
    n->left = lhs;
    n->right = rhs;
    return n;
}

ASTNode *ast_make_var_decl(char *name, ASTNode *init) {
    ASTNode *n = new_node(AST_VAR_DECL);
    strncpy(n->name, name, 31);
    n->left = init;
    free(name);
    return n;
}

ASTNode *ast_make_block(ASTNode *stmts) {
    ASTNode *n = new_node(AST_BLOCK);
    n->left = stmts;
    return n;
}

ASTNode *ast_make_if(ASTNode *cond, ASTNode *thenb, ASTNode *elseb) {
    ASTNode *n = new_node(AST_IF);
    n->left = cond;
    n->right = thenb;
    n->third = elseb;
    return n;
}

ASTNode *ast_make_while(ASTNode *cond, ASTNode *body) {
    ASTNode *n = new_node(AST_WHILE);
    n->left = cond;
    n->right = body;
    return n;
}

void ast_free(ASTNode *n) {
    if (!n) return;
    ast_free(n->left);
    ast_free(n->right);
    ast_free(n->third);
    ast_free(n->next);
    free(n);
}