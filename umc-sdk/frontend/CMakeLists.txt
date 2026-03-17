cmake_minimum_required(VERSION 3.16)

# Frontend module for language parsing
add_library(frontend STATIC
    ASTNode.cpp
    LanguageDetector.cpp
    FrontendBase.cpp
    CFrontend.cpp
    JavaFrontend.cpp
    EnhancedJavaFrontend.cpp
    PythonFrontend.cpp
    EnhancedPythonFrontend.cpp
    TreeSitterFrontend.cpp
    ASTOptimizer.cpp
)

target_include_directories(frontend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(frontend ${llvm_libs})

# Add ANTLR4 if available
if(ANTLR4Runtime_FOUND)
    target_include_directories(frontend PRIVATE ${ANTLR4_INCLUDE_DIR})
    target_link_libraries(frontend PRIVATE ${ANTLR4_LIBRARY})
endif()

# Add Tree-sitter if available
if(TREE_SITTER_FOUND)
    target_include_directories(frontend PRIVATE ${TREE_SITTER_INCLUDE_DIR})
    target_link_libraries(frontend PRIVATE ${TREE_SITTER_LIBRARY})
endif()

# Compiler flags for frontend
if(MSVC)
    target_compile_options(frontend PRIVATE /W4 /WX)
else()
    target_compile_options(frontend PRIVATE -Wall -Wextra -g)
endif()
