#!/bin/bash
# build.sh
set -e

# 컴파일러 엔진 빌드
mkdir -p build && cd build && cmake .. && make && cd ..

# 1. 정상 작동 케이스 테스트 (Hello World + 안전한 메모리 운용)
echo -e "\n[Moca Test] Case 1: 정상 소스 빌드 시도"
cat << 'EOF' > input_good.moca
fn main() {
    println!("Hello Moca Engine!");
    let:16 p1;
    let:16 p2;
    p2 = p1;
}
EOF

./build/moca_compiler input_good.moca
gcc -O3 moca_output.c -o moca_good_bin
./moca_good_bin

# 2. 버퍼 오버플로우 발생 케이스 테스트
echo -e "\n[Moca Test] Case 2: 버퍼 오버플로우 탐지 테스트"
cat << 'EOF' > input_bad.moca
fn main() {
    let:8 p2;
    let:16 p1;
    p2 = p1; # ❌ 8바이트 공간에 16바이트 복사 감지선 걸림
}
EOF

echo "-> 아래 에러가 정상 출력되면 정적 방어 시스템이 성공적으로 연동된 것입니다:"
./build/moca_compiler input_bad.moca || true
