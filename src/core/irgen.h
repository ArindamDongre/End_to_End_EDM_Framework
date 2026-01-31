#ifndef IRGEN_H
#define IRGEN_H

#include "ast.h"
#include "ir.h"

/* Generate IR from AST */
IR *irgen_generate(ASTNode *root);

#endif
