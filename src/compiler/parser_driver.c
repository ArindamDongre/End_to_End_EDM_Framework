#include <stdio.h>
#include "ast.h"

extern ASTNode *ast_root;
extern int yyparse();
extern FILE *yyin;
extern int yylineno;

ASTNode* parse_source(const char* source_file) {
    FILE *f = fopen(source_file, "r");
    if (!f) {
        perror("Error opening file");
        return NULL;
    }
    
    yylineno = 1; // Reset line counter
    yyin = f;
    ast_root = NULL;

    if (yyparse() == 0) {
        fclose(f);
        return ast_root;
    }
    
    fclose(f);
    return NULL;
}