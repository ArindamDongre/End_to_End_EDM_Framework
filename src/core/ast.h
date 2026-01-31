#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_BINOP,
    AST_INT,
    AST_IDENT
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;

    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *third;
    struct ASTNode *next;

    char *name;
    int value;
    int op;
} ASTNode;

/* Constructors */
ASTNode *ast_make_int(int value);
ASTNode *ast_make_ident(char *name);
ASTNode *ast_make_binop(int op, ASTNode *left, ASTNode *right);
ASTNode *ast_make_assign(ASTNode *left, ASTNode *right);
ASTNode *ast_make_var_decl(char *name, ASTNode *init);
ASTNode *ast_make_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch);
ASTNode *ast_make_while(ASTNode *cond, ASTNode *body);
ASTNode *ast_make_block(ASTNode *stmts);

/* Utility */
void ast_print(ASTNode *node, int indent);
void ast_free(ASTNode *node);

#endif
