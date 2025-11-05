#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "LanguageDetector.h"
#include "FrontendBase.h"
#include "IRGenerator.h"
#include "Optimizer.h"
#include "CodeGenerator.h"
#include "Runtime.h"

std::unique_ptr<FrontendBase> createFrontend(Language lang);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ucc <source_file> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -o <output>    Specify output file" << std::endl;
        std::cerr << "  -O <level>     Optimization level (0-3)" << std::endl;
        std::cerr << "  --emit-llvm    Emit LLVM IR instead of object code" << std::endl;
        std::cerr << "Example: ucc hello.c" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string outputFile = "a.out";
    int optLevel = 0;
    bool emitLLVM = false;

    // Parse command line arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "-O" && i + 1 < argc) {
            optLevel = std::stoi(argv[++i]);
        } else if (arg == "--emit-llvm") {
            emitLLVM = true;
        }
    }

    // Check if file exists
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return 1;
    }

    // Read source code
    std::string sourceCode((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

    // Detect language
    Language lang = LanguageDetector::detectLanguage(filename);
    if (lang == Language::UNKNOWN) {
        std::cerr << "Error: Could not detect language for file: " << filename << std::endl;
        return 1;
    }

    std::cout << "Detected language: " << LanguageDetector::languageToString(lang) << std::endl;

    // Create appropriate frontend
    auto frontend = createFrontend(lang);
    if (!frontend) {
        std::cerr << "Error: Could not create frontend for language: "
                  << LanguageDetector::languageToString(lang) << std::endl;
        return 1;
    }

    // Parse source code
    if (!frontend->parse(sourceCode)) {
        std::cerr << "Error: Failed to parse source code" << std::endl;
        return 1;
    }

    // Display AST if available
    auto ast = frontend->getAST();
    if (ast) {
        std::cout << "\n=== Generated AST ===" << std::endl;
        std::cout << ast->toString() << std::endl;
    }

    // Generate IR
    IRGenerator irGenerator;
    std::string moduleName = filename.substr(0, filename.find_last_of('.'));
    if (!irGenerator.generateIR(*frontend, moduleName)) {
        std::cerr << "Error: Failed to generate IR" << std::endl;
        return 1;
    }

    // Print generated IR
    std::cout << "\n=== Generated LLVM IR ===" << std::endl;
    irGenerator.printIR();

    // Optimize if requested
    if (optLevel > 0) {
        Optimizer optimizer;
        auto module = irGenerator.getModule();
        if (optimizer.optimize(std::move(module))) {
            std::cout << "\n=== Optimized IR ===" << std::endl;
            optimizer.printOptimizedIR();
        }
    }

    // Generate code if not emitting LLVM
    if (!emitLLVM) {
        CodeGenerator codeGen;
        auto finalModule = optLevel > 0 ?
            optimizer.getOptimizedModule() : irGenerator.getModule();

        if (finalModule) {
            // Try JIT execution first for testing
            if (codeGen.generateAndExecute(std::move(finalModule), argc, argv)) {
                std::cout << "\n=== JIT Execution Complete ===" << std::endl;
                std::cout << "Program executed successfully via JIT" << std::endl;
            } else {
                // Fallback to object file generation
                if (codeGen.generateObjectCode(std::move(finalModule), outputFile)) {
                    std::cout << "\n=== Object Code Generation Complete ===" << std::endl;
                    std::cout << "Generated: " << outputFile << std::endl;

                    // Try to run with runtime system
                    Runtime runtime;
                    if (runtime.loadExecutable(outputFile)) {
                        if (runtime.execute(argc, argv)) {
                            std::cout << "Runtime execution successful" << std::endl;
                        }
                    }
                }
            }
        }
    }

    return 0;
}
