cmake_minimum_required(VERSION 3.16)

project(LL1Checker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp 
    src/ll1_parser.cpp 
    src/symbol_table.cpp 
    src/lexer.cpp 
    src/grammar.cpp)

add_executable(ll1 ${SOURCES})

target_include_directories(ll1 PRIVATE include)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if (MSVC)
    target_compile_options(ll1 PRIVATE /W4)
else()
    target_compile_options(ll1 PRIVATE -Wall -Wextra -pedantic -O3)
endif()