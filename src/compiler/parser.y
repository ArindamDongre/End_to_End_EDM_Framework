%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern ASTNode *ast_root;
extern int yylineno;
void yyerror(const char *s);
int yylex(void);

static ASTNode* node_with_line(ASTNode* node) {
    if (node) node->line = yylineno;
    return node;
}
%}

%union {
    int ival;
    char *sval;
    ASTNode *node;
}

%token VAR IF ELSE WHILE FOR ASSIGN SEMI LBRACE RBRACE LPAREN RPAREN
%token PLUS MINUS MUL DIV EQ NE LT GT LE GE
%token <ival> INTEGER
%token <sval> IDENTIFIER

%type <node> program stmt_list stmt block expr for_assign

/* Precedence to fix Dangling Else conflict */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%left EQ NE LT GT LE GE
%left PLUS MINUS
%left MUL DIV

%start program

%%

program
    : /* empty */ { ast_root = NULL; }
    | stmt_list   { ast_root = ast_make_block($1); }
    ;

stmt_list
    : stmt { $$ = $1; }
    | stmt_list stmt {
        if ($1 == NULL) { $$ = $2; }
        else {
            ASTNode *n = $1;
            while (n->next) n = n->next;
            n->next = $2;
            $$ = $1;
        }
    }
    ;

stmt
    /* ✅ WRAP ALL ACTIONS WITH node_with_line(...) */
    : VAR IDENTIFIER SEMI { $$ = node_with_line(ast_make_var_decl($2, NULL)); }
    | VAR IDENTIFIER ASSIGN expr SEMI { $$ = node_with_line(ast_make_var_decl($2, $4)); }
    | IDENTIFIER ASSIGN expr SEMI { $$ = node_with_line(ast_make_assign(ast_make_ident($1), $3)); }
    | IF LPAREN expr RPAREN stmt %prec LOWER_THAN_ELSE { $$ = node_with_line(ast_make_if($3, $5, NULL)); }
    | IF LPAREN expr RPAREN stmt ELSE stmt { $$ = node_with_line(ast_make_if($3, $5, $7)); }
    | WHILE LPAREN expr RPAREN stmt { $$ = node_with_line(ast_make_while($3, $5)); }
    | FOR LPAREN stmt expr SEMI for_assign RPAREN stmt { $$ = node_with_line(ast_make_for($3, $4, $6, $8)); }
    | block { $$ = $1; } /* Block usually inherits or doesn't need specific line */
    | SEMI { $$ = NULL; }
    ;

for_assign
    : IDENTIFIER ASSIGN expr { $$ = node_with_line(ast_make_assign(ast_make_ident($1), $3)); }
    ;



block
    : LBRACE stmt_list RBRACE { $$ = ast_make_block($2); }
    | LBRACE RBRACE { $$ = ast_make_block(NULL); }
    ;

expr
    : INTEGER { $$ = node_with_line(ast_make_int($1)); }
    | IDENTIFIER { $$ = node_with_line(ast_make_ident($1)); }
    | expr PLUS  expr { $$ = node_with_line(ast_make_binop(AST_OP_ADD, $1, $3)); }
    | expr MINUS expr { $$ = node_with_line(ast_make_binop(AST_OP_SUB, $1, $3)); }
    | expr MUL   expr { $$ = node_with_line(ast_make_binop(AST_OP_MUL, $1, $3)); }
    | expr DIV   expr { $$ = node_with_line(ast_make_binop(AST_OP_DIV, $1, $3)); }
    | expr EQ    expr { $$ = node_with_line(ast_make_binop(AST_OP_EQ,  $1, $3)); }
    | expr NE    expr { $$ = node_with_line(ast_make_binop(AST_OP_NE,  $1, $3)); }
    | expr LT    expr { $$ = node_with_line(ast_make_binop(AST_OP_LT,  $1, $3)); }
    | expr GT    expr { $$ = node_with_line(ast_make_binop(AST_OP_GT,  $1, $3)); }
    | expr LE    expr { $$ = node_with_line(ast_make_binop(AST_OP_LE,  $1, $3)); }
    | expr GE    expr { $$ = node_with_line(ast_make_binop(AST_OP_GE,  $1, $3)); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    extern int yylineno;
    fprintf(stderr, "Parser error at line %d: %s\n", yylineno, s);
}