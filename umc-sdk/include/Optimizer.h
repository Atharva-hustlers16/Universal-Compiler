#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <llvm/IR/Module.h>
#include <llvm/Passes/PassBuilder.h>
#include <memory>

class Optimizer {
public:
    Optimizer();
    ~Optimizer();

    bool optimize(std::unique_ptr<llvm::Module> module);
    bool optimize(std::unique_ptr<llvm::Module> module, int level);
    std::unique_ptr<llvm::Module> getOptimizedModule() const { return std::move(optimizedModule_); }

    void printOptimizedIR() const;
    void printOptimizationStats() const;
    void setOptimizationLevel(int level);

private:
    std::unique_ptr<llvm::Module> optimizedModule_;
    int optLevel_ = 2;  // Default to O2

    llvm::ModulePassManager createO0Pipeline(llvm::PassBuilder& PB, llvm::LoopAnalysisManager& LAM,
                                           llvm::FunctionAnalysisManager& FAM, llvm::CGSCCAnalysisManager& CGAM,
                                           llvm::ModuleAnalysisManager& MAM);
    llvm::ModulePassManager createO1Pipeline(llvm::PassBuilder& PB, llvm::LoopAnalysisManager& LAM,
                                           llvm::FunctionAnalysisManager& FAM, llvm::CGSCCAnalysisManager& CGAM,
                                           llvm::ModuleAnalysisManager& MAM);
    llvm::ModulePassManager createO2Pipeline(llvm::PassBuilder& PB, llvm::LoopAnalysisManager& LAM,
                                           llvm::FunctionAnalysisManager& FAM, llvm::CGSCCAnalysisManager& CGAM,
                                           llvm::ModuleAnalysisManager& MAM);
    llvm::ModulePassManager createO3Pipeline(llvm::PassBuilder& PB, llvm::LoopAnalysisManager& LAM,
                                           llvm::FunctionAnalysisManager& FAM, llvm::CGSCCAnalysisManager& CGAM,
                                           llvm::ModuleAnalysisManager& MAM);
};

#endif // OPTIMIZER_H
