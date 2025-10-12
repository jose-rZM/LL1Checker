add_executable(${EXECUTABLE} main.cpp)
target_link_libraries(${EXECUTABLE} PRIVATE ${LIB_NAME} cxxopts::cxxopts)
target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_SOURCE_DIR}/include)

if(LL1CHECKER_FORCE_STATIC_RUNTIME AND NOT MSVC)
    target_link_options(${EXECUTABLE} PRIVATE
        $<$<CONFIG:Release>:-static>
        $<$<CONFIG:Release>:-static-libstdc++>
        $<$<CONFIG:Release>:-static-libgcc>)
endif()
