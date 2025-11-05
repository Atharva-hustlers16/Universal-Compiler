#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>
#include <string>
#include <memory>

class CodeGenerator {
public:
    CodeGenerator();
    ~CodeGenerator();

    bool generateCode(std::unique_ptr<llvm::Module> module, const std::string& outputFile);
    bool generateObjectCode(std::unique_ptr<llvm::Module> module, const std::string& outputFile);

    std::string getTargetTriple() const;
    std::string getTargetCPU() const;

private:
    std::unique_ptr<llvm::TargetMachine> targetMachine_;

    bool initializeTarget();
    void setTargetOptions();
};

#endif // CODE_GENERATOR_H
