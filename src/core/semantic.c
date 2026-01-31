#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple symbol table */
typedef struct Symbol {
    char name[32];
    struct Symbol *next;
} Symbol;

static Symbol *symbol_table = NULL;

/* Add variable */
static void declare_var(const char *name)
{
    Symbol *s = symbol_table;
    while (s) {
        if (strcmp(s->name, name) == 0) {
            printf("Semantic Error: Variable '%s' already declared.\n", name);
            exit(1);
        }
        s = s->next;
    }

    Symbol *new_sym = malloc(sizeof(Symbol));
    strncpy(new_sym->name, name, sizeof(new_sym->name));
    new_sym->next = symbol_table;
    symbol_table = new_sym;
}

/* Check variable exists */
static void check_var(const char *name)
{
    Symbol *s = symbol_table;
    while (s) {
        if (strcmp(s->name, name) == 0)
            return;
        s = s->next;
    }

    printf("Semantic Error: Variable '%s' used before declaration.\n", name);
    exit(1);
}

/* Walk AST */
static void visit(ASTNode *node)
{
    if (!node) return;

    switch (node->type)
    {
        case AST_VAR_DECL:
            declare_var(node->name);

            if (node->right)
                visit(node->right);
            break;

        case AST_ASSIGN:
            check_var(node->left->name);
            visit(node->right);
            break;

        case AST_IDENT:
            check_var(node->name);
            break;

        case AST_BINOP:
            visit(node->left);
            visit(node->right);
            break;

        case AST_BLOCK:
            visit(node->left);
            break;

        default:
            break;
    }

    visit(node->next);
}

/* Entry */
void semantic_check(ASTNode *root)
{
    printf("DEBUG: Running semantic check...\n");
    visit(root);
    printf("DEBUG: Semantic check passed.\n");
}
