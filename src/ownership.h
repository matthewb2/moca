// src/ownership.h
#ifndef OWNERSHIP_H
#define OWNERSHIP_H

#include "ast.h" // 👈 여기에서 ASTNode 정의를 직접 완벽하게 가져옵니다.

typedef enum {
    STATE_UNINIT,
    STATE_ALIVE,
    STATE_MOVED
} MocaState;

typedef struct {
    char name[50];
    MocaState state;
    int scope_level;
    int allocated_size;
} MocaSymbol;

void init_ownership_engine();
void analyze_ownership(ASTNode* node); // 이제 완벽하게 호환됩니다.
void enter_scope();
void exit_scope();

#endif
