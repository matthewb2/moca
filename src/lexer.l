%{
#include "parser.h"
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

"#"([^\n]*)     { /* 한 줄 주석 스킵 */ }

[ \t\n\r]+      { /* 공백 및 개행 무시 */ }

.               { /* 알 수 없는 문자 무시하여 파서 크래시 방지 */ }
%%

int yywrap() { return 1; }
