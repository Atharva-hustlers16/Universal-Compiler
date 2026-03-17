cmake_minimum_required(VERSION 3.16)

# IR module for intermediate representation
add_library(ir STATIC
    IRGenerator.cpp
    IRModule.cpp
)

target_include_directories(ir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
if(LLVM_FOUND)
    target_link_libraries(ir ${llvm_libs} frontend)
else()
    target_link_libraries(ir frontend)
endif()

# Compiler flags for IR module
if(MSVC)
    target_compile_options(ir PRIVATE /W4)
else()
    target_compile_options(ir PRIVATE -Wall -Wextra -g)
endif()
