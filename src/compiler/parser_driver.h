#ifndef PARSER_DRIVER_H
#define PARSER_DRIVER_H

#include "../core/ast.h"
#include "../core/ir.h"

int parse_source(const char *filename, ASTNode **ast_out, IR **ir_out);

#endif
