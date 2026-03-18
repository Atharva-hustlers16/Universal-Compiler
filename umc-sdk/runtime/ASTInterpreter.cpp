#include "ASTInterpreter.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

ASTInterpreter::ASTInterpreter() {
    // Built-in functions are handled separately, no storage needed
}

bool ASTInterpreter::execute(const ASTNode& ast) {
    try {
        evaluateNode(ast);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return false;
    }
}

std::any ASTInterpreter::getVariable(const std::string& name) const {
    auto it = variables_.find(name);
    if (it == variables_.end()) {
        throw std::runtime_error("Undefined variable: " + name);
    }
    return it->second;
}

void ASTInterpreter::setVariable(const std::string& name, const std::any& value) {
    variables_[name] = value;
}

std::any ASTInterpreter::evaluateNode(const ASTNode& node) {
    switch (node.getType()) {
        case ASTNodeType::PROGRAM:
            for (const auto& child : node.getChildren()) {
                evaluateNode(*child);
            }
            return std::any();
            
        case ASTNodeType::FUNCTION_DECL:
            // For now, just skip function declarations
            return std::any();
            
        case ASTNodeType::FUNCTION_CALL:
            return evaluateFunctionCall(node);
            
        case ASTNodeType::RETURN_STMT:
            return evaluateReturnStmt(node);
            
        case ASTNodeType::VARIABLE_DECL:
            return evaluateVariableDecl(node);
            
        case ASTNodeType::BINARY_OP:
            return evaluateBinaryOp(node);
            
        case ASTNodeType::LITERAL:
            return evaluateLiteral(node);
            
        case ASTNodeType::IDENTIFIER:
            return evaluateIdentifier(node);
            
        default:
            // For unknown node types, just process children
            for (const auto& child : node.getChildren()) {
                evaluateNode(*child);
            }
            return std::any();
    }
}

std::any ASTInterpreter::evaluateFunctionCall(const ASTNode& node) {
    std::string name = node.getValue();
    
    // Extract function name from call expression
    if (name.find('(') != std::string::npos) {
        name = name.substr(0, name.find('('));
    }
    
    // Evaluate arguments
    std::vector<std::any> args;
    for (const auto& child : node.getChildren()) {
        args.push_back(evaluateNode(*child));
    }
    
    return callFunction(name, args);
}

std::any ASTInterpreter::evaluateReturnStmt(const ASTNode& node) {
    if (!node.getChildren().empty()) {
        return evaluateNode(*node.getChildren()[0]);
    }
    return std::any();
}

std::any ASTInterpreter::evaluateVariableDecl(const ASTNode& node) {
    std::string name = node.getValue();
    
    // Extract variable name from declaration
    if (name.find(' ') != std::string::npos) {
        name = name.substr(name.find_last_of(' ') + 1);
    }
    
    if (!node.getChildren().empty()) {
        std::any value = evaluateNode(*node.getChildren()[0]);
        setVariable(name, value);
        return value;
    }
    
    setVariable(name, std::any());
    return std::any();
}

std::any ASTInterpreter::evaluateBinaryOp(const ASTNode& node) {
    if (node.getChildren().size() < 2) {
        return std::any();
    }
    
    std::any left = evaluateNode(*node.getChildren()[0]);
    std::any right = evaluateNode(*node.getChildren()[1]);
    
    std::string op = node.getValue();
    
    if (op == "+") {
        if (left.type() == typeid(int) && right.type() == typeid(int)) {
            return std::any(std::any_cast<int>(left) + std::any_cast<int>(right));
        }
        if (left.type() == typeid(double) || right.type() == typeid(double)) {
            return std::any(toDouble(left) + toDouble(right));
        }
        return std::any(toString(left) + toString(right));
    }
    else if (op == "-") {
        if (left.type() == typeid(int) && right.type() == typeid(int)) {
            return std::any(std::any_cast<int>(left) - std::any_cast<int>(right));
        }
        return std::any(toDouble(left) - toDouble(right));
    }
    else if (op == "*") {
        if (left.type() == typeid(int) && right.type() == typeid(int)) {
            return std::any(std::any_cast<int>(left) * std::any_cast<int>(right));
        }
        return std::any(toDouble(left) * toDouble(right));
    }
    else if (op == "/") {
        return std::any(toDouble(left) / toDouble(right));
    }
    
    return std::any();
}

