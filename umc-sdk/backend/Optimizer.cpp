#include "Optimizer.h"
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/SCCP.h>
#include <llvm/Transforms/Scalar/DeadStoreElimination.h>
#include <llvm/Transforms/Scalar/EarlyCSE.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/IPO/Inliner.h>
#include <llvm/Transforms/IPO/DeadArgumentElimination.h>
#include <llvm/Transforms/IPO/GlobalDCE.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Scalar/LoopUnrollPass.h>
#include <llvm/Transforms/Scalar/LoopDeletion.h>
#include <llvm/Transforms/Scalar/LoopInstSimplify.h>
#include <llvm/Transforms/Scalar/LoopRotate.h>
#include <llvm/Transforms/Scalar/LoopSimplifyCFG.h>
#include <iostream>

Optimizer::Optimizer() = default;
Optimizer::~Optimizer() = default;

bool Optimizer::optimize(std::unique_ptr<llvm::Module> module) {
    if (!module) {
        std::cerr << "Error: No module to optimize" << std::endl;
        return false;
    }

    optimizedModule_ = std::move(module);

    std::cout << "Running comprehensive optimization passes..." << std::endl;

    // Create analysis managers
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // Register analysis passes
    llvm::PassBuilder PB;

    // Register standard instrumentations
    llvm::StandardInstrumentations SI(*optimizedModule_->getContext(),
                                    /*DebugLogging*/ true);
    SI.registerCallbacks(PB, &MAM);

    // Register analysis passes
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.registerCGSCCAnalyses(CGAM);

    // Create optimization pipeline based on level
    llvm::ModulePassManager MPM;

    switch (optLevel_) {
        case 0:
            MPM = createO0Pipeline(PB, LAM, FAM, CGAM, MAM);
            break;
        case 1:
            MPM = createO1Pipeline(PB, LAM, FAM, CGAM, MAM);
            break;
        case 2:
            MPM = createO2Pipeline(PB, LAM, FAM, CGAM, MAM);
            break;
        case 3:
            MPM = createO3Pipeline(PB, LAM, FAM, CGAM, MAM);
            break;
        default:
            std::cout << "Warning: Unknown optimization level, using O2" << std::endl;
            MPM = createO2Pipeline(PB, LAM, FAM, CGAM, MAM);
            break;
    }

    // Run the passes
    MPM.run(*optimizedModule_, MAM);

    std::cout << "Optimization completed with level " << optLevel_ << std::endl;
    return true;
}

bool Optimizer::optimize(std::unique_ptr<llvm::Module> module, int level) {
    optLevel_ = level;
    return optimize(std::move(module));
}

void Optimizer::setOptimizationLevel(int level) {
    optLevel_ = level;
}

llvm::ModulePassManager Optimizer::createO0Pipeline(
    llvm::PassBuilder& PB,
    llvm::LoopAnalysisManager& LAM,
    llvm::FunctionAnalysisManager& FAM,
    llvm::CGSCCAnalysisManager& CGAM,
    llvm::ModuleAnalysisManager& MAM) {

    llvm::ModulePassManager MPM;

    // Basic function passes for O0
    llvm::FunctionPassManager FPM;
    FPM.addPass(llvm::PromotePass());  // Mem2Reg

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
    return MPM;
}

llvm::ModulePassManager Optimizer::createO1Pipeline(
    llvm::PassBuilder& PB,
    llvm::LoopAnalysisManager& LAM,
    llvm::FunctionAnalysisManager& FAM,
    llvm::CGSCCAnalysisManager& CGAM,
    llvm::ModuleAnalysisManager& MAM) {

    llvm::ModulePassManager MPM;

    // Function passes for O1
    llvm::FunctionPassManager FPM;
    FPM.addPass(llvm::PromotePass());  // Mem2Reg
    FPM.addPass(llvm::InstCombinePass());  // Instruction combining
    FPM.addPass(llvm::SimplifyCFGPass());  // Simplify control flow

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

    // Module passes for O1
    MPM.addPass(llvm::GlobalDCEPass());  // Dead global elimination

    return MPM;
}

