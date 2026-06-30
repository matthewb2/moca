// Moca v0.1.2 Transpiled Native C Code
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("%s\n", "Hello Moca Engine!");
    void* p1 = malloc(16);
    void* p2 = malloc(16);
    p2 = p1;
    free(p1); // Moca 정적 수명 제어에 의한 자동 해제
    free(p2); // Moca 정적 수명 제어에 의한 자동 해제
    return 0;
}
