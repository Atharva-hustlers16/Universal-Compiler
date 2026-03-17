#ifndef AST_OPTIMIZER_H
#define AST_OPTIMIZER_H

#include "ASTNode.h"
#include <memory>
#include <vector>
#include <any>
#include <string>

class ASTOptimizer {
public:
    ASTOptimizer();
    ~ASTOptimizer() = default;
    
    std::unique_ptr<ASTNode> optimize(std::unique_ptr<ASTNode> ast);
    
    // Optimization passes
    void constantFolding(std::unique_ptr<ASTNode>& node);
    void deadCodeElimination(std::unique_ptr<ASTNode>& node);
    void functionInlining(std::unique_ptr<ASTNode>& node);
    void commonSubexpressionElimination(std::unique_ptr<ASTNode>& node);
    void algebraicSimplification(std::unique_ptr<ASTNode>& node);
    
    // Utility methods
    bool isConstant(const ASTNode& node);
    std::any evaluateConstantExpression(const ASTNode& node);
    bool isSideEffectFree(const ASTNode& node);
    bool isDeadCode(const ASTNode& node);
    
    // Statistics
    struct OptimizationStats {
        int constantsFolded = 0;
        int deadCodeRemoved = 0;
        int functionsInlined = 0;
        int subexpressionsEliminated = 0;
        int algebraicSimplifications = 0;
        int totalOptimizations = 0;
    };
    
    OptimizationStats getStats() const { return stats_; }
    void resetStats() { stats_ = OptimizationStats{}; }

private:
    OptimizationStats stats_;
    
    // Helper methods for optimization
    std::unique_ptr<ASTNode> foldBinaryOp(const ASTNode& node);
    std::unique_ptr<ASTNode> simplifyAlgebraicExpression(const ASTNode& node);
    bool canInline(const ASTNode& functionNode);
    
    // Tree traversal helpers
    void optimizeNode(std::unique_ptr<ASTNode>& node);
    void optimizeChildren(std::unique_ptr<ASTNode>& node);
    
    // Expression evaluation
    std::any evaluateLiteral(const ASTNode& node);
    std::any evaluateBinaryOperation(const std::string& op, const std::any& left, const std::any& right);
    
    // Type conversion utilities
    int toInt(const std::any& value);
    double toDouble(const std::any& value);
    std::string toString(const std::any& value);
    bool isNumeric(const std::any& value);
};

#endif // AST_OPTIMIZER_H