llvm::ModulePassManager Optimizer::createO2Pipeline(
    llvm::PassBuilder& PB,
    llvm::LoopAnalysisManager& LAM,
    llvm::FunctionAnalysisManager& FAM,
    llvm::CGSCCAnalysisManager& CGAM,
    llvm::ModuleAnalysisManager& MAM) {

    llvm::ModulePassManager MPM;

    // Function passes for O2
    llvm::FunctionPassManager FPM;
    FPM.addPass(llvm::PromotePass());  // Mem2Reg
    FPM.addPass(llvm::InstCombinePass());  // Instruction combining
    FPM.addPass(llvm::ReassociatePass());  // Reassociation
    FPM.addPass(llvm::GVNPass());  // Global value numbering
    FPM.addPass(llvm::SimplifyCFGPass());  // Simplify control flow
    FPM.addPass(llvm::DeadStoreEliminationPass());  // Dead store elimination

    // Loop passes for O2
    llvm::LoopPassManager LPM;
    LPM.addPass(llvm::LoopInstSimplifyPass());
    LPM.addPass(llvm::LoopSimplifyCFGPass());
    LPM.addPass(llvm::LoopRotatePass());
    LPM.addPass(llvm::LoopUnrollPass());  // Loop unrolling

    FPM.addPass(llvm::createFunctionToLoopPassAdaptor(std::move(LPM)));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

    // Module passes for O2
    MPM.addPass(llvm::GlobalDCEPass());  // Dead global elimination
    MPM.addPass(llvm::ConstantMergePass());  // Constant merging

    return MPM;
}

llvm::ModulePassManager Optimizer::createO3Pipeline(
    llvm::PassBuilder& PB,
    llvm::LoopAnalysisManager& LAM,
    llvm::FunctionAnalysisManager& FAM,
    llvm::CGSCCAnalysisManager& CGAM,
    llvm::ModuleAnalysisManager& MAM) {

    llvm::ModulePassManager MPM;

    // Function passes for O3 (aggressive optimization)
    llvm::FunctionPassManager FPM;
    FPM.addPass(llvm::PromotePass());  // Mem2Reg
    FPM.addPass(llvm::InstCombinePass());  // Instruction combining
    FPM.addPass(llvm::ReassociatePass());  // Reassociation
    FPM.addPass(llvm::GVNPass());  // Global value numbering
    FPM.addPass(llvm::SCCPPass());  // Sparse conditional constant propagation
    FPM.addPass(llvm::DeadStoreEliminationPass());  // Dead store elimination
    FPM.addPass(llvm::EarlyCSEPass());  // Early common subexpression elimination
    FPM.addPass(llvm::SimplifyCFGPass());  // Simplify control flow

    // Aggressive loop optimizations for O3
    llvm::LoopPassManager LPM;
    LPM.addPass(llvm::LoopInstSimplifyPass());
    LPM.addPass(llvm::LoopSimplifyCFGPass());
    LPM.addPass(llvm::LoopRotatePass());
    LPM.addPass(llvm::LoopUnrollPass(llvm::LoopUnrollOptions().setPartialThreshold(300)));
    LPM.addPass(llvm::LoopDeletionPass());

    FPM.addPass(llvm::createFunctionToLoopPassAdaptor(std::move(LPM)));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));

    // Module passes for O3
    MPM.addPass(llvm::GlobalDCEPass());  // Dead global elimination
    MPM.addPass(llvm::ConstantMergePass());  // Constant merging
    MPM.addPass(llvm::DeadArgumentEliminationPass());  // Dead argument elimination

    return MPM;
}

void Optimizer::printOptimizedIR() const {
    if (optimizedModule_) {
        std::cout << "\n=== Optimized IR (Level " << optLevel_ << ") ===" << std::endl;
        optimizedModule_->print(llvm::outs(), nullptr);
    }
}

void Optimizer::printOptimizationStats() const {
    if (!optimizedModule_) return;

    std::cout << "\n=== Optimization Statistics ===" << std::endl;
    std::cout << "Functions: " << optimizedModule_->getFunctionList().size() << std::endl;
    std::cout << "Global Variables: " << optimizedModule_->getGlobalList().size() << std::endl;

    // Count basic blocks and instructions
    size_t totalBlocks = 0;
    size_t totalInstructions = 0;

    for (auto& func : *optimizedModule_) {
        totalBlocks += func.getBasicBlockList().size();
        for (auto& bb : func) {
            totalInstructions += bb.getInstList().size();
        }
    }

    std::cout << "Basic Blocks: " << totalBlocks << std::endl;
    std::cout << "Instructions: " << totalInstructions << std::endl;
    std::cout << "Optimization Level: " << optLevel_ << std::endl;
}
