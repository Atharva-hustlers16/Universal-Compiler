cmake_minimum_required(VERSION 3.16)

# Backend module for code generation
add_library(backend STATIC
    Optimizer.cpp
    CodeGenerator.cpp
)

target_include_directories(backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(backend ${llvm_libs} ir)

# Compiler flags for backend module
target_compile_options(backend PRIVATE -Wall -Wextra -g)
