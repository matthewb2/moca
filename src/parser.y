%{
#include <stdio.h>
#include <stdlib.h>

// 1. Bison %union 규칙이 상단 헤더보다 먼저 ASTNode 구조체 포인터를 인식할 수 있도록 전방 선언
struct ASTNode;
typedef struct ASTNode ASTNode;

// 2. 실제 AST 함수 선언이 담긴 헤더 포함
#include "ast.h"

void yyerror(const char *s);
int yylex();

// 최상위 루트 노드 정의
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
    statement                  { $$ = create_list_node($1); }
    | statement_list statement { $$ = append_to_list($1, $2); } /* 🌟 좌재귀로 변경: 기존 리스트($1) 뒤에 새 구문($2)을 올바르게 붙임 */
    ;
    
statement:
    LET ':' NUMBER IDENTIFIER ';'   { $$ = create_moca_decl_node($4, $3, 0); }
    | IDENTIFIER '=' expression ';' { $$ = create_assignment_node($1, $3); }
    | PRINTLN '(' IDENTIFIER ')' ';' { $$ = create_print_node($3); } /* 🌟 추가: 변수명 출력 지원 */
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
