#ifndef AST_INTERPRETER_H
#define AST_INTERPRETER_H

#include "ASTNode.h"
#include <string>
#include <map>
#include <vector>
#include <any>

class ASTInterpreter {
public:
    ASTInterpreter();
    ~ASTInterpreter() = default;

    bool execute(const ASTNode& ast);
    std::any getVariable(const std::string& name) const;
    void setVariable(const std::string& name, const std::any& value);

private:
    std::map<std::string, std::any> variables_;
    std::map<std::string, std::pair<std::vector<std::string>, std::unique_ptr<ASTNode>>> functions_;

    std::any evaluateNode(const ASTNode& node);
    std::any evaluateFunctionDecl(const ASTNode& node);
    std::any evaluateFunctionCall(const ASTNode& node);
    std::any evaluateReturnStmt(const ASTNode& node);
    std::any evaluateVariableDecl(const ASTNode& node);
    std::any evaluateBinaryOp(const ASTNode& node);
    std::any evaluateLiteral(const ASTNode& node);
    std::any evaluateIdentifier(const ASTNode& node);
    std::any evaluateIfStmt(const ASTNode& node);
    std::any evaluateForLoop(const ASTNode& node);
    std::any evaluateWhileLoop(const ASTNode& node);
    std::any evaluateBlock(const ASTNode& node);

    std::any callFunction(const std::string& name, const std::vector<std::any>& args);
    bool isBuiltinFunction(const std::string& name) const;
    std::any callBuiltinFunction(const std::string& name, const std::vector<std::any>& args);

    // Type conversion helpers
    int toInt(const std::any& value) const;
    std::string toString(const std::any& value) const;
    double toDouble(const std::any& value) const;
};

#endif // AST_INTERPRETER_H
