#include "include/EnhancedJavaFrontend.h"
#include "include/EnhancedPythonFrontend.h"
#include "include/ASTOptimizer.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== UMC-SDK Test Program ===" << std::endl;
    
    // Test Java parsing
    std::cout << "\n--- Testing Java Frontend ---" << std::endl;
    EnhancedJavaFrontend javaParser;
    std::string javaCode = R"(
package examples;

public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello from UMC-SDK!");
        int x = 2 + 3;  // Should be optimized to 5
        int y = x * 1;  // Should be optimized to x
        System.out.println("Result: " + (x + y));
    }
}
)";
    
    bool javaParsed = javaParser.parse(javaCode);
    std::cout << "Java parsing: " << (javaParsed ? "SUCCESS" : "FAILED") << std::endl;
    
    if (javaParsed) {
        auto javaAST = javaParser.getAST();
        if (javaAST) {
            std::cout << "Java AST generated successfully" << std::endl;
            std::cout << "AST: " << javaAST->toString(2) << std::endl;
        } else {
            std::cout << "Java AST is null" << std::endl;
        }
    }
    
    // Test Python parsing
    std::cout << "\n--- Testing Python Frontend ---" << std::endl;
    EnhancedPythonFrontend pythonParser;
    std::string pythonCode = R"(
def hello_world():
    x = 2 + 3  # Should be optimized to 5
    y = x * 1  # Should be optimized to x
    print(f"Hello from UMC-SDK! Result: {x + y}")
    return x + y

if __name__ == "__main__":
    hello_world()
)";
    
    bool pythonParsed = pythonParser.parse(pythonCode);
    std::cout << "Python parsing: " << (pythonParsed ? "SUCCESS" : "FAILED") << std::endl;
    
    if (pythonParsed) {
        auto pythonAST = pythonParser.getAST();
        if (pythonAST) {
            std::cout << "Python AST generated successfully" << std::endl;
            std::cout << "AST: " << pythonAST->toString(2) << std::endl;
        } else {
            std::cout << "Python AST is null" << std::endl;
        }
    }
    
    // Test AST optimization
    std::cout << "\n--- Testing AST Optimizer ---" << std::endl;
    ASTOptimizer optimizer;
    
    if (javaParsed) {
        auto javaAST = javaParser.getAST();
        if (javaAST) {
            std::cout << "Optimizing Java AST..." << std::endl;
            auto optimizedAST = optimizer.optimize(std::make_unique<ASTNode>(*javaAST));
            if (optimizedAST) {
                std::cout << "Java AST optimization successful" << std::endl;
                auto stats = optimizer.getStats();
                std::cout << "Optimizations performed: " << stats.totalOptimizations << std::endl;
                std::cout << "Constants folded: " << stats.constantsFolded << std::endl;
                std::cout << "Algebraic simplifications: " << stats.algebraicSimplifications << std::endl;
            } else {
                std::cout << "Java AST optimization failed" << std::endl;
            }
        }
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}