std::any ASTInterpreter::evaluateLiteral(const ASTNode& node) {
    std::string value = node.getValue();
    
    // Try to parse as integer
    try {
        size_t pos;
        int intValue = std::stoi(value, &pos);
        if (pos == value.size()) {
            return std::any(intValue);
        }
    } catch (...) {
        // Not an integer
    }
    
    // Try to parse as double
    try {
        size_t pos;
        double doubleValue = std::stod(value, &pos);
        if (pos == value.size()) {
            return std::any(doubleValue);
        }
    } catch (...) {
        // Not a double
    }
    
    // Handle string literals
    if (value.front() == '"' && value.back() == '"') {
        return std::any(value.substr(1, value.size() - 2));
    }
    
    // Return as string
    return std::any(value);
}

std::any ASTInterpreter::evaluateIdentifier(const ASTNode& node) {
    return getVariable(node.getValue());
}

std::any ASTInterpreter::callFunction(const std::string& name, const std::vector<std::any>& args) {
    // Check for built-in functions first
    if (isBuiltinFunction(name)) {
        return callBuiltinFunction(name, args);
    }
    
    // For now, just print that we're calling a user function
    std::cout << "Calling user function: " << name << " with " << args.size() << " args" << std::endl;
    return std::any();
}

bool ASTInterpreter::isBuiltinFunction(const std::string& name) const {
    return name == "print" || name == "printf" || name == "str" || name == "int" || name == "float" || name == "len";
}

std::any ASTInterpreter::callBuiltinFunction(const std::string& name, const std::vector<std::any>& args) {
    if (name == "print" || name == "printf") {
        for (const auto& arg : args) {
            std::cout << toString(arg);
        }
        std::cout << std::endl;
        return std::any();
    }
    else if (name == "str") {
        if (args.empty()) {
            return std::any(std::string(""));
        }
        return std::any(toString(args[0]));
    }
    else if (name == "int") {
        if (args.empty()) {
            return std::any(0);
        }
        try {
            if (args[0].type() == typeid(std::string)) {
                return std::any(std::stoi(std::any_cast<std::string>(args[0])));
            }
            return std::any(toInt(args[0]));
        } catch (...) {
            return std::any(0);
        }
    }
    else if (name == "float") {
        if (args.empty()) {
            return std::any(0.0);
        }
        try {
            if (args[0].type() == typeid(std::string)) {
                return std::any(std::stod(std::any_cast<std::string>(args[0])));
            }
            return std::any(toDouble(args[0]));
        } catch (...) {
            return std::any(0.0);
        }
    }
    else if (name == "len") {
        if (args.empty()) {
            return std::any(0);
        }
        if (args[0].type() == typeid(std::string)) {
            return std::any(static_cast<int>(std::any_cast<std::string>(args[0]).length()));
        }
        return std::any(0);
    }
    
    throw std::runtime_error("Unknown built-in function: " + name);
}

std::string ASTInterpreter::toString(const std::any& value) const {
    if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    }
    if (value.type() == typeid(int)) {
        return std::to_string(std::any_cast<int>(value));
    }
    if (value.type() == typeid(double)) {
        return std::to_string(std::any_cast<double>(value));
    }
    return "";
}

double ASTInterpreter::toDouble(const std::any& value) const {
    if (value.type() == typeid(double)) {
        return std::any_cast<double>(value);
    }
    if (value.type() == typeid(int)) {
        return static_cast<double>(std::any_cast<int>(value));
    }
    return std::stod(toString(value));
}

int ASTInterpreter::toInt(const std::any& value) const {
    if (value.type() == typeid(int)) {
        return std::any_cast<int>(value);
    }
    if (value.type() == typeid(double)) {
        return static_cast<int>(std::any_cast<double>(value));
    }
    try {
        return std::stoi(toString(value));
    } catch (...) {
        return 0;
    }
}
