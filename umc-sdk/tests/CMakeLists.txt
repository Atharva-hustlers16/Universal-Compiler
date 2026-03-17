cmake_minimum_required(VERSION 3.16)

# Tests module
add_executable(test_umc
    test_main.cpp
    test_language_detector.cpp
    test_frontend.cpp
    test_ir_generator.cpp
    test_optimizer.cpp
    test_code_generator.cpp
    TestValidator.cpp
    TestFramework.cpp
)

# Include directories
target_include_directories(test_umc 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Link libraries - note: the actual static lib is "backend_lib", not "backend"
target_link_libraries(test_umc 
    ${llvm_libs} 
    frontend 
    ir 
    backend_lib
    runtime_lib
)

# Add ANTLR runtime if available
if(ANTLR4_FOUND)
    target_link_libraries(test_umc ${ANTLR4_LIBRARY})
    target_compile_definitions(test_umc PRIVATE ANTLR_AVAILABLE)
endif()

# Add Tree-sitter if available
if(TREE_SITTER_FOUND)
    target_link_libraries(test_umc ${TREE_SITTER_LIBRARY})
    target_compile_definitions(test_umc PRIVATE TREESITTER_AVAILABLE)
endif()

# Compiler flags
if(MSVC)
    target_compile_options(test_umc PRIVATE /W3)
    target_compile_definitions(test_umc PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(test_umc PRIVATE -Wall -Wextra -g)
endif()

# Register tests with CTest
enable_testing()
add_test(NAME test_umc COMMAND test_umc)
