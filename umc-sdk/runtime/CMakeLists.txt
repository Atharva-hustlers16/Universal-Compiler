cmake_minimum_required(VERSION 3.16)

# Runtime module for execution
add_library(runtime_lib STATIC
    Runtime.cpp
    MemoryManager.cpp
    ASTInterpreter.cpp
)

target_include_directories(runtime_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Compiler flags for runtime module
if(MSVC)
    target_compile_options(runtime_lib PRIVATE /W4 /WX)
else()
    target_compile_options(runtime_lib PRIVATE -Wall -Wextra -g)
endif()
