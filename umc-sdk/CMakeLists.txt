cmake_minimum_required(VERSION 3.16)
project(UMC-SDK-Full)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add MSYS2 paths
set(MSYS2_PREFIX "C:/msys64/mingw64")
set(CMAKE_PREFIX_PATH ${MSYS2_PREFIX} ${CMAKE_PREFIX_PATH})

# Add LLVM path - prioritize MSVC installation
set(LLVM_DIR "C:/Program Files/LLVM/lib/cmake/llvm")
set(CMAKE_PREFIX_PATH "C:/Program Files/LLVM" ${CMAKE_PREFIX_PATH})

# Clear any existing LLVM paths to avoid conflicts
unset(LLVM_FOUND CACHE)
unset(LLVM_DIR CACHE)

# Include directories
include_directories(include)
include_directories(${MSYS2_PREFIX}/include)
include_directories(${MSYS2_PREFIX}/include/tree_sitter)
include_directories("C:/Program Files/LLVM/include")

# Find packages - manual LLVM configuration
set(LLVM_FOUND TRUE)
set(LLVM_INCLUDE_DIRS "C:/Program Files/LLVM/include")
set(LLVM_LIBRARY_DIRS "C:/Program Files/LLVM/lib")
set(LLVM_LIBRARY_DIR "C:/Program Files/LLVM/lib")
message(STATUS "Using manual LLVM configuration from: C:/Program Files/LLVM")

# Set LLVM libraries manually
set(LLVM_LIBS
    "C:/Program Files/LLVM/lib/LLVM-C.lib"
    "C:/Program Files/LLVM/lib/libclang.lib"
)

set(llvm_libs ${LLVM_LIBS})

# Check for Tree-sitter
if(EXISTS "${MSYS2_PREFIX}/include/tree_sitter/api.h")
    set(TREE_SITTER_FOUND TRUE)
    set(TREE_SITTER_INCLUDE_DIR "${MSYS2_PREFIX}/include/tree_sitter")
    set(TREE_SITTER_LIBRARY "${MSYS2_PREFIX}/lib/libtree-sitter.dll.a")
    message(STATUS "Found Tree-sitter: ${TREE_SITTER_INCLUDE_DIR}")
else()
    message(WARNING "Tree-sitter not found")
    set(TREE_SITTER_FOUND FALSE)
endif()

# Check for ANTLR4
find_path(ANTLR4_INCLUDE_DIR antlr4-runtime.h PATHS ${MSYS2_PREFIX}/include)
find_library(ANTLR4_LIBRARY antlr4-runtime PATHS ${MSYS2_PREFIX}/lib)
if(ANTLR4_INCLUDE_DIR AND ANTLR4_LIBRARY)
    set(ANTLR4_FOUND TRUE)
    message(STATUS "Found ANTLR4: ${ANTLR4_LIBRARY}")
else()
    message(WARNING "ANTLR4 not found")
    set(ANTLR4_FOUND FALSE)
endif()

# Define source files based on available dependencies
set(FRONTEND_SOURCES
    "frontend/ASTNode.cpp"
    "frontend/FrontendBase.cpp"
    "frontend/LanguageDetector.cpp"
    "frontend/CFrontend.cpp"
    "frontend/JavaFrontend.cpp"
    "frontend/PythonFrontend.cpp"
)

set(BACKEND_SOURCES 
    "backend/CodeGenerator.cpp"
    "backend/Optimizer.cpp"
)

set(IR_SOURCES 
    "ir/IRGenerator.cpp"
    "ir/IRModule.cpp"
)

set(OPTIMIZER_SOURCES 
    "frontend/ASTOptimizer.cpp"
)

set(RUNTIME_SOURCES
    "runtime/ASTInterpreter_simple.cpp"
    "runtime/MemoryManager.cpp"
    "runtime/Runtime.cpp"
)

# Add enhanced frontends if dependencies are available
if(ANTLR4_FOUND)
    # list(APPEND FRONTEND_SOURCES "frontend/EnhancedJavaFrontend.cpp")
