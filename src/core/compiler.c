#include "compiler.h"
#include "../compiler/parser_driver.h"

int compile_program(Program *p) {

    printf("DEBUG: compile_program() called for PID %d\n", p->pid);

    ASTNode *ast = NULL;
    IR *ir = NULL;

    if (!parse_source(p->source_path, &ast, &ir)) {
        printf("DEBUG: parse_source failed.\n");
        return 0;
    }

    printf("DEBUG: IR generated successfully.\n");

    p->ast = ast;
    p->ir = ir;

    return 1;
}
