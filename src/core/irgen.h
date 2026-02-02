#ifndef IRGEN_H
#define IRGEN_H

#include "ast.h"
#include "ir.h"

#ifdef __cplusplus
extern "C" {
#endif

IR* generate_ir(ASTNode *root);

#ifdef __cplusplus
}
#endif

#endif
