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

    // 1. Parsing
    // ✅ Now matches the fixed header: 1 argument, returns ASTNode*
    ASTNode* root = parse_source(p->source_path);

    if (root == NULL) {
        printf("DEBUG: parse_source failed (returned NULL).\n");
        return 0;
    }

    // 2. Semantic Analysis
    if (semantic_analysis(root) != 0) {
        printf("DEBUG: Semantic analysis failed.\n");
        // Optionally free root here if you have an ast_free function
        return 0;
    }

    // 3. IR Generation 
    IR* generated_ir_ptr = generate_ir(root);
    
    // Save the pointer to the program struct so the shell can access it
    p->ir = generated_ir_ptr;
    
    if (p->ir) {
        printf("DEBUG: IR generated successfully.\n");
        p->state = PROGRAM_READY; 
        return 1; // Success
    }

    printf("DEBUG: IR generation failed.\n");
    return 0; 
}