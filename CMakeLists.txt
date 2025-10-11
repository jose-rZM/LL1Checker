cmake_minimum_required(VERSION 3.16)

project(LL1Checker LANGUAGES CXX)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LL1CHECKER_FORCE_STATIC_RUNTIME
       "Link C++ runtime and dependencies statically in Release builds" OFF)

if(LL1CHECKER_FORCE_STATIC_RUNTIME)
    set(BUILD_SHARED_LIBS OFF)
    if(MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        add_link_options($<$<CONFIG:Release>:-static>
                         $<$<CONFIG:Release>:-static-libstdc++>
                         $<$<CONFIG:Release>:-static-libgcc>)
    endif()
    set(Boost_USE_STATIC_LIBS ON)
endif()

set(LIB_NAME ll1_core)
set(EXECUTABLE ll1)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(Boost 1.78 REQUIRED CONFIG COMPONENTS headers)

find_package(cxxopts QUIET CONFIG)
if(NOT cxxopts_FOUND)
    find_path(CXXOPTS_INCLUDE_DIR cxxopts.hpp)
    if(CXXOPTS_INCLUDE_DIR)
        add_library(cxxopts INTERFACE)
        add_library(cxxopts::cxxopts ALIAS cxxopts)
        target_include_directories(cxxopts INTERFACE ${CXXOPTS_INCLUDE_DIR})
    else()
        include(FetchContent)
        FetchContent_Declare(
            cxxopts
            GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
            GIT_TAG        v3.1.1
        )
        set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
        FetchContent_MakeAvailable(cxxopts)
        if(TARGET cxxopts)
            add_library(cxxopts::cxxopts ALIAS cxxopts)
        endif()
    endif()
endif()

add_subdirectory(src)
add_subdirectory(app)
