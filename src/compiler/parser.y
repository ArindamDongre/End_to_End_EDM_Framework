%define parse.error verbose

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int yylex(void);
void yyerror(const char *s);

/* Root of AST */
ASTNode *ast_root;
%}

%code requires {
    #include "ast.h"
}

/* Semantic values */
%union {
    int ival;
    char *sval;
    ASTNode *node;
}

/* Tokens */
%token VAR IF ELSE WHILE
%token <sval> IDENTIFIER
%token <ival> INTEGER

%token PLUS MINUS MUL DIV
%token ASSIGN EQ NE LT GT LE GE

%token LPAREN RPAREN LBRACE RBRACE SEMI

/* Non-terminals */
%type <node> program stmt stmt_list block expr

/* Operator precedence (LOW → HIGH) */
%left EQ NE LT GT LE GE
%left PLUS MINUS
%left MUL DIV

%start program

%%

program
    : stmt_list
        {
            ast_root = ast_make_block($1);
        }
    ;

stmt_list
    : stmt
        { $$ = $1; }
    | stmt_list stmt
        {
            ASTNode *n = $1;
            while (n->next) n = n->next;
            n->next = $2;
            $$ = $1;
        }
    ;

stmt
    : VAR IDENTIFIER SEMI
        { $$ = ast_make_var_decl($2, NULL); }
    | VAR IDENTIFIER ASSIGN expr SEMI
        { $$ = ast_make_var_decl($2, $4); }
    | IDENTIFIER ASSIGN expr SEMI
        {
            $$ = ast_make_assign(
                    ast_make_ident($1),
                    $3
                 );
        }
    | IF LPAREN expr RPAREN stmt
        { $$ = ast_make_if($3, $5, NULL); }
    | IF LPAREN expr RPAREN stmt ELSE stmt
        { $$ = ast_make_if($3, $5, $7); }
    | WHILE LPAREN expr RPAREN stmt
        { $$ = ast_make_while($3, $5); }
    | block
        { $$ = $1; }
    ;

block
    : LBRACE stmt_list RBRACE
        { $$ = ast_make_block($2); }
    ;

expr
    : INTEGER
        { $$ = ast_make_int($1); }
    | IDENTIFIER
        { $$ = ast_make_ident($1); }
    | expr PLUS expr
        { $$ = ast_make_binop(PLUS, $1, $3); }
    | expr MINUS expr
        { $$ = ast_make_binop(MINUS, $1, $3); }
    | expr MUL expr
        { $$ = ast_make_binop(MUL, $1, $3); }
    | expr DIV expr
        { $$ = ast_make_binop(DIV, $1, $3); }
    | expr EQ expr
        { $$ = ast_make_binop(EQ, $1, $3); }
    | expr NE expr
        { $$ = ast_make_binop(NE, $1, $3); }
    | expr LT expr
        { $$ = ast_make_binop(LT, $1, $3); }
    | expr GT expr
        { $$ = ast_make_binop(GT, $1, $3); }
    | expr LE expr
        { $$ = ast_make_binop(LE, $1, $3); }
    | expr GE expr
        { $$ = ast_make_binop(GE, $1, $3); }
    | LPAREN expr RPAREN
        { $$ = $2; }
    ;

%%

extern int yylineno;

void yyerror(const char *s)
{
    fprintf(stderr, "Parser error at line %d: %s\n", yylineno, s);
}

