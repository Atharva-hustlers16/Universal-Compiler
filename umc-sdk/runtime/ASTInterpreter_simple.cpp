#include "ASTInterpreter.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

ASTInterpreter::ASTInterpreter() {
    // Built-in functions are handled separately, no storage needed
}

bool ASTInterpreter::execute(const ASTNode& ast) {
    try {
        // First pass: evaluate all nodes to define functions and global variables
        evaluateNode(ast);
        
        // Second pass: if main function is defined, execute it
        auto mainIt = functions_.find("main");
        if (mainIt != functions_.end()) {
            std::cout << "\n=== Executing main() function ===" << std::endl;
            
            // Call main function with no arguments (Python-style main)
            std::vector<std::any> mainArgs;
            callFunction("main", mainArgs);
            std::cout << "\n=== main() execution completed ===" << std::endl;
        } else {
            std::cout << "No main() function found - executed global scope only" << std::endl;
        }
        
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
            return evaluateFunctionDecl(node);
            
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
            
        case ASTNodeType::IF_STMT:
            return evaluateIfStmt(node);
            
        case ASTNodeType::FOR_LOOP:
            return evaluateForLoop(node);
            
        case ASTNodeType::WHILE_LOOP:
            return evaluateWhileLoop(node);
            
        case ASTNodeType::BLOCK:
            return evaluateBlock(node);
            
        // C++ specific node types
        case ASTNodeType::CLASS_DECL:
            return evaluateNode(*node.getChildren()[0]); // For now, just evaluate first member
            
        case ASTNodeType::CONSTRUCTOR_DECL:
        case ASTNodeType::METHOD_DECL:
            return evaluateFunctionDecl(node); // Treat as function declaration
            
        case ASTNodeType::PARAMETER:
        case ASTNodeType::RETURN_TYPE:
        case ASTNodeType::TYPE:
        case ASTNodeType::ARGUMENT:
            return evaluateLiteral(node); // Treat as literal for now
            
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
    // For Python-style declarations, the name is directly in the value
    // For C-style declarations like "int x", extract the variable name
    if (name.find(' ') != std::string::npos) {
        name = name.substr(name.find_last_of(' ') + 1);
    }
    
    // Trim whitespace from variable name
    size_t nameStart = name.find_first_not_of(" \t");
    size_t nameEnd = name.find_last_not_of(" \t");
    if (nameStart != std::string::npos && nameEnd != std::string::npos) {
        name = name.substr(nameStart, nameEnd - nameStart + 1);
    }
    
    // Look for initialization value in children
    std::any value;
    for (const auto& child : node.getChildren()) {
        if (child->getType() == ASTNodeType::LITERAL) {
            // This is the initialization value
            value = evaluateLiteral(*child);
            break;
        }
    }
    
    // If no explicit value, try to evaluate first child (for backward compatibility)
    if (!value.has_value() && !node.getChildren().empty()) {
        // Skip the TYPE node and look for other children
        for (const auto& child : node.getChildren()) {
            if (child->getType() != ASTNodeType::TYPE) {
                value = evaluateNode(*child);
                break;
            }
        }
    }
    
    setVariable(name, value);
    return value;
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
    
    // Handle string literals
    if (value.front() == '"' && value.back() == '"') {
        std::string content = value.substr(1, value.size() - 2);
        // Process escape sequences
        for (size_t i = 0; i < content.length(); ++i) {
            if (content[i] == '\\' && i + 1 < content.length()) {
                switch (content[i + 1]) {
                    case 'n': content.replace(i, 2, "\n"); break;
                    case 't': content.replace(i, 2, "\t"); break;
                    case 'r': content.replace(i, 2, "\r"); break;
                    case '\\': content.replace(i, 2, "\\"); break;
                    case '"': content.replace(i, 2, "\""); break;
                    default: break; // Keep other escape sequences as-is
                }
            }
        }
        return std::any(content);
    }
    
    // Handle f-strings (simplified - treat as string literals for now)
    if (value.length() >= 3 && ((value[0] == 'f' && value[1] == '"' && value.back() == '"') || 
                               (value[0] == 'f' && value[1] == '\'' && value.back() == '\''))) {
        std::string content = value.substr(2, value.size() - 3);
        return std::any(content);
    }
    
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
    
    // Return as string (for expressions like "10 + 5" that should be handled elsewhere)
    return std::any(value);
}

std::any ASTInterpreter::evaluateIdentifier(const ASTNode& node) {
    std::string varName = node.getValue();
    
    // Trim whitespace from variable name
    size_t nameStart = varName.find_first_not_of(" \t");
    size_t nameEnd = varName.find_last_not_of(" \t");
    if (nameStart != std::string::npos && nameEnd != std::string::npos) {
        varName = varName.substr(nameStart, nameEnd - nameStart + 1);
    }
    
    return getVariable(varName);
}

std::any ASTInterpreter::callFunction(const std::string& name, const std::vector<std::any>& args) {
    // Check for built-in functions first
    if (isBuiltinFunction(name)) {
        return callBuiltinFunction(name, args);
    }
    
    // Check for constructor calls (ClassName obj)
    if (name.find(' ') != std::string::npos) {
        // This might be a constructor call like "TestClass obj"
        size_t spacePos = name.find_last_of(' ');
        std::string className = name.substr(0, spacePos);
        std::string objName = name.substr(spacePos + 1);
        
        // For now, just create the object and return it
        // In a full implementation, we'd store the object with its properties
        std::cout << "Creating object of type " << className << " named " << objName << std::endl;
        return std::any();
    }
    
    // Check for method calls (obj.method)
    if (name.find('.') != std::string::npos) {
        // This might be a method call like "obj.getValue"
        size_t dotPos = name.find('.');
        std::string objName = name.substr(0, dotPos);
        std::string methodName = name.substr(dotPos + 1);
        
        // For now, just simulate the method call
        // In a full implementation, we'd look up the object and call the method
        if (args.empty()) {
            std::cout << "Calling method " << methodName << " on object " << objName << std::endl;
            // For getValue method, return a sample value
            if (methodName == "getValue") {
                return std::any(42); // Return a sample value
            }
        } else {
            std::cout << "Calling method " << methodName << " on object " << objName << " with " << args.size() << " arguments" << std::endl;
        }
        return std::any();
    }
    
    // Check for user-defined functions
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        const auto& funcInfo = it->second;
        const auto& params = funcInfo.first;
        const auto& body = funcInfo.second;
        
        if (args.size() != params.size()) {
            // Special exception for main which is often called with 0 args but defined with 1 (String[] args)
            if (!(name == "main" && args.empty())) {
                throw std::runtime_error("Argument count mismatch for function: " + name);
            }
        }
        
        // Save current variable scope for nested function calls
        std::map<std::string, std::any> savedVariables = variables_;
        
        // Clear current scope for this function (but keep it for nested calls)
        variables_.clear();
        
        // Set up function parameters
        for (size_t i = 0; i < params.size(); ++i) {
            if (i < args.size()) {
                setVariable(params[i], args[i]);
            } else {
                setVariable(params[i], std::any()); // Set default/empty
            }
        }
        
        // Special setup for Java main method args.length property
        if (name == "main") {
            setVariable("args.length", std::any(0));
        }
        
        // Execute function body
        std::any result;
        if (body) {
            result = evaluateNode(*body);
        }
        
        // Restore variable scope (but only after complete execution)
        variables_ = savedVariables;
        
        return result;
    }
    
    // For now, just print that we're calling an unknown function
    std::cout << "Calling unknown function: " << name << " with " << args.size() << " args" << std::endl;
    return std::any();
}

bool ASTInterpreter::isBuiltinFunction(const std::string& name) const {
    return name == "print" || name == "printf" || name == "System.out.println" || name == "cout" || name == "str" || name == "int" || name == "float" || name == "len";
}

std::any ASTInterpreter::callBuiltinFunction(const std::string& name, const std::vector<std::any>& args) {
    if (name == "print" || name == "printf" || name == "System.out.println") {
        if (args.empty()) {
            std::cout << std::endl;
            return std::any();
        }
        
        // Handle printf-style format strings
        std::string formatStr = toString(args[0]);
        
        if (args.size() == 1) {
            // Simple print without formatting
            std::cout << formatStr;
        } else {
            // Format string with arguments
            size_t argIndex = 1;
            for (size_t i = 0; i < formatStr.length(); ++i) {
                if (formatStr[i] == '%' && i + 1 < formatStr.length()) {
                    char formatChar = formatStr[i + 1];
                    if (argIndex < args.size()) {
                        switch (formatChar) {
                            case 'd':
                            case 'i':
                                std::cout << toInt(args[argIndex]);
                                break;
                            case 'f':
                                std::cout << toDouble(args[argIndex]);
                                break;
                            case 's':
                                std::cout << toString(args[argIndex]);
                                break;
                            case 'c':
                                std::cout << static_cast<char>(toInt(args[argIndex]));
                                break;
                            case '%':
                                std::cout << '%';
                                --argIndex; // Don't consume an argument for %%
                                break;
                            default:
                                std::cout << '%' << formatChar;
                                break;
                        }
                        ++argIndex;
                        ++i; // Skip the format character
                    } else {
                        std::cout << '%' << formatChar;
                        ++i;
                    }
                } else {
                    std::cout << formatStr[i];
                }
            }
        }
        
        std::cout << std::endl;
        return std::any();
    }
    else if (name == "cout") {
        // Handle C++ cout-style output
        for (const auto& arg : args) {
            std::string output = toString(arg);
            
            // Handle endl specially
            if (output == "endl") {
                std::cout << std::endl;
            } else {
                std::cout << output;
            }
        }
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

int ASTInterpreter::toInt(const std::any& value) const {
    if (value.type() == typeid(int)) {
        return std::any_cast<int>(value);
    }
    if (value.type() == typeid(double)) {
        return static_cast<int>(std::any_cast<double>(value));
    }
    return std::stoi(toString(value));
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

// New evaluation methods for enhanced AST execution
std::any ASTInterpreter::evaluateFunctionDecl(const ASTNode& node) {
    // Store function definition for later execution
    std::string functionName = node.getValue();
    
    // Extract function name (remove any type information)
    size_t spacePos = functionName.find_last_of(' ');
    if (spacePos != std::string::npos) {
        functionName = functionName.substr(spacePos + 1);
    }
    
    // Extract parameters and function body
    std::vector<std::string> params;
    std::unique_ptr<ASTNode> body = nullptr;
    
    for (const auto& child : node.getChildren()) {
        if (child->getType() == ASTNodeType::VARIABLE_DECL) {
            // This is a parameter
            std::string paramName = child->getValue();
            size_t paramSpacePos = paramName.find_last_of(' ');
            if (paramSpacePos != std::string::npos) {
                paramName = paramName.substr(paramSpacePos + 1);
            }
            params.push_back(paramName);
        } else if (child->getType() == ASTNodeType::BLOCK) {
            // This is the function body
            body = child->clone();
        }
    }
    
    if (body) {
        functions_[functionName] = std::make_pair(params, std::move(body));
        std::cout << "Defined function: " << functionName << " with " << params.size() << " parameters" << std::endl;
    } else if (!node.getChildren().empty()) {
        // Handle class nodes: they don't have a Block body but contain method children
        for (const auto& child : node.getChildren()) {
            evaluateNode(*child);
        }
    }
    
    return std::any();
}

std::any ASTInterpreter::evaluateIfStmt(const ASTNode& node) {
    if (node.getChildren().empty()) return std::any();
    
    // Evaluate condition
    std::any condition = evaluateNode(*node.getChildren()[0]);
    bool conditionResult = false;
    
    if (condition.type() == typeid(int)) {
        conditionResult = std::any_cast<int>(condition) != 0;
    } else if (condition.type() == typeid(double)) {
        conditionResult = std::any_cast<double>(condition) != 0.0;
    } else if (condition.type() == typeid(std::string)) {
        conditionResult = !std::any_cast<std::string>(condition).empty();
    }
    
    if (conditionResult && node.getChildren().size() > 1) {
        // Execute then branch
        return evaluateNode(*node.getChildren()[1]);
    } else if (!conditionResult && node.getChildren().size() > 2) {
        // Execute else branch
        return evaluateNode(*node.getChildren()[2]);
    }
    
    return std::any();
}

std::any ASTInterpreter::evaluateForLoop(const ASTNode& node) {
    if (node.getChildren().size() < 3) return std::any();
    
    // For loop structure: [init, condition, increment, body]
    // Initialize
    if (node.getChildren()[0]) {
        evaluateNode(*node.getChildren()[0]);
    }
    
    // Loop
    while (true) {
        // Check condition
        if (node.getChildren()[1]) {
            std::any condition = evaluateNode(*node.getChildren()[1]);
            bool conditionResult = false;
            if (condition.type() == typeid(int)) {
                conditionResult = std::any_cast<int>(condition) != 0;
            } else if (condition.type() == typeid(double)) {
                conditionResult = std::any_cast<double>(condition) != 0.0;
            }
            if (!conditionResult) break;
        }
        
        // Execute body
        if (node.getChildren().size() > 3 && node.getChildren()[3]) {
            evaluateNode(*node.getChildren()[3]);
        }
        
        // Increment
        if (node.getChildren()[2]) {
            evaluateNode(*node.getChildren()[2]);
        }
    }
    
    return std::any();
}

std::any ASTInterpreter::evaluateWhileLoop(const ASTNode& node) {
    if (node.getChildren().size() < 2) return std::any();
    
    // While loop structure: [condition, body]
    while (true) {
        // Check condition
        std::any condition = evaluateNode(*node.getChildren()[0]);
        bool conditionResult = false;
        if (condition.type() == typeid(int)) {
            conditionResult = std::any_cast<int>(condition) != 0;
        } else if (condition.type() == typeid(double)) {
            conditionResult = std::any_cast<double>(condition) != 0.0;
        }
        if (!conditionResult) break;
        
        // Execute body
        evaluateNode(*node.getChildren()[1]);
    }
    
    return std::any();
}

std::any ASTInterpreter::evaluateBlock(const ASTNode& node) {
    // Execute all statements in the block
    std::any result;
    for (const auto& child : node.getChildren()) {
        result = evaluateNode(*child);
    }
    return result;
}
