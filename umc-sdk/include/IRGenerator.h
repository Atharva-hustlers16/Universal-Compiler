#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#ifdef NO_LLVM
// LLVM is disabled - provide stub implementation
#include <memory>
#include "FrontendBase.h"

class IRGenerator {
public:
    IRGenerator() = default;
    ~IRGenerator() = default;

    bool generateIR(const FrontendBase& frontend, const std::string& moduleName = "main") {
        (void)frontend; (void)moduleName;
        return false; // No LLVM support
    }

    void printIR() const {
        // No-op when LLVM is disabled
    }
};

#else
// LLVM is enabled - provide full implementation
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <memory>
#include "FrontendBase.h"

class IRGenerator {
public:
    IRGenerator();
    ~IRGenerator();

    bool generateIR(const FrontendBase& frontend, const std::string& moduleName = "main");
    std::unique_ptr<llvm::Module> getModule() { return std::move(module_); }

    void printIR() const;

private:
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    bool generateFromAST(const ASTNode& ast);
    bool generateProgramIR(const ASTNode& programNode);
    bool generateFunctionDeclIR(const ASTNode& funcNode);
    bool generateVariableDeclIR(const ASTNode& varNode);
    bool generateBinaryOpIR(const ASTNode& binOpNode);
    bool generateLiteralIR(const ASTNode& literalNode);
    bool generateIdentifierIR(const ASTNode& idNode);
    bool generateReturnStmtIR(const ASTNode& returnNode);
    bool generateBlockIR(const ASTNode& blockNode);
    bool hasMainFunction();
    bool hasReturnStatement(llvm::Function* function);
    void createDefaultMain();
};
#endif

#endif // IR_GENERATOR_H
