#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// 🌟 [중요] 파서(parser.y)와 메인(main.c)이 함께 공유할 실제 AST 루트 노드 공간 정의
ASTNode* root_node = NULL;

// 1. 단일 선언/대입문 등을 감싸는 블록 노드 생성
ASTNode* create_block_node(ASTNode* stmt) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "메모리 할당 실패: create_block_node\n");
        exit(1);
    }
    node->type = NODE_BLOCK;
    node->data.list.stmt = stmt;
    node->data.list.next = NULL;
    return node;
}

// 2. 구문들을 연결하기 위한 리스트의 시작(헤드) 노드 생성
ASTNode* create_list_node(ASTNode* stmt) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "메모리 할당 실패: create_list_node\n");
        exit(1);
    }
    node->type = NODE_LIST;
    node->data.list.stmt = stmt;
    node->data.list.next = NULL;
    return node;
}

// src/ast.c 내의 append_to_list 함수를 아래 코드로 교체

ASTNode* append_to_list(ASTNode* head, ASTNode* tail) {
    // 1. 붙일 꼬리가 없으면 머리를 그대로 반환
    if (!tail) return head;
    
    // 2. 머리가 없으면 새 리스트 노드를 만들어 꼬리를 담아 반환
    if (!head) return create_list_node(tail);
    
    // 3. 만약 head가 리스트 타입이 아니라면 리스트 노드로 감싸줍니다. (방어적 코드)
    if (head->type != NODE_LIST && head->type != NODE_PROGRAM) {
        ASTNode* new_head = create_list_node(head);
        new_head->data.list.next = create_list_node(tail);
        return new_head;
    }

    // 4. 리스트의 가장 마지막 연결 고리(next가 NULL인 지점)까지 안전하게 이동
    ASTNode* current = head;
    while (current->data.list.next != NULL) {
        current = current->data.list.next;
    }
    
    // 5. 마지막 지점에 새 리스트 노드를 생성하여 꼬리를 매핑
    current->data.list.next = create_list_node(tail);
    
    return head;
}

// src/ast.c 내부에서 아래 함수들을 찾아 안전하게 문자열을 정제하도록 수정합니다.

ASTNode* create_moca_decl_node(char* name, int size, int init) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "메모리 할당 실패: create_moca_decl_node\n");
        exit(1);
    }
    node->type = NODE_DECL;
    memset(node->data.decl.name, 0, sizeof(node->data.decl.name)); // 🌟 버퍼 완전 초기화
    if (name) {
        strncpy(node->data.decl.name, name, sizeof(node->data.decl.name) - 1);
    }
    node->data.decl.size = size;
    return node;
}

ASTNode* create_assignment_node(char* name, ASTNode* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "메모리 할당 실패: create_assignment_node\n");
        exit(1);
    }
    node->type = NODE_ASSIGN;
    memset(node->data.assign.name, 0, sizeof(node->data.assign.name)); // 🌟 버퍼 완전 초기화
    if (name) {
        strncpy(node->data.assign.name, name, sizeof(node->data.assign.name) - 1);
    }
    node->data.assign.value = value;
    return node;
}

ASTNode* create_print_node(char* var_name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "메모리 할당 실패: create_print_node\n");
        exit(1);
    }
    node->type = NODE_PRINT;
    memset(node->data.var_name, 0, sizeof(node->data.var_name)); // 🌟 버퍼 완전 초기화
    if (var_name) {
        strncpy(node->data.var_name, var_name, sizeof(node->data.var_name) - 1);
    }
    return node;
}

ASTNode* create_var_node(char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "메모리 할당 실패: create_var_node\n");
        exit(1);
    }
    node->type = NODE_VAR;
    memset(node->data.var_name, 0, sizeof(node->data.var_name)); // 🌟 버퍼 완전 초기화
    if (name) {
        strncpy(node->data.var_name, name, sizeof(node->data.var_name) - 1);
    }
    return node;
}



// src/ast.c 내의 create_num_node 함수를 아래와 같이 수정

ASTNode* create_num_node(int val) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "메모리 할당 실패: create_num_node\n");
        exit(1);
    }
    
    // 🌟 핵심 수정: 타입을 NODE_VAR가 아닌, 수식이나 숫자 상수를 뜻하는 타입으로 지정해야 합니다.
    // 만약 ast.h에 NODE_NUM이 정의되어 있지 않다면, 소유권 분석 스위치문에서 
    // 아무것도 하지 않고 패스하도록 분기 처리가 가능한 독립된 타입 상수여야 합니다.
    node->type = 999; // 임시로 분석기(ownership.c) 내 switch-case의 default로 빠지게 유도하거나, 
                      // ast.h에 선언된 숫자용 enum이 있다면 그것을 기입합니다.
                      
    node->data.num_value = val;
    return node;
}

// 9. 빌드가 끝난 후 AST 전체 메모리 해제 (재귀 트리 순회)
void free_ast(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_ASSIGN:
            free_ast(node->data.assign.value);
            break;
            
        case NODE_BLOCK:
            free_ast(node->data.list.stmt);
            break;
            
        case NODE_LIST:
        case NODE_PROGRAM:
            free_ast(node->data.list.stmt);
            free_ast(node->data.list.next);
            break;
            
        default:
            break;
    }
    free(node);
}
