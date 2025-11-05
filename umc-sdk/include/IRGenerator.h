#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <memory>
#include "FrontendBase.h"

class IRGenerator {
public:
    IRGenerator();
    ~IRGenerator();

    bool generateIR(const FrontendBase& frontend, const std::string& moduleName = "main");
    std::unique_ptr<llvm::Module> getModule() const { return std::move(module_); }

    void printIR() const;

private:
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;

    bool generateFromAST(const ASTNode& ast);
};

#endif // IR_GENERATOR_H
