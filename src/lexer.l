%{
#include "parser.tab.h"
#include <string.h>
%}

%%
"fn"            { return FN; }
"main"          { return MAIN; }
"let"           { return LET; }
"println!"      { return PRINTLN; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return IDENTIFIER; }
[0-9]+          { yylval.num = atoi(yytext); return NUMBER; }

\"[^\"\n]*\"    { 
                    yylval.str = strndup(yytext + 1, strlen(yytext) - 2); 
                    return STRING_LITERAL; 
                }

":"             { return ':'; }
"="             { return '='; }
";"             { return ';'; }
"{"             { return '{'; }
"}"             { return '}'; }
"("             { return '('; }
")"             { return ')'; }

[ \t\n]+        { /* 공백 무시 */ }
.               { printf("Moca Lexer Error: %s\n", yytext); }
%%

int yywrap() { return 1; }
