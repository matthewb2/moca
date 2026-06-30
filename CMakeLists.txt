# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(MocaCompiler C)

# 컴파일러 플래그 설정 (-O3 최적화 및 경고 활성화)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall")

# 시스템에 설치된 Flex & Bison 패키지 로드
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# 소스 파일 경로 지정 (src 디렉터리 내부)
set(LEXER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l)
set(PARSER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y)

# Flex & Bison 자동 빌드 규칙 및 출력 파일 정의
FLEX_TARGET(MocaLexer ${LEXER_SRC} ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
BISON_TARGET(MocaParser ${PARSER_SRC} ${CMAKE_CURRENT_BINARY_DIR}/parser.c)

# 헤더 파일 탐색 경로 포함 (src 디렉터리와 빌드 임시 디렉터리)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

# 모든 모듈을 하나로 엮어 최종 Moca 컴파일러 실행 파일 생성
add_executable(moca_compiler
    ${BISON_MocaParser_OUTPUTS}
    ${FLEX_MocaLexer_OUTPUTS}    
    src/ast.c
    src/ownership.c
    src/codegen.c
    src/main.c
)
