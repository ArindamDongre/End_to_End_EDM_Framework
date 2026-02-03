#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "compiler.h"
#include "ast.h"
#include "ir.h"
#include "../compiler/parser_driver.h"

/* ✅ Bridge for C++ Linking */
#ifdef __cplusplus
extern "C" {
#endif
    int semantic_analysis(ASTNode* root);
    IR* generate_ir(ASTNode* root);
#ifdef __cplusplus
}
#endif

int compile_program(Program* p) {
    if (!p || !p->source_path) return 0;

    printf("DEBUG: compile_program() called for PID %d\n", p->pid);

    ASTNode* root = parse_source(p->source_path);

    if (root == NULL) {
        printf("DEBUG: parse_source failed (returned NULL).\n");
        return 0;
    }

    // Semantic Analysis
    if (semantic_analysis(root) != 0) {
        printf("DEBUG: Semantic analysis failed.\n");
        ast_free(root);          // <-- important
        return 0;
    }

    // IR Generation 
    IR* generated_ir_ptr = generate_ir(root);
    ast_free(root);              // <-- AST no longer needed

    if (!generated_ir_ptr) {
        printf("DEBUG: IR generation failed.\n");
        return 0;
    }

    ir_resolve_labels(generated_ir_ptr);

    p->ir = generated_ir_ptr;
    printf("DEBUG: IR generated successfully.\n");
    p->state = PROGRAM_READY; 
    return 1;
}
