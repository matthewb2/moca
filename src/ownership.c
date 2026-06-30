#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ownership.h"

#define MAX_SYMBOLS 1000

static MocaSymbol symbol_table[MAX_SYMBOLS];
static int symbol_count = 0;
static int current_scope = 0;

static int find_symbol(const char* name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].scope_level <= current_scope) {
            return i;
        }
    }
    return -1;
}

// 🌟 트리 구조 디버깅을 위한 타입 문자열 변환 함수
static const char* get_node_type_name(int type) {
    switch(type) {
        case 0: return "NODE_PROGRAM";
        case 1: return "NODE_BLOCK";
        case 2: return "NODE_LIST";
        case 3: return "NODE_DECL";
        case 4: return "NODE_ASSIGN";
        case 5: return "NODE_VAR";
        case 6: return "NODE_PRINT";
        default: return "UNKNOWN_NODE_TYPE";
    }
}

void analyze_ownership(ASTNode* node) {
    if (!node) {
        printf("[Moca Debug] NULL 노드 도달\n");
        return;
    }

    // 🌟 [디버깅 추가] 현재 분석 엔진이 밟고 있는 노드의 물리적 정보 출력
    printf("[Moca Debug] 현재 노드 주소: %p, 타입: %s (%d)\n", (void*)node, get_node_type_name(node->type), node->type);

    switch (node->type) {
        case NODE_DECL: {
            printf("[Moca Debug] NODE_DECL 발견 -> 변수명: '%s', 크기: %d\n", node->data.decl.name, node->data.decl.size);
            int idx = find_symbol(node->data.decl.name);
            if (idx != -1 && symbol_table[idx].scope_level == current_scope) {
                fprintf(stderr, "Moca Compile Error: 중복 선언된 변수 '%s'가 존재합니다.\n", node->data.decl.name);
                exit(1);
            }
            symbol_table[symbol_count].scope_level = current_scope;
            strcpy(symbol_table[symbol_count].name, node->data.decl.name);
            symbol_table[symbol_count].allocated_size = node->data.decl.size;
            symbol_table[symbol_count].state = STATE_ALIVE;
            
            printf("[Moca Analyzer] 변수 등록: %s (%d 바이트, 상태: ALIVE)\n", 
                   node->data.decl.name, node->data.decl.size);
            symbol_count++;
            break;
        }

case NODE_ASSIGN: {
            printf("[Moca Debug] NODE_ASSIGN 발견 -> 대입 대상 변수명: '%s'\n", node->data.assign.name);
            int dest_idx = find_symbol(node->data.assign.name);
            if (dest_idx == -1) {
                fprintf(stderr, "Moca Compile Error: 선언되지 않은 변수 '%s'에 대입을 시도했습니다.\n", node->data.assign.name);
                exit(1);
            }

            int is_pure_var = 0; // 우변이 단순 변수인지 체크할 플래그

            if (node->data.assign.value) {
                printf("[Moca Debug] 대입 우변(Value) 노드 타입: %s\n", get_node_type_name(node->data.assign.value->type));
                if (node->data.assign.value->type == NODE_VAR) {
                    is_pure_var = 1; // 🌟 단순 변수 참조이므로 마킹
                    printf("[Moca Debug] 우변 변수명 참조 시도: '%s'\n", node->data.assign.value->data.var_name);
                    int src_idx = find_symbol(node->data.assign.value->data.var_name);
                    if (src_idx != -1) {
                        if (symbol_table[src_idx].state == STATE_MOVED) {
                            fprintf(stderr, "Moca Compile Error: 소유권이 이미 상실(MOVED)된 변수 '%s'를 이용할 수 없습니다.\n", symbol_table[src_idx].name);
                            exit(1);
                        }
                        if (symbol_table[dest_idx].allocated_size < symbol_table[src_idx].allocated_size) {
                            fprintf(stderr, "Moca Compile Error: Buffer Overflow 위험 감지! '%s'(%d바이트) 공간에 '%s'(%d바이트) 크기를 대입할 수 없습니다.\n",
                                    symbol_table[dest_idx].name, symbol_table[dest_idx].allocated_size,
                                    symbol_table[src_idx].name, symbol_table[src_idx].allocated_size);
                            exit(1);
                        }
                        symbol_table[src_idx].state = STATE_MOVED;
                        symbol_table[dest_idx].state = STATE_ALIVE;
                        printf("[Moca Analyzer] 소유권 이동 감지: '%s' -> '%s'\n", symbol_table[src_idx].name, symbol_table[dest_idx].name);
                    } else {
                        printf("[Moca Debug] 우변 변수 '%s'를 심볼 테이블에서 찾지 못함\n", node->data.assign.value->data.var_name);
                    }
                }
            }
            
            // 🌟 우변이 단순 변수(NODE_VAR)가 아닐 때만(예: 수식이나 숫자) 하위 노드를 추가 분석합니다.
            // 단순 변수일 때는 위에서 소유권 이동 처리를 완료했으므로 중복 검사로 진입하지 않습니다.
            if (!is_pure_var) {
                analyze_ownership(node->data.assign.value);
            }
            break;
        }
        case NODE_VAR: {
            printf("[Moca Debug] NODE_VAR 참조 발견 -> 변수명: '%s'\n", node->data.var_name);
            int idx = find_symbol(node->data.var_name);
            if (idx != -1) {
                printf("[Moca Debug] NODE_VAR 심볼 상태 확인 -> 변수명: '%s', 현재 상태: %d (0:UNINIT, 1:ALIVE, 2:MOVED)\n", 
                       symbol_table[idx].name, symbol_table[idx].state);
                if (symbol_table[idx].state == STATE_MOVED) {
                    fprintf(stderr, "Moca Compile Error: 소유권이 상실(MOVED)된 메모리 '%s'에 잘못 접근했습니다.\n", symbol_table[idx].name);
                    exit(1);
                }
            } else {
                printf("[Moca Debug] NODE_VAR '%s'는 심볼 테이블에 등록되어 있지 않음\n", node->data.var_name);
            }
            break;
        }

        case NODE_PRINT: {
            printf("[Moca Debug] NODE_PRINT 발견 -> 대상 이름: '%s'\n", node->data.var_name);
            int idx = find_symbol(node->data.var_name);
            if (idx != -1) {
                if (symbol_table[idx].state == STATE_MOVED) {
                    fprintf(stderr, "Moca Compile Error: 소유권이 상실(MOVED)된 메모리 '%s'에 잘못 접근했습니다.\n", symbol_table[idx].name);
                    exit(1);
                }
            }
            break;
        }

        case NODE_BLOCK:
            printf("[Moca Debug] NODE_BLOCK 진입 -> 스코프 스택 다운\n");
            // 기존 스코프 진입 로직 수행 코드 유지 처리를 위해 내부 직접 기입
            current_scope++; 
            analyze_ownership(node->data.list.stmt);
            current_scope--;
            printf("[Moca Debug] NODE_BLOCK 탈출 -> 스코프 스택 업\n");
            break;

        case NODE_LIST:
        case NODE_PROGRAM:
            printf("[Moca Debug] LIST 노드 전개 -> stmt: %p, next: %p\n", (void*)node->data.list.stmt, (void*)node->data.list.next);
            // 현재 타임라인 추적을 위해 기본 순서 유지 후 로그 확인
            analyze_ownership(node->data.list.stmt);
            analyze_ownership(node->data.list.next);
            break;

        default:
            printf("[Moca Debug] 디폴트 분기 처리 거침 (타입 번호: %d)\n", node->type);
            break;
    }
}

void init_ownership_engine() {
    symbol_count = 0;
    current_scope = 0;
}
