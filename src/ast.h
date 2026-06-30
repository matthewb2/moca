// src/ast.h
#ifndef AST_H
#define AST_H

// 명확하게 struct와 typedef를 동시에 선언
typedef struct ASTNode {
    int type; // NODE_DECL, NODE_ASSIGN 등
    union {
        struct { char name[50]; int size; } decl;
        struct { char name[50]; struct ASTNode* value; } assign;
        struct { struct ASTNode* stmt; struct ASTNode* next; } list;
        char var_name[50];
        int num_value;
    } data;
} ASTNode;

// 빌드 에러에 언급된 enum 구조도 명시적으로 존재해야 합니다.
typedef enum {
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_LIST,
    NODE_DECL,
    NODE_ASSIGN,
    NODE_VAR,
    NODE_PRINT
} NodeType;

// 추상 구문 트리 생성 함수들...
ASTNode* create_block_node(ASTNode* stmt);
ASTNode* create_list_node(ASTNode* stmt);
ASTNode* append_to_list(ASTNode* head, ASTNode* tail);
ASTNode* create_moca_decl_node(char* name, int size, int init);
ASTNode* create_assignment_node(char* name, ASTNode* value);
ASTNode* create_print_node(char* var_name);
ASTNode* create_var_node(char* name);
ASTNode* create_num_node(int val);
void free_ast(ASTNode* node);

#endif
