#ifndef ENHANCED_JAVA_FRONTEND_H
#define ENHANCED_JAVA_FRONTEND_H

#include "FrontendBase.h"

#ifdef ANTLR_AVAILABLE
#include "JavaLexer.h"
#include "JavaParser.h"
#include "JavaParserVisitor.h"
#include "antlr4-runtime.h"
#endif

class EnhancedJavaFrontend : public FrontendBase {
public:
    EnhancedJavaFrontend();
    ~EnhancedJavaFrontend() override;

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() override;

private:
    std::unique_ptr<ASTNode> ast_;
    
    void parseWithANTLR(const std::string& sourceCode);
    void parseWithFallback(const std::string& sourceCode);
    
    // Helper methods for fallback parsing
    void extractClasses(const std::string& source);
    void extractMethods(const std::string& source);
    void extractImports(const std::string& source);
    
    // AST construction helpers
    std::unique_ptr<ASTNode> createClassNode(const std::string& className);
    std::unique_ptr<ASTNode> createMethodNode(const std::string& methodName, 
                                            const std::string& returnType, 
                                            const std::vector<std::string>& params);
    std::unique_ptr<ASTNode> createImportNode(const std::string& importName);
    
    // String utilities
    std::string extractBetween(const std::string& source, const std::string& start, const std::string& end);
    std::vector<std::string> splitBy(const std::string& source, const std::string& delimiter);
    std::string trim(const std::string& str);
};

#ifdef ANTLR_AVAILABLE
class JavaASTVisitor : public JavaParserVisitor {
public:
    JavaASTVisitor();
    std::unique_ptr<ASTNode> getAST();
    
    // Override visitor methods
    antlrcpp::Any visitCompilationUnit(JavaParser::CompilationUnitContext* ctx) override;
    antlrcpp::Any visitClassDeclaration(JavaParser::ClassDeclarationContext* ctx) override;
    antlrcpp::Any visitMethodDeclaration(JavaParser::MethodDeclarationContext* ctx) override;
    antlrcpp::Any visitImportDeclaration(JavaParser::ImportDeclarationContext* ctx) override;
    
private:
    std::unique_ptr<ASTNode> ast_;
    std::vector<std::unique_ptr<ASTNode>> nodeStack_;
};
#endif

#endif // ENHANCED_JAVA_FRONTEND_H
