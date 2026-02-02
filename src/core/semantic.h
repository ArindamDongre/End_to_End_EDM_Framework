#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/* Reset symbol table per compile */
void semantic_reset();

/* Main semantic pass */
int semantic_check(ASTNode *root);

#endif
