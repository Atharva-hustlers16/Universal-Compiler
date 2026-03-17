#ifndef JAVA_FRONTEND_H
#define JAVA_FRONTEND_H

#include "FrontendBase.h"
#include "ASTNode.h"
#include <string>
#include <memory>

// Check if ANTLR4 is available
#ifdef ANTLR4Runtime_FOUND
// Forward declarations for ANTLR generated classes
namespace antlr4 {
    class ANTLRInputStream;
    class CommonTokenStream;
    class tree::ParseTree;
}

class JavaLexer;
class JavaParser;
class JavaParserBaseVisitor;
#else
// Dummy definitions when ANTLR4 is not available
namespace antlr4 {
    class ANTLRInputStream {};
    class CommonTokenStream {};
    namespace tree {
        class ParseTree {};
    }
}
class JavaLexer {};
class JavaParser {};
class JavaParserBaseVisitor {};
#endif

class JavaFrontend : public FrontendBase {
public:
    JavaFrontend();
    ~JavaFrontend();

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override;
    Language getLanguage() const override { return Language::JAVA; }

private:
#ifdef ANTLR4Runtime_FOUND
    std::unique_ptr<antlr4::ANTLRInputStream> input_;
    std::unique_ptr<JavaLexer> lexer_;
    std::unique_ptr<antlr4::CommonTokenStream> tokens_;
    std::unique_ptr<JavaParser> parser_;
    antlr4::tree::ParseTree* parseTree_;
    std::unique_ptr<class JavaASTVisitor> visitor_;
#endif

    // Fallback parser helper methods
    void parseStatements(const std::string& body, ASTNode& blockNode);
    std::unique_ptr<ASTNode> parseStatement(const std::string& statement);
    std::unique_ptr<ASTNode> parseExpression(const std::string& expr);
    void parseFunctionArguments(const std::string& argsStr, ASTNode& callNode);
    size_t findOperatorAtLevel(const std::string& expr, const std::string& operators);
};

// Factory function for creating Java frontend
std::unique_ptr<FrontendBase> createJavaFrontend();

#endif // JAVA_FRONTEND_H
