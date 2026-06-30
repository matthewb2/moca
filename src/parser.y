%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

void yyerror(const char *s);
int yylex();
extern ASTNode* root_node;
%}

%union {
    int num;
    char* str;
    struct ASTNode* node;
}

%token <num> NUMBER
%token <str> IDENTIFIER STRING_LITERAL
%token FN MAIN LET PRINTLN

%type <node> program function_decl block statement_list statement expression

%%

program:
    function_decl { root_node = $1; }
    ;

function_decl:
    FN MAIN '(' ')' block { $$ = $5; }
    ;

block:
    '{' statement_list '}' { $$ = create_block_node($2); }
    ;

statement_list:
    statement                { $$ = create_list_node($1); }
    | statement statement_list { $$ = append_to_list($1, $2); }
    ;

statement:
    LET ':' NUMBER IDENTIFIER ';' { $$ = create_moca_decl_node($4, $3, 0); }
    | IDENTIFIER '=' expression ';' { $$ = create_assignment_node($1, $3); }
    | PRINTLN '(' STRING_LITERAL ')' ';' { $$ = create_print_node($3); }
    ;

expression:
    IDENTIFIER { $$ = create_var_node($1); }
    | NUMBER   { $$ = create_num_node($1); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Moca 구문 에러: %s\n", s);
}
