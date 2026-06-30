// src/ownership.h
#ifndef OWNERSHIP_H
#define OWNERSHIP_H

#include "ast.h"

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
void analyze_ownership(ASTNode* node);
void enter_scope();
void exit_scope();

#endif
