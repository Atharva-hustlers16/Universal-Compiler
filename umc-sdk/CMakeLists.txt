cmake_minimum_required(VERSION 3.16)
project(UMC-SDK VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Find ANTLR runtime (optional, for Java frontend)
find_package(ANTLR4Runtime QUIET)

# Find Tree-sitter (optional, for Python frontend)
find_package(TreeSitter QUIET)

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Create executable for the main compiler
add_executable(ucc cli/main.cpp)

# Create executable for the runtime
add_executable(urun runtime/main.cpp)

# Create executable for tests
add_executable(test_umc tests/test_main.cpp)

# Include directories
include_directories(include)
include_directories(generated/antlr)  # For ANTLR generated files

# Add subdirectories for modular build
add_subdirectory(frontend)
add_subdirectory(ir)
add_subdirectory(backend)
add_subdirectory(runtime)
add_subdirectory(tests)

# Create ANTLR generated parser library (if ANTLR is available)
if(ANTLR4Runtime_FOUND)
    message(STATUS "ANTLR4 Runtime found - Java frontend will use generated parser")

    # Generate ANTLR parser from grammar (this would be done at build time)
    # For now, assume generated files are present

    # Add ANTLR generated files to Java frontend
    target_sources(ucc PRIVATE
        generated/antlr/JavaLexer.cpp
        generated/antlr/JavaParser.cpp
        generated/antlr/JavaParserBaseVisitor.cpp
        generated/antlr/JavaParserVisitor.cpp
    )

    # Link ANTLR runtime
    target_link_libraries(ucc antlr4-runtime)
    target_link_libraries(test_umc antlr4-runtime)

    # Define ANTLR available
    target_compile_definitions(ucc PRIVATE ANTLR_AVAILABLE)
    target_compile_definitions(test_umc PRIVATE ANTLR_AVAILABLE)

else()
    message(STATUS "ANTLR4 Runtime not found - Java frontend will use fallback parser")
endif()

# Tree-sitter integration for Python frontend
if(TreeSitter_FOUND)
    message(STATUS "Tree-sitter found - Python frontend will use generated parser")

    # Link Tree-sitter
    target_link_libraries(ucc tree-sitter)
    target_link_libraries(test_umc tree-sitter)

    # Define Tree-sitter available
    target_compile_definitions(ucc PRIVATE TREESITTER_AVAILABLE)
    target_compile_definitions(test_umc PRIVATE TREESITTER_AVAILABLE)

    # Include Tree-sitter Python language (assumes it's built/linked)
    # In a real implementation, you'd build tree-sitter-python as a library

else()
    message(STATUS "Tree-sitter not found - Python frontend will use fallback parser")
endif()

# Link LLVM libraries
llvm_map_components_to_libnames(llvm_libs
    Core
    ExecutionEngine
    Interpreter
    MC
    MCJIT
    Support
    nativecodegen
    Analysis
    Passes
    TransformUtils
    ScalarOpts
    Vectorize
    InstCombine
    Target
    X86
    ARM
)

# Link libraries to executables
target_link_libraries(ucc ${llvm_libs})
target_link_libraries(urun ${llvm_libs})
target_link_libraries(test_umc ${llvm_libs})

# Compiler flags
target_compile_options(ucc PRIVATE -Wall -Wextra -g)
target_compile_options(urun PRIVATE -Wall -Wextra -g)
target_compile_options(test_umc PRIVATE -Wall -Wextra -g)

# Enable testing
enable_testing()

# Print LLVM information
message(STATUS "Found LLVM ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH}")
message(STATUS "LLVM libraries: ${llvm_libs}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")

# Print ANTLR status
if(ANTLR4Runtime_FOUND)
    message(STATUS "ANTLR4 Runtime: ${ANTLR4Runtime_VERSION}")
else()
    message(STATUS "ANTLR4 Runtime: Not found (Java frontend will use fallback)")
endif()
