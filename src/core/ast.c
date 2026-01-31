#include "ast.h"

static ASTNode *ast_new(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    node->left = node->right = node->third = node->next = NULL;
    node->name = NULL;
    node->value = 0;
    node->op = 0;
    return node;
}

ASTNode *ast_make_int(int value) {
    ASTNode *n = ast_new(AST_INT);
    n->value = value;
    return n;
}

ASTNode *ast_make_ident(char *name) {
    ASTNode *n = ast_new(AST_IDENT);
    n->name = strdup(name);
    return n;
}

ASTNode *ast_make_binop(int op, ASTNode *l, ASTNode *r) {
    ASTNode *n = ast_new(AST_BINOP);
    n->op = op;
    n->left = l;
    n->right = r;
    return n;
}

ASTNode *ast_make_assign(ASTNode *l, ASTNode *r) {
    ASTNode *n = ast_new(AST_ASSIGN);
    n->left = l;
    n->right = r;
    return n;
}

ASTNode *ast_make_var_decl(char *name, ASTNode *init) {
    ASTNode *n = ast_new(AST_VAR_DECL);
    n->name = strdup(name);
    n->right = init;
    return n;
}

ASTNode *ast_make_if(ASTNode *c, ASTNode *t, ASTNode *e) {
    ASTNode *n = ast_new(AST_IF);
    n->left = c;
    n->right = t;
    n->third = e;
    return n;
}

ASTNode *ast_make_while(ASTNode *c, ASTNode *b) {
    ASTNode *n = ast_new(AST_WHILE);
    n->left = c;
    n->right = b;
    return n;
}

ASTNode *ast_make_block(ASTNode *stmts) {
    ASTNode *n = ast_new(AST_BLOCK);
    n->left = stmts;
    return n;
}

void ast_free(ASTNode *node) {
    if (!node) return;
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->third);
    ast_free(node->next);
    if (node->name) free(node->name);
    free(node);
}
