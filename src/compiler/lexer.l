%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration for the parser */
typedef struct ASTNode ASTNode;
#include "parser.tab.h"
%}

%option yylineno

%%
"var"       { return VAR; }
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"for"       { return FOR; }

"=="        { return EQ; }
"!="        { return NE; }
"<="        { return LE; }
">="        { return GE; }
"<"         { return LT; }
">"         { return GT; }

"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MUL; }
"/"         { return DIV; }
"="         { return ASSIGN; }

"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
";"         { return SEMI; }

[0-9]+      { yylval.ival = atoi(yytext); return INTEGER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return IDENTIFIER; }

[ \t\r\n]+  { /* skip whitespace */ }
"//".* { /* skip comments */ }
.           { printf("Lexer error at line %d: Unknown character '%s'\n", yylineno, yytext); }

%%
int yywrap() { return 1; }