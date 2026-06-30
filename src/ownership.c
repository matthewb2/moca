// src/ownership.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ownership.h"

#define MAX_SYMBOLS 1000

static MocaSymbol symbol_table[MAX_SYMBOLS];
static int symbol_count = 0;
static int current_scope = 0;

void init_ownership_engine() {
    symbol_count = 0;
    current_scope = 0;
}

void enter_scope() {
    current_scope++;
}

void exit_scope() {
    for (int i = 0; i < symbol_count; i++) {
        if (symbol_table[i].scope_level == current_scope) {
            if (symbol_table[i].state == STATE_ALIVE) {
                printf("[Moca Analyzer] 스코프 종료: '%s'의 정적 해제(free) 타이밍 확정\n", symbol_table[i].name);
            }
            symbol_table[i].state = STATE_MOVED;
        }
    }
    current_scope--;
}

static int find_symbol(const char* name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].scope_level <= current_scope) {
            return i;
        }
    }
    return -1;
}

void analyze_ownership(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_DECL: {
            int idx = find_symbol(node->data.decl.name);
            if (idx != -1 && symbol_table[idx].scope_level == current_scope) {
                fprintf(stderr, "Moca Compile Error: 중복 선언된 변수 '%s'가 존재합니다.\n", node->data.decl.name);
                exit(1);
            }
            symbol_table[symbol_count].scope_level = current_scope;
            strcpy(symbol_table[symbol_count].name, node->data.decl.name);
            symbol_table[symbol_count].allocated_size = node->data.decl.size;
            symbol_table[symbol_count].state = STATE_ALIVE; // 할당 즉시 사용 가능
            
            printf("[Moca Analyzer] 변수 등록: %s (%d 바이트, 상태: ALIVE)\n", 
                   node->data.decl.name, node->data.decl.size);
            symbol_count++;
            break;
        }

        case NODE_ASSIGN: {
            int dest_idx = find_symbol(node->data.assign.name);
            if (dest_idx == -1) {
                fprintf(stderr, "Moca Compile Error: 선언되지 않은 변수 '%s'에 대입을 시도했습니다.\n", node->data.assign.name);
                exit(1);
            }

            if (node->data.assign.value && node->data.assign.value->type == NODE_VAR) {
                int src_idx = find_symbol(node->data.assign.value->data.var_name);
                if (src_idx != -1) {
                    // 1. Use-After-Free 차단
                    if (symbol_table[src_idx].state == STATE_MOVED) {
                        fprintf(stderr, "Moca Compile Error: 소유권이 이미 상실(MOVED)된 변수 '%s'를 이용할 수 없습니다.\n", symbol_table[src_idx].name);
                        exit(1);
                    }
                    
                    // 2. 버퍼 오버플로우 정적 차단
                    if (symbol_table[dest_idx].allocated_size < symbol_table[src_idx].allocated_size) {
                        fprintf(stderr, "Moca Compile Error: Buffer Overflow 위험 감지! '%s'(%d바이트) 공간에 '%s'(%d바이트) 크기를 대입할 수 없습니다.\n",
                                symbol_table[dest_idx].name, symbol_table[dest_idx].allocated_size,
                                symbol_table[src_idx].name, symbol_table[src_idx].allocated_size);
                        exit(1);
                    }

                    // 3. 소유권 이동(Move Semantics) 강제
                    symbol_table[src_idx].state = STATE_MOVED;
                    symbol_table[dest_idx].state = STATE_ALIVE;
                    printf("[Moca Analyzer] 소유권 이동 감지: '%s' -> '%s'\n", symbol_table[src_idx].name, symbol_table[dest_idx].name);
                }
            }
            analyze_ownership(node->data.assign.value);
            break;
        }

        case NODE_VAR: {
            int idx = find_symbol(node->data.var_name);
            if (idx != -1) {
                if (symbol_table[idx].state == STATE_UNINIT) {
                    fprintf(stderr, "Moca Compile Error: 초기화되지 않은 변수 '%s'를 참조했습니다.\n", symbol_table[idx].name);
                    exit(1);
                }
                if (symbol_table[idx].state == STATE_MOVED) {
                    fprintf(stderr, "Moca Compile Error: 소유권이 상실(MOVED)된 메모리 '%s'에 잘못 접근했습니다.\n", symbol_table[idx].name);
                    exit(1);
                }
            }
            break;
        }

        case NODE_BLOCK:
            enter_scope();
            analyze_ownership(node->data.list.stmt);
            exit_scope();
            break;

        case NODE_LIST:
        case NODE_PROGRAM:
            analyze_ownership(node->data.list.stmt);
            analyze_ownership(node->data.list.next);
            break;

        default:
            break;
    }
}
