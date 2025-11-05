cmake_minimum_required(VERSION 3.16)

# Frontend module for language parsing
add_library(frontend STATIC
    LanguageDetector.cpp
    FrontendBase.cpp
    CFrontend.cpp
    JavaFrontend.cpp
    PythonFrontend.cpp
)

target_include_directories(frontend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(frontend ${llvm_libs})

# Compiler flags for frontend
target_compile_options(frontend PRIVATE -Wall -Wextra -g)
