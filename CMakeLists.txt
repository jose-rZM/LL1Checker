cmake_minimum_required(VERSION 3.16)
project(LL1Checker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SRC_DIR src)
set(HPP_DIR include)
set(OBJ_DIR out)

find_package(Boost REQUIRED COMPONENTS regex)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/grammar.cpp
    ${SRC_DIR}/symbol_table.cpp
    ${SRC_DIR}/lexer.cpp
    ${SRC_DIR}/ll1_parser.cpp
)

set(HEADERS
    ${HPP_DIR}/grammar.hpp
    ${HPP_DIR}/symbol_table.hpp
    ${HPP_DIR}/lexer.hpp
    ${HPP_DIR}/ll1_parser.hpp
)

add_executable(ll1 ${SOURCES} ${HEADERS})

set(Boost_USE_STATIC_LIBS ON)
target_link_libraries(ll1 PRIVATE Boost::regex)

set_target_properties(ll1 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OBJ_DIR}
)