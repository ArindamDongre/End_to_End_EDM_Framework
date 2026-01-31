#include <stdio.h>
#include <stdlib.h>

/* Correct includes */
#include "../core/ast.h"
#include "../core/ir.h"
#include "../core/irgen.h"
#include "../core/semantic.h"


#include "parser_driver.h"

/* Flex/Bison globals */
extern int yyparse(void);
extern FILE *yyin;

/* AST root from parser */
extern ASTNode *ast_root;

int parse_source(const char *path, ASTNode **out_ast, IR **out_ir)
{
    printf("DEBUG: Opening source file: %s\n", path);

    yyin = fopen(path, "r");
    if (!yyin) {
        perror("fopen");
        return 0;
    }

    /* Parse */
    if (yyparse() != 0) {
        printf("Parsing failed.\n");
        fclose(yyin);
        return 0;
    }

    fclose(yyin);

    printf("DEBUG: Parsing finished successfully.\n");

    /* Semantic check */
    semantic_check(ast_root);

    /* IR Generation */
    IR *ir = irgen_generate(ast_root);

    *out_ast = ast_root;
    *out_ir = ir;

    return 1;
}
