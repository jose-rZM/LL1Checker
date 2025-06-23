add_library(${LIB_NAME}
    ll1_parser.cpp
    symbol_table.cpp
    lexer.cpp
    grammar.cpp
)

target_include_directories(${LIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(${LIB_NAME} PRIVATE /W4)
else()
    target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -pedantic -O3)
endif()