%option yylineno
%option noyywrap


%{
#include "parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

%}

%%
[ \t\r]+      ;
\n            ;

"var"        return VAR;
"if"         return IF;
"else"       return ELSE;
"while"      return WHILE;

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.sval = strdup(yytext);
    return IDENTIFIER;
}

[0-9]+ {
    yylval.ival = atoi(yytext);
    return INTEGER;
}

"=="    return EQ;
"!="    return NE;
"<="    return LE;
">="    return GE;

"="     return ASSIGN;
"<"     return LT;
">"     return GT;
"+"     return PLUS;
"-"     return MINUS;
"*"     return MUL;
"/"     return DIV;

"("     return LPAREN;
")"     return RPAREN;
"{"     return LBRACE;
"}"     return RBRACE;
";"     return SEMI;

"//".*     ;

. {
    fprintf(stderr, "Lexical error at line %d: illegal character '%s'\n",
            yylineno, yytext);
    exit(1);
}


%%
