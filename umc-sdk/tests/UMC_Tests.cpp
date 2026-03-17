#include "TestFramework.h"
#include "../include/ASTNode.h"
#include "../include/ASTOptimizer.h"
#include "../include/TreeSitterFrontend.h"
#include "../include/EnhancedJavaFrontend.h"
#include "../include/EnhancedPythonFrontend.h"
#include <memory>

// AST Node Tests
TEST(ASTNode, BasicCreation) {
    auto node = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "test");
    ASSERT_NOT_NULL(node.get());
    ASSERT_EQ(ASTNodeType::PROGRAM, node->getType());
    ASSERT_EQ("test", node->getValue());
    return true;
}

TEST(ASTNode, AddChildren) {
    auto parent = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "root");
    auto child1 = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, "func1");
    auto child2 = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, "func2");
    
    ASSERT_EQ(0, parent->getChildren().size());
    
    parent->addChild(std::move(child1));
    ASSERT_EQ(1, parent->getChildren().size());
    
    parent->addChild(std::move(child2));
    ASSERT_EQ(2, parent->getChildren().size());
    
    return true;
}

TEST(ASTNode, ToString) {
    auto program = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "root");
    auto func = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, "main");
    auto call = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, "print()");
    
    func->addChild(std::move(call));
    program->addChild(std::move(func));
    
    std::string str = program->toString();
    ASSERT_FALSE(str.empty());
    ASSERT_NE(std::string::npos, str.find("Program"));
    ASSERT_NE(std::string::npos, str.find("FunctionDecl"));
    ASSERT_NE(std::string::npos, str.find("FunctionCall"));
    
    return true;
}

// AST Optimizer Tests
TEST(ASTOptimizer, ConstantFolding) {
    ASTOptimizer optimizer;
    
    // Create AST for 2 + 3
    auto two = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "2");
    auto three = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "3");
    auto add = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
    add->addChild(std::move(two));
    add->addChild(std::move(three));
    
    auto program = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "test");
    program->addChild(std::move(add));
    
    auto optimized = optimizer.optimize(std::move(program));
    ASSERT_NOT_NULL(optimized.get());
    
    // After optimization, should have a single literal "5"
    ASSERT_EQ(1, optimized->getChildren().size());
    auto result = optimized->getChildren()[0].get();
    ASSERT_EQ(ASTNodeType::LITERAL, result->getType());
    ASSERT_EQ("5", result->getValue());
    
    auto stats = optimizer.getStats();
    ASSERT_GT(stats.constantsFolded, 0);
    
    return true;
}

TEST(ASTOptimizer, AlgebraicSimplification) {
    ASTOptimizer optimizer;
    
    // Create AST for x + 0
    auto x = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "x");
    auto zero = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "0");
    auto add = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
    add->addChild(std::move(x));
    add->addChild(std::move(zero));
    
    auto program = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "test");
    program->addChild(std::move(add));
    
    auto optimized = optimizer.optimize(std::move(program));
    ASSERT_NOT_NULL(optimized.get());
    
    // After optimization, should have just "x"
    ASSERT_EQ(1, optimized->getChildren().size());
    auto result = optimized->getChildren()[0].get();
    ASSERT_EQ(ASTNodeType::LITERAL, result->getType());
    ASSERT_EQ("x", result->getValue());
    
    auto stats = optimizer.getStats();
    ASSERT_GT(stats.algebraicSimplifications, 0);
    
    return true;
}

TEST(ASTOptimizer, DeadCodeElimination) {
    ASTOptimizer optimizer;
    
    // Create empty function (dead code)
    auto emptyFunc = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, "empty");
    
    auto program = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "test");
    program->addChild(std::move(emptyFunc));
    
    auto optimized = optimizer.optimize(std::move(program));
    ASSERT_NOT_NULL(optimized.get());
    
    // Empty function should be removed
    ASSERT_EQ(0, optimized->getChildren().size());
    
    auto stats = optimizer.getStats();
    ASSERT_GT(stats.deadCodeRemoved, 0);
    
    return true;
}