endif()

if(TREE_SITTER_FOUND)
    list(APPEND FRONTEND_SOURCES "frontend/TreeSitterFrontend.cpp")
    list(APPEND FRONTEND_SOURCES "frontend/EnhancedPythonFrontend.cpp")
else()
    list(APPEND FRONTEND_SOURCES "frontend/EnhancedPythonFrontend.cpp")
endif()

# Add optimizer (commented out due to compilation issues)
# list(APPEND FRONTEND_SOURCES "frontend/ASTOptimizer.cpp")

set(CLI_SOURCES
    "cli/main.cpp"
)

# Create libraries
add_library(frontend STATIC ${FRONTEND_SOURCES})
add_library(runtime_lib STATIC ${RUNTIME_SOURCES})
if(LLVM_FOUND)
    add_library(backend_lib STATIC ${BACKEND_SOURCES} ${IR_SOURCES} ${OPTIMIZER_SOURCES})
endif()

# Create executable
add_executable(ucc ${CLI_SOURCES})

# Link libraries
if(LLVM_FOUND)
    target_link_libraries(ucc PRIVATE frontend runtime_lib backend_lib)
else()
    target_link_libraries(ucc PRIVATE frontend runtime_lib)
endif()

# Add dependency libraries
# Note: LLVM detected but minimal installation - linking enabled
if(LLVM_FOUND)
    target_link_libraries(ucc PRIVATE ${llvm_libs})
    target_link_libraries(frontend PRIVATE ${llvm_libs})
    
    # Add threading support for LLVM on Windows
    target_link_libraries(ucc PRIVATE kernel32)
endif()

if(TREE_SITTER_FOUND)
    target_link_libraries(ucc PRIVATE ${TREE_SITTER_LIBRARY})
    target_link_libraries(frontend PRIVATE ${TREE_SITTER_LIBRARY})
    target_include_directories(frontend PRIVATE ${TREE_SITTER_INCLUDE_DIR})
endif()

if(ANTLR4_FOUND)
    target_link_libraries(ucc PRIVATE ${ANTLR4_LIBRARY})
    target_link_libraries(frontend PRIVATE ${ANTLR4_LIBRARY})
    target_include_directories(frontend PRIVATE ${ANTLR4_INCLUDE_DIR})
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(ucc PRIVATE /W3)
    target_compile_options(frontend PRIVATE /W3)
    target_compile_options(runtime_lib PRIVATE /W3)
    if(LLVM_FOUND)
        target_compile_options(backend_lib PRIVATE /W3)
    endif()
    
    # Disable specific warnings for MSVC
    target_compile_definitions(ucc PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(frontend PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(runtime_lib PRIVATE _CRT_SECURE_NO_WARNINGS)
    if(LLVM_FOUND)
        target_compile_definitions(backend_lib PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endif()

# Add definitions for optional dependencies
if(TREE_SITTER_FOUND)
    target_compile_definitions(frontend PRIVATE ENABLE_TREE_SITTER)
endif()

if(ANTLR4_FOUND)
    target_compile_definitions(frontend PRIVATE ENABLE_ANTLR)
endif()

# Note: LLVM detected but minimal installation - ENABLE_LLVM enabled
if(LLVM_FOUND)
    target_compile_definitions(ucc PRIVATE ENABLE_LLVM)
    target_compile_definitions(backend_lib PRIVATE ENABLE_LLVM)
    target_link_libraries(backend_lib PRIVATE frontend runtime_lib ${llvm_libs})
    target_include_directories(backend_lib PRIVATE include "C:/Program Files/LLVM/include")
endif()

# Add ir as an alias so test_umc can link against it
# (IR sources are compiled into backend_lib — expose as interface)
add_library(ir INTERFACE)
target_link_libraries(ir INTERFACE backend_lib)

# Include tests
enable_testing()
add_subdirectory(tests)
