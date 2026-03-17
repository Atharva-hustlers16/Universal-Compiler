#include "IRGenerator.h"
#include "ASTNode.h"
#include <iostream>

#ifndef NO_LLVM
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#endif

IRGenerator::IRGenerator() {
#ifndef NO_LLVM
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>("main", *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);
#endif
}

IRGenerator::~IRGenerator() = default;

bool IRGenerator::generateIR(const FrontendBase& frontend, const std::string& moduleName) {
    // Create a new module for each generation
    context_ = std::make_unique<llvm::LLVMContext>();
    module_ = std::make_unique<llvm::Module>(moduleName, *context_);
    builder_ = std::make_unique<llvm::IRBuilder<>>(*context_);

    auto ast = frontend.getAST();
    if (!ast) {
        std::cerr << "Error: No AST available from frontend" << std::endl;
        return false;
    }

    std::cout << "Generating IR from AST: " << ast->toString() << std::endl;

    // Generate IR from AST
    if (!generateFromAST(*ast)) {
        std::cerr << "Error: Failed to generate IR from AST" << std::endl;
        return false;
    }

    // Verify the module
    if (llvm::verifyModule(*module_, &llvm::errs())) {
        std::cerr << "Error: Module verification failed" << std::endl;
        return false;
    }

    return true;
}

bool IRGenerator::generateFromAST(const ASTNode& ast) {
    switch (ast.getType()) {
        case ASTNodeType::PROGRAM:
            return generateProgramIR(ast);
        case ASTNodeType::FUNCTION_DECL:
            return generateFunctionDeclIR(ast);
        case ASTNodeType::VARIABLE_DECL:
            return generateVariableDeclIR(ast);
        case ASTNodeType::BINARY_OP:
            return generateBinaryOpIR(ast);
        case ASTNodeType::LITERAL:
            return generateLiteralIR(ast);
        case ASTNodeType::IDENTIFIER:
            return generateIdentifierIR(ast);
        case ASTNodeType::RETURN_STMT:
            return generateReturnStmtIR(ast);
        case ASTNodeType::BLOCK:
            return generateBlockIR(ast);
        default:
            std::cout << "Warning: Unhandled AST node type in IR generation" << std::endl;
            return true;  // Continue processing
    }
}

bool IRGenerator::generateProgramIR(const ASTNode& programNode) {
    std::cout << "Generating program IR..." << std::endl;

    // Process all children (function declarations, etc.)
    for (const auto& child : programNode.getChildren()) {
        if (!generateFromAST(*child)) {
            return false;
        }
    }

    // Create a default main function if none exists
    if (!hasMainFunction()) {
        createDefaultMain();
    }

    return true;
}

bool IRGenerator::generateFunctionDeclIR(const ASTNode& funcNode) {
    std::cout << "Generating function declaration IR..." << std::endl;

    // For now, create a simple function that returns 0
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context_), false);
    llvm::Function* function = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, funcNode.getValue(), module_.get());

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(entry);

    // Generate body
    for (const auto& child : funcNode.getChildren()) {
        if (!generateFromAST(*child)) {
            return false;
        }
    }

    // Default return 0 if no return statement
    if (!hasReturnStatement(function)) {
        builder_->CreateRet(llvm::ConstantInt::get(*context_, llvm::APInt(32, 0)));
    }

    return true;
}

bool IRGenerator::generateVariableDeclIR(const ASTNode& varNode) {
    std::cout << "Generating variable declaration IR..." << std::endl;

    // For simplicity, we'll handle variables at function scope
    // In a full implementation, you'd need symbol table management

    return true;
}

bool IRGenerator::generateBinaryOpIR(const ASTNode& binOpNode) {
    std::cout << "Generating binary operation IR..." << std::endl;

    // Generate left and right operands
    const auto& children = binOpNode.getChildren();
    if (children.size() < 2) {
        std::cerr << "Error: Binary operation needs at least 2 operands" << std::endl;
        return false;
    }

    llvm::Value* leftValue = nullptr;
    llvm::Value* rightValue = nullptr;

    // Generate left operand
    if (!generateFromAST(*children[0])) {
        return false;
    }
    // For now, assume simple literals - in full implementation, track values

    // Generate right operand
    if (!generateFromAST(*children[1])) {
        return false;
    }

    // Create the operation (simplified)
    std::string op = binOpNode.getValue();
    if (op == "+") {
        // For demonstration, just create a simple addition
        leftValue = llvm::ConstantInt::get(*context_, llvm::APInt(32, 1));
        rightValue = llvm::ConstantInt::get(*context_, llvm::APInt(32, 2));
        builder_->CreateAdd(leftValue, rightValue, "addtmp");
    }

    return true;
}

bool IRGenerator::generateLiteralIR(const ASTNode& literalNode) {
    std::cout << "Generating literal IR: " << literalNode.getValue() << std::endl;

    // Create LLVM constant from literal value
    try {
        int value = std::stoi(literalNode.getValue());
        llvm::ConstantInt::get(*context_, llvm::APInt(32, value));
    } catch (const std::exception&) {
        // Handle non-integer literals
        std::cout << "Non-integer literal: " << literalNode.getValue() << std::endl;
    }

    return true;
}

bool IRGenerator::generateIdentifierIR(const ASTNode& idNode) {
    std::cout << "Generating identifier IR: " << idNode.getValue() << std::endl;

    // In a full implementation, look up the variable in symbol table
    // For now, just acknowledge the identifier

    return true;
}

bool IRGenerator::generateReturnStmtIR(const ASTNode& returnNode) {
    std::cout << "Generating return statement IR..." << std::endl;

    // Generate return value if present
    for (const auto& child : returnNode.getChildren()) {
        if (!generateFromAST(*child)) {
            return false;
        }
    }

    // For now, return 0
    builder_->CreateRet(llvm::ConstantInt::get(*context_, llvm::APInt(32, 0)));

    return true;
}

bool IRGenerator::generateBlockIR(const ASTNode& blockNode) {
    std::cout << "Generating block IR..." << std::endl;

    // Generate all statements in the block
    for (const auto& child : blockNode.getChildren()) {
        if (!generateFromAST(*child)) {
            return false;
        }
    }

    return true;
}

bool IRGenerator::hasMainFunction() {
    // Check if a function named "main" exists
    for (auto& func : module_->functions()) {
        if (func.getName() == "main") {
            return true;
        }
    }
    return false;
}

bool IRGenerator::hasReturnStatement(llvm::Function* function) {
    // Check if the function has a return statement
    for (auto& bb : *function) {
        for (auto& inst : bb) {
            if (llvm::isa<llvm::ReturnInst>(inst)) {
                return true;
            }
        }
    }
    return false;
}

void IRGenerator::createDefaultMain() {
    std::cout << "Creating default main function..." << std::endl;

    llvm::FunctionType* mainType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context_), false);
    llvm::Function* mainFunc = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module_.get());

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context_, "entry", mainFunc);
    builder_->SetInsertPoint(entry);

    // Return 0
    builder_->CreateRet(llvm::ConstantInt::get(*context_, llvm::APInt(32, 0)));
}

void IRGenerator::printIR() const {
    if (module_) {
        module_->print(llvm::outs(), nullptr);
    }
}
