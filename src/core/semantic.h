#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/*
   Semantic Analysis Phase

   Checks things like:
   - Variable used before declaration
   - Duplicate declarations
*/

void semantic_check(ASTNode *root);

#endif
