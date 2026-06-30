// src/codegen.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

static FILE* out = NULL;
static int indent_level = 0;

static void print_indent() {
    for (int i = 0; i < indent_level; i++) fprintf(out, "    ");
}

void init_codegen(FILE* output_file) {
    out = output_file;
    indent_level = 0;
    fprintf(out, "// Moca v0.1.2 Transpiled Native C Code\n");
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n\n");
    fprintf(out, "int main() {\n");
    indent_level++;
}

void finalize_codegen() {
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");
    out = NULL;
}

void generate_code(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_DECL:
            print_indent();
            fprintf(out, "void* %s = malloc(%d);\n", node->data.decl.name, node->data.decl.size);
            break;

        case NODE_ASSIGN:
            print_indent();
            if (node->data.assign.value && node->data.assign.value->type == NODE_VAR) {
                fprintf(out, "%s = %s;\n", node->data.assign.name, node->data.assign.value->data.var_name);
            }
            break;

        case NODE_PRINT:
            print_indent();
            fprintf(out, "printf(\"%%s\\n\", \"%s\");\n", node->data.var_name);
            break;

        case NODE_BLOCK: {
            // 블록 내부 구문 생성
            generate_code(node->data.list.stmt);

            // [메모리 누수 원천 차단] 스코프 종료 직전 선언된 자원 자동 해제 로직 주입
            ASTNode* curr = node->data.list.stmt;
            while (curr && curr->type == NODE_LIST) {
                ASTNode* stmt = curr->data.list.stmt;
                if (stmt && stmt->type == NODE_DECL) {
                    print_indent();
                    fprintf(out, "free(%s); // Moca 정적 수명 제어에 의한 자동 해제\n", stmt->data.decl.name);
                }
                curr = curr->data.list.next;
            }
            break;
        }

        case NODE_LIST:
        case NODE_PROGRAM:
            generate_code(node->data.list.stmt);
            generate_code(node->data.list.next);
            break;

        default:
            break;
    }
}