// Enhanced Frontend Tests
TEST(EnhancedJava, BasicParsing) {
    EnhancedJavaFrontend frontend;
    std::string javaCode = R"(
public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
)";
    
    bool parsed = frontend.parse(javaCode);
    ASSERT_TRUE(parsed);
    
    auto ast = frontend.getAST();
    ASSERT_NOT_NULL(ast.get());
    ASSERT_EQ(ASTNodeType::PROGRAM, ast->getType());
    
    // Should have at least one class
    bool hasClass = false;
    for (const auto& child : ast->getChildren()) {
        if (child->getType() == ASTNodeType::CLASS_DECL) {
            hasClass = true;
            break;
        }
    }
    ASSERT_TRUE(hasClass);
    
    return true;
}

TEST(EnhancedPython, BasicParsing) {
    EnhancedPythonFrontend frontend;
    std::string pythonCode = R"(
def hello_world():
    print("Hello, World!")
    return 42

if __name__ == "__main__":
    hello_world()
)";
    
    bool parsed = frontend.parse(pythonCode);
    ASSERT_TRUE(parsed);
    
    auto ast = frontend.getAST();
    ASSERT_NOT_NULL(ast.get());
    ASSERT_EQ(ASTNodeType::PROGRAM, ast->getType());
    
    // Should have at least one function
    bool hasFunction = false;
    for (const auto& child : ast->getChildren()) {
        if (child->getType() == ASTNodeType::FUNCTION_DECL) {
            hasFunction = true;
            break;
        }
    }
    ASSERT_TRUE(hasFunction);
    
    return true;
}

TEST(EnhancedPython, ImportExtraction) {
    EnhancedPythonFrontend frontend;
    std::string pythonCode = R"(
import os
import sys
from collections import defaultdict

def main():
    print("Imports extracted")
)";
    
    bool parsed = frontend.parse(pythonCode);
    ASSERT_TRUE(parsed);
    
    auto ast = frontend.getAST();
    ASSERT_NOT_NULL(ast.get());
    
    // Should have import statements
    int importCount = 0;
    for (const auto& child : ast->getChildren()) {
        if (child->getType() == ASTNodeType::IMPORT_DECL) {
            importCount++;
        }
    }
    ASSERT_EQ(3, importCount);
    
    return true;
}

// Integration Tests
TEST(Integration, ParseAndOptimize) {
    // Parse with enhanced frontend
    EnhancedPythonFrontend frontend;
    std::string code = R"(
def calculate():
    result = 2 + 3  # Should be folded to 5
    return result
)";
    
    bool parsed = frontend.parse(code);
    ASSERT_TRUE(parsed);
    
    auto ast = frontend.getAST();
    ASSERT_NOT_NULL(ast.get());
    
    // Optimize the AST
    ASTOptimizer optimizer;
    auto optimized = optimizer.optimize(std::move(ast));
    ASSERT_NOT_NULL(optimized.get());
    
    // Verify optimization occurred
    auto stats = optimizer.getStats();
    ASSERT_GT(stats.totalOptimizations, 0);
    
    return true;
}

// Performance Benchmarks
BENCHMARK(ASTNode, Creation) {
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        auto node = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "benchmark");
        auto child = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, "func");
        node->addChild(std::move(child));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0; // Return milliseconds
}

BENCHMARK(EnhancedPython, Parsing) {
    std::string code = R"(
import os
import sys
from collections import defaultdict

def process_data(data):
    result = []
    for item in data:
        if item is not None:
            result.append(item * 2)
    return result

def main():
    data = [1, 2, 3, 4, 5]
    processed = process_data(data)
    print(f"Processed: {processed}")
)";
    
    const int iterations = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        EnhancedPythonFrontend frontend;
        frontend.parse(code);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0; // Return milliseconds
}

BENCHMARK(ASTOptimizer, Optimization) {
    // Create a complex AST for optimization benchmark
    ASTOptimizer optimizer;
    const int iterations = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        // Create AST with optimization opportunities
        auto program = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "test");
        
        // Add constant folding opportunity
        auto two = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "2");
        auto three = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "3");
        auto add = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
        add->addChild(std::move(two));
        add->addChild(std::move(three));
        
        // Add algebraic simplification opportunity
        auto x = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "x");
        auto zero = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "0");
        auto add2 = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
        add2->addChild(std::move(x));
        add2->addChild(std::move(zero));
        
        program->addChild(std::move(add));
        program->addChild(std::move(add2));
        
        optimizer.optimize(std::move(program));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0; // Return milliseconds
}
