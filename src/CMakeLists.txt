add_library(${LIB_NAME}
    ll1_parser.cpp
    symbol_table.cpp
    lexer.cpp
    grammar.cpp
)

target_include_directories(${LIB_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${LIB_NAME} PUBLIC Boost::headers)

if(MSVC)
    target_compile_options(${LIB_NAME} PRIVATE /W4)
else()
    target_compile_options(${LIB_NAME} PRIVATE
        $<$<CONFIG:Debug>:-Wall -Wextra -pedantic -g>
        $<$<CONFIG:Release>:-Wall -Wextra -pedantic -O3>
    )
endif()
