// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "ownership.h"
#include "codegen.h"

extern int yyparse();
extern FILE* yyin;
extern ASTNode* root_node;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "사용법: moca_compiler <소스파일>\n");
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        fprintf(stderr, "에러: 파일을 열 수 없습니다: %s\n", argv[1]);
        return 1;
    }
    yyin = input;

    printf("[Moca 0.1.2] 1단계: 구문 분석 및 AST 그래프 구축...\n");
    if (yyparse() != 0) {
        fclose(input);
        return 1;
    }
    fclose(input);

    printf("[Moca 0.1.2] 2단계: 정적 메모리 오류(UAF/오버플로우) 분석 모듈 가동...\n");
    init_ownership_engine();
    analyze_ownership(root_node);

    printf("[Moca 0.1.2] 3단계: 검증 완료. 네이티브 C 변환 시작...\n");
    FILE* output = fopen("moca_output.c", "w");
    init_codegen(output);
    generate_code(root_node);
    finalize_codegen();
    fclose(output);

    free_ast(root_node);
    printf("[Moca 0.1.2] 컴파일 성공: 'moca_output.c' 파일 생성 완료.\n");
    return 0;
}
