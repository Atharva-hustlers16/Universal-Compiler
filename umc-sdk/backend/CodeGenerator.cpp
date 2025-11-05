#include "CodeGenerator.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <iostream>
#include <fstream>

CodeGenerator::CodeGenerator() {
    initializeTarget();
}

CodeGenerator::~CodeGenerator() = default;

bool CodeGenerator::initializeTarget() {
    // Initialize LLVM targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // Initialize native target for JIT
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // Find target for host
    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string error;

    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        std::cerr << "Error: " << error << std::endl;
        return false;
    }

    // Create target machine
    llvm::TargetOptions opt;
    setTargetOptions();

    llvm::Optional<llvm::Reloc::Model> relocModel = llvm::Optional<llvm::Reloc::Model>();
    targetMachine_ = std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(targetTriple, "generic", "", opt, relocModel));

    return targetMachine_ != nullptr;
}

void CodeGenerator::setTargetOptions() {
    // Set optimization options for code generation
    // These can be customized based on requirements
}

bool CodeGenerator::generateCode(std::unique_ptr<llvm::Module> module, const std::string& outputFile) {
    if (!targetMachine_) {
        std::cerr << "Error: Target machine not initialized" << std::endl;
        return false;
    }

    if (!module) {
        std::cerr << "Error: No module to generate code from" << std::endl;
        return false;
    }

    std::error_code EC;
    llvm::raw_fd_ostream dest(outputFile, EC, llvm::sys::fs::OF_None);

    if (EC) {
        std::cerr << "Error: Could not open file: " << EC.message() << std::endl;
        return false;
    }

    llvm::legacy::PassManager pass;
    auto fileType = llvm::CGFT_ObjectFile;

    if (targetMachine_->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        std::cerr << "Error: TargetMachine can't emit a file of this type" << std::endl;
        return false;
    }

    pass.run(*module);
    dest.flush();

    std::cout << "Generated object code: " << outputFile << std::endl;
    return true;
}

bool CodeGenerator::generateObjectCode(std::unique_ptr<llvm::Module> module, const std::string& outputFile) {
    return generateCode(std::move(module), outputFile);
}

bool CodeGenerator::generateExecutable(std::unique_ptr<llvm::Module> module, const std::string& outputFile) {
    if (!module) {
        std::cerr << "Error: No module to generate executable from" << std::endl;
        return false;
    }

    // For demonstration, we'll use JIT compilation
    // In a real implementation, you'd link with runtime libraries

    std::cout << "Generating executable using JIT compilation..." << std::endl;

    // Create execution engine for JIT
    std::string error;
    llvm::EngineBuilder engineBuilder(std::move(module));
    engineBuilder.setEngineKind(llvm::EngineKind::JIT);
    engineBuilder.setErrorStr(&error);

    llvm::ExecutionEngine* executionEngine = engineBuilder.create();
    if (!executionEngine) {
        std::cerr << "Error creating execution engine: " << error << std::endl;
        return false;
    }

    // Look for main function
    llvm::Function* mainFunc = executionEngine->FindFunctionNamed("main");
    if (!mainFunc) {
        std::cerr << "Error: No main function found" << std::endl;
        delete executionEngine;
        return false;
    }

    std::cout << "Found main function, preparing for execution..." << std::endl;

    // Execute the function (simplified - no arguments)
    std::vector<llvm::GenericValue> args;
    llvm::GenericValue result = executionEngine->runFunction(mainFunc, args);

    std::cout << "JIT execution completed with result: " << result.IntVal.getSExtValue() << std::endl;

    delete executionEngine;
    return true;
}

bool CodeGenerator::generateAndExecute(std::unique_ptr<llvm::Module> module, int argc, char** argv) {
    if (!module) {
        std::cerr << "Error: No module to execute" << std::endl;
        return false;
    }

    // Use JIT execution for testing
    return generateExecutable(std::move(module), "jit_execution");
}

std::string CodeGenerator::getTargetTriple() const {
    if (targetMachine_) {
        return targetMachine_->getTargetTriple().str();
    }
    return "";
}

std::string CodeGenerator::getTargetCPU() const {
    if (targetMachine_) {
        return targetMachine_->getTargetCPU().str();
    }
    return "";
}

std::string CodeGenerator::getTargetFeatures() const {
    if (targetMachine_) {
        return targetMachine_->getTargetFeatureString().str();
    }
    return "";
}
