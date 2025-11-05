cmake_minimum_required(VERSION 3.16)

# Tests module
add_executable(test_umc
    test_main.cpp
    test_language_detector.cpp
    test_frontend.cpp
    test_ir_generator.cpp
    test_optimizer.cpp
    test_code_generator.cpp
)

target_include_directories(test_umc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(test_umc ${llvm_libs} frontend ir backend runtime_lib)

# Compiler flags for tests
target_compile_options(test_umc PRIVATE -Wall -Wextra -g)

# Register tests
add_test(NAME test_umc COMMAND test_umc)
