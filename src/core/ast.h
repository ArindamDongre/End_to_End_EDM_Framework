#ifndef AST_H
#define AST_H

typedef enum {
    AST_OP_ADD, AST_OP_SUB, AST_OP_MUL, AST_OP_DIV,
    AST_OP_EQ, AST_OP_NE, AST_OP_LT, AST_OP_GT, AST_OP_LE, AST_OP_GE
} ASTOp;

typedef enum {
    AST_BLOCK, AST_VAR_DECL, AST_ASSIGN, AST_BINOP,
    AST_INT, AST_IDENT, AST_IF, AST_WHILE, AST_FOR
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    struct ASTNode *left, *right, *third, *next;
    char name[32];
    int value;
    ASTOp op;
} ASTNode;

ASTNode *ast_make_int(int v);
ASTNode *ast_make_ident(char *name);
ASTNode *ast_make_binop(ASTOp op, ASTNode *lhs, ASTNode *rhs);
ASTNode *ast_make_assign(ASTNode *lhs, ASTNode *rhs);
ASTNode *ast_make_var_decl(char *name, ASTNode *init);
ASTNode *ast_make_block(ASTNode *stmts);
ASTNode *ast_make_if(ASTNode *cond, ASTNode *thenb, ASTNode *elseb);
ASTNode *ast_make_while(ASTNode *cond, ASTNode *body);
ASTNode *ast_make_for(ASTNode *init,
                      ASTNode *cond,
                      ASTNode *step,
                      ASTNode *body);
void ast_free(ASTNode *node);

#endif