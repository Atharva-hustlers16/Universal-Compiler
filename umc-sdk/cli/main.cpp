#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "LanguageDetector.h"
#include "FrontendBase.h"
#include "ASTNode.h"

#ifdef ENABLE_LLVM
#include "IRGenerator.h"
#include "Optimizer.h"
#include "CodeGenerator.h"
#endif
#include "Runtime.h"

std::unique_ptr<FrontendBase> createFrontend(Language lang);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ucc <source_file> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -o <output>    Specify output file" << std::endl;
        std::cerr << "  -O <level>     Optimization level (0-3)" << std::endl;
        std::cerr << "  --emit-llvm    Emit LLVM IR instead of object code" << std::endl;
        std::cerr << "  --run          Execute parsed AST directly (no compilation)" << std::endl;
        std::cerr << "Example: ucc hello.c" << std::endl;
        std::cerr << "Example: ucc hello.py --run" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string outputFile = "a.out";
    int optLevel = 0;
    bool emitLLVM = false;
    bool runAST = false;

    // Parse command line arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "-O" && i + 1 < argc) {
            optLevel = std::stoi(argv[++i]);
        } else if (arg == "--emit-llvm") {
            emitLLVM = true;
        } else if (arg == "--run") {
            runAST = true;
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

    // If --run flag is specified, execute AST directly
    if (runAST) {
        if (!ast) {
            std::cerr << "Error: No AST available for execution" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== AST Execution Mode ===" << std::endl;
        Runtime runtime;
        if (runtime.executeAST(*ast)) {
            std::cout << "AST execution completed successfully" << std::endl;
            return 0;
        } else {
            std::cerr << "AST execution failed" << std::endl;
            return 1;
        }
    }

#ifdef ENABLE_LLVM
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
#ifdef ENABLE_LLVM
        Optimizer optimizer;
        auto module = irGenerator.getModule();
        if (optimizer.optimize(std::move(module))) {
            std::cout << "\n=== Optimized IR ===" << std::endl;
            optimizer.printOptimizedIR();
        }
#else
        std::cout << "\n=== Optimization Disabled ===" << std::endl;
        std::cout << "Optimization requires LLVM support." << std::endl;
#endif
    }

    // Generate code if not emitting LLVM
    if (!emitLLVM) {
#ifdef ENABLE_LLVM
        CodeGenerator codeGen;
        std::unique_ptr<llvm::Module> finalModule;
        
        if (optLevel > 0) {
            // Get optimized module
            Optimizer optimizer;
            auto module = irGenerator.getModule();
            if (optimizer.optimize(std::move(module))) {
                finalModule = optimizer.getOptimizedModule();
            }
        } else {
            // Get unoptimized module
            finalModule = irGenerator.getModule();
        }

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
                }
            }
        }
#else
        std::cout << "\n=== LLVM Support Disabled ===" << std::endl;
        std::cout << "Parsing completed successfully, but code generation requires LLVM support." << std::endl;
        std::cout << "Rebuild with LLVM integration enabled to use full compilation pipeline." << std::endl;
        return 0;
#endif
    }
#else
    std::cout << "\n=== LLVM Support Disabled ===" << std::endl;
    std::cout << "Parsing completed successfully, but code generation requires LLVM support." << std::endl;
    std::cout << "Rebuild with -DENABLE_LLVM=ON to enable full compilation pipeline." << std::endl;
#endif

    return 0;
}
