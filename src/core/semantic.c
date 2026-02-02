#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Symbol {
    char *name;
    struct Symbol *next;
} Symbol;

static Symbol *symtab = NULL;

/* Reset table per program */
void semantic_reset() {
    while (symtab) {
        Symbol *tmp = symtab;
        symtab = symtab->next;
        free(tmp->name);
        free(tmp);
    }
    symtab = NULL; // Ensure it's explicitly null
}

static int symbol_exists(const char *name) {
    for (Symbol *s = symtab; s; s = s->next) {
        if (strcmp(s->name, name) == 0)
            return 1;
    }
    return 0;
}

static void symbol_add(const char *name) {
    Symbol *s = malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->next = symtab;
    symtab = s;
}

/* Your internal recursive check */
int semantic_check(ASTNode *node) {
    if (!node) return 0;

    switch (node->type) {
        case AST_VAR_DECL:
            if (symbol_exists(node->name)) {
                printf("Semantic Error: Variable '%s' already declared.\n", node->name);
                return 1;
            }
            symbol_add(node->name);
            break;

        case AST_ASSIGN:
            if (node->left && node->left->type == AST_IDENT) {
                if (!symbol_exists(node->left->name)) {
                    printf("Semantic Error: Variable '%s' not declared.\n", node->left->name);
                    return 1;
                }
            }
            break;

        case AST_IDENT:
            if (!symbol_exists(node->name)) {
                printf("Semantic Error: Variable '%s' not declared.\n", node->name);
                return 1;
            }
            break;

        default:
            break;
    }

    if (semantic_check(node->left))  return 1;
    if (semantic_check(node->right)) return 1;
    if (semantic_check(node->third)) return 1;
    if (semantic_check(node->next))  return 1;

    return 0;
}

/* ✅ THIS IS THE FUNCTION THE COMPILER IS LOOKING FOR */
int semantic_analysis(ASTNode *root) {
    if (!root) return 0;
    
    // 1. Clear old symbols from previous runs
    semantic_reset();
    
    if (semantic_check(root)) return 1;

    return 0;
}