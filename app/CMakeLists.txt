add_executable(${EXECUTABLE} main.cpp)
target_link_libraries(${EXECUTABLE} PRIVATE ${LIB_NAME})
target_include_directories(${EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/include)