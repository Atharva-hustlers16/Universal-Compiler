#include "ASTOptimizer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <any>

ASTOptimizer::ASTOptimizer() {
    std::cout << "AST Optimizer initialized" << std::endl;
}

std::unique_ptr<ASTNode> ASTOptimizer::optimize(std::unique_ptr<ASTNode> ast) {
    if (!ast) {
        return nullptr;
    }
    
    std::cout << "Starting AST optimization..." << std::endl;
    resetStats();
    
    // Run optimization passes
    constantFolding(ast);
    deadCodeElimination(ast);
    algebraicSimplification(ast);
    commonSubexpressionElimination(ast);
    functionInlining(ast);
    
    std::cout << "AST optimization completed. Total optimizations: " << stats_.totalOptimizations << std::endl;
    return ast;
}

void ASTOptimizer::constantFolding(std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    
    // First optimize children
    optimizeChildren(node);
    
    // Then try to fold this node if it's a binary operation
    if (node->getType() == ASTNodeType::BINARY_OP && node->getChildren().size() >= 2) {
        auto folded = foldBinaryOp(*node);
        if (folded) {
            node = std::move(folded);
            stats_.constantsFolded++;
            stats_.totalOptimizations++;
        }
    }
}

std::unique_ptr<ASTNode> ASTOptimizer::foldBinaryOp(const ASTNode& node) {
    if (node.getType() != ASTNodeType::BINARY_OP || node.getChildren().size() < 2) {
        return nullptr;
    }
    
    const auto& children = node.getChildren();
    const auto& left = *children[0];
    const auto& right = *children[1];
    
    // Check if both operands are constants
    if (!isConstant(left) || !isConstant(right)) {
        return nullptr;
    }
    
    // Evaluate the constant expression
    std::any result = evaluateConstantExpression(node);
    
    // Create a new literal node with the result
    std::string resultStr;
    if (result.type() == typeid(int)) {
        resultStr = std::to_string(std::any_cast<int>(result));
    } else if (result.type() == typeid(double)) {
        resultStr = std::to_string(std::any_cast<double>(result));
    } else if (result.type() == typeid(std::string)) {
        resultStr = std::any_cast<std::string>(result);
    } else {
        return nullptr;
    }
    
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, resultStr);
}

void ASTOptimizer::deadCodeElimination(std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    
    // Optimize children first
    optimizeChildren(node);
    
    // Remove dead code
    if (isDeadCode(*node)) {
        node = nullptr;
        stats_.deadCodeRemoved++;
        stats_.totalOptimizations++;
        return;
    }
    
    // Remove unreachable code after return statements
    if (node->getType() == ASTNodeType::FUNCTION_DECL) {
        const auto& children = node->getChildren();
        bool foundReturn = false;
        
        for (auto it = children.begin(); it != children.end(); ) {
            if ((*it)->getType() == ASTNodeType::RETURN_STMT) {
                foundReturn = true;
                ++it;
            } else if (foundReturn && isSideEffectFree(**it)) {
                // Remove dead code after return
                it = node->getChildren().erase(it);
                stats_.deadCodeRemoved++;
                stats_.totalOptimizations++;
            } else {
                ++it;
            }
        }
    }
}

void ASTOptimizer::functionInlining(std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    
    // Optimize children first
    optimizeChildren(node);
    
    // Inline small functions
    if (node->getType() == ASTNodeType::FUNCTION_CALL && canInline(*node)) {
        // Find function definition (simplified - in real implementation would use symbol table)
        // For now, just skip inlining as it requires complex analysis
        std::cout << "Function inlining not yet implemented for: " << node->getValue() << std::endl;
    }
}

void ASTOptimizer::commonSubexpressionElimination(std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    
    // This is a simplified CSE - real implementation would be more sophisticated
    optimizeChildren(node);
    
    // Look for duplicate subexpressions in binary operations
    if (node->getType() == ASTNodeType::BINARY_OP && node->getChildren().size() >= 2) {
        const auto& left = node->getChildren()[0];
        const auto& right = node->getChildren()[1];
        
        // Check if left and right are identical expressions
        if (left->toString() == right->toString()) {
            std::string op = node->getValue();
            
            // Apply CSE rules
            if (op == "+" && isConstant(*left)) {
                // a + a = 2 * a
                auto twoNode = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "2");
                auto multNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "*");
                multNode->addChild(std::move(twoNode));
                multNode->addChild(left->clone());
                node = std::move(multNode);
                stats_.subexpressionsEliminated++;
                stats_.totalOptimizations++;
            } else if (op == "*" && isConstant(*left)) {
                // a * a = a^2 (simplified)
                auto twoNode = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "2");
                auto powNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "^");
                powNode->addChild(left->clone());
                powNode->addChild(std::move(twoNode));
                node = std::move(powNode);
                stats_.subexpressionsEliminated++;
                stats_.totalOptimizations++;
            }
        }
    }
}

void ASTOptimizer::algebraicSimplification(std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    
    // Optimize children first
    optimizeChildren(node);
    
    // Apply algebraic simplifications
    if (node->getType() == ASTNodeType::BINARY_OP && node->getChildren().size() >= 2) {
        auto simplified = simplifyAlgebraicExpression(*node);
        if (simplified) {
            node = std::move(simplified);
            stats_.algebraicSimplifications++;
            stats_.totalOptimizations++;
        }
    }
}

std::unique_ptr<ASTNode> ASTOptimizer::simplifyAlgebraicExpression(const ASTNode& node) {
    if (node.getType() != ASTNodeType::BINARY_OP || node.getChildren().size() < 2) {
        return nullptr;
    }
    
    const auto& children = node.getChildren();
    const auto& left = *children[0];
    const auto& right = *children[1];
    std::string op = node.getValue();
    
    // Identity elements
    if (op == "+") {
        // x + 0 = x
        if (isConstant(right) && evaluateConstantExpression(right).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(right)) == 0) {
            return left.clone();
        }
        // 0 + x = x
        if (isConstant(left) && evaluateConstantExpression(left).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(left)) == 0) {
            return right.clone();
        }
    } else if (op == "*") {
        // x * 1 = x
        if (isConstant(right) && evaluateConstantExpression(right).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(right)) == 1) {
            return left.clone();
        }
        // 1 * x = x
        if (isConstant(left) && evaluateConstantExpression(left).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(left)) == 1) {
            return right.clone();
        }
        // x * 0 = 0
        if (isConstant(right) && evaluateConstantExpression(right).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(right)) == 0) {
            return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "0");
        }
        // 0 * x = 0
        if (isConstant(left) && evaluateConstantExpression(left).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(left)) == 0) {
            return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "0");
        }
    } else if (op == "-") {
        // x - 0 = x
        if (isConstant(right) && evaluateConstantExpression(right).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(right)) == 0) {
            return left.clone();
        }
        // x - x = 0
        if (left.toString() == right.toString()) {
            return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "0");
        }
    } else if (op == "/") {
        // x / 1 = x
        if (isConstant(right) && evaluateConstantExpression(right).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(right)) == 1) {
            return left.clone();
        }
        // 0 / x = 0 (if x != 0)
        if (isConstant(left) && evaluateConstantExpression(left).type() == typeid(int) && 
            std::any_cast<int>(evaluateConstantExpression(left)) == 0) {
            return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "0");
        }
    }
    
    return nullptr;
}

bool ASTOptimizer::isConstant(const ASTNode& node) {
    if (node.getType() == ASTNodeType::LITERAL) {
        return true;
    }
    
    // Check if it's a constant binary operation
    if (node.getType() == ASTNodeType::BINARY_OP && node.getChildren().size() >= 2) {
        return isConstant(*node.getChildren()[0]) && isConstant(*node.getChildren()[1]);
    }
    
    return false;
}

std::any ASTOptimizer::evaluateConstantExpression(const ASTNode& node) {
    if (node.getType() == ASTNodeType::LITERAL) {
        return evaluateLiteral(node);
    }
    
    if (node.getType() == ASTNodeType::BINARY_OP && node.getChildren().size() >= 2) {
        std::any left = evaluateConstantExpression(*node.getChildren()[0]);
        std::any right = evaluateConstantExpression(*node.getChildren()[1]);
        return evaluateBinaryOperation(node.getValue(), left, right);
    }
    
    return std::any();
}

std::any ASTOptimizer::evaluateLiteral(const ASTNode& node) {
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

std::any ASTOptimizer::evaluateBinaryOperation(const std::string& op, const std::any& left, const std::any& right) {
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

bool ASTOptimizer::isSideEffectFree(const ASTNode& node) {
    // Simplified check - in reality this would be more complex
    if (node.getType() == ASTNodeType::FUNCTION_CALL) {
        // Assume function calls have side effects unless proven otherwise
        return false;
    }
    
    if (node.getType() == ASTNodeType::VARIABLE_DECL) {
        return false; // Variable declarations have side effects
    }
    
    // Check children
    for (const auto& child : node.getChildren()) {
        if (!isSideEffectFree(*child)) {
            return false;
        }
    }
    
    return true;
}

bool ASTOptimizer::isDeadCode(const ASTNode& node) {
    // Simplified dead code detection
    if (node.getType() == ASTNodeType::FUNCTION_DECL && node.getChildren().empty()) {
        return true; // Empty function
    }
    
    if (node.getType() == ASTNodeType::PROGRAM && node.getChildren().empty()) {
        return true; // Empty program
    }
    
    return false;
}

bool ASTOptimizer::canInline(const ASTNode& functionNode) {
    // Simplified inlining criteria
    if (functionNode.getType() != ASTNodeType::FUNCTION_DECL) {
        return false;
    }
    
    // Only inline small functions (simplified)
    const auto& children = functionNode.getChildren();
    return children.size() <= 3; // Max 3 statements
}

void ASTOptimizer::optimizeNode(std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    
    constantFolding(node);
    deadCodeElimination(node);
    algebraicSimplification(node);
    commonSubexpressionElimination(node);
    functionInlining(node);
}

void ASTOptimizer::optimizeChildren(std::unique_ptr<ASTNode>& node) {
    if (!node) return;
    
    for (auto& child : node->getChildren()) {
        optimizeNode(child);
    }
}

int ASTOptimizer::toInt(const std::any& value) {
    if (value.type() == typeid(int)) {
        return std::any_cast<int>(value);
    }
    return static_cast<int>(toDouble(value));
}

double ASTOptimizer::toDouble(const std::any& value) {
    if (value.type() == typeid(double)) {
        return std::any_cast<double>(value);
    }
    if (value.type() == typeid(int)) {
        return static_cast<double>(std::any_cast<int>(value));
    }
    return std::stod(toString(value));
}

std::string ASTOptimizer::toString(const std::any& value) {
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

bool ASTOptimizer::isNumeric(const std::any& value) {
    return value.type() == typeid(int) || value.type() == typeid(double);
}
