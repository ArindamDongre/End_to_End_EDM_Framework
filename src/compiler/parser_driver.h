#ifndef PARSER_DRIVER_H
#define PARSER_DRIVER_H

#include "ast.h"
#include "ir.h"

/* ✅ Fixed signature: Returns the AST root pointer, takes only the filename */
ASTNode* parse_source(const char *filename);

#endif