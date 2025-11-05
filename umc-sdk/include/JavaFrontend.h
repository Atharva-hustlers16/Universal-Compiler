#ifndef JAVA_FRONTEND_H
#define JAVA_FRONTEND_H

#include "FrontendBase.h"
#include "ASTNode.h"
#include <string>
#include <memory>

// Forward declarations for ANTLR generated classes
namespace antlr4 {
    class ANTLRInputStream;
    class CommonTokenStream;
    class tree::ParseTree;
}

class JavaLexer;
class JavaParser;
class JavaParserBaseVisitor;

class JavaFrontend : public FrontendBase {
public:
    JavaFrontend();
    ~JavaFrontend();

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override;
    Language getLanguage() const override { return Language::JAVA; }

private:
    std::unique_ptr<ProgramNode> ast_;
    std::unique_ptr<JavaLexer> lexer_;
    std::unique_ptr<JavaParser> parser_;
    std::unique_ptr<antlr4::CommonTokenStream> tokens_;
    std::unique_ptr<antlr4::ANTLRInputStream> input_;

    std::unique_ptr<ASTNode> buildASTFromParseTree(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitCompilationUnit(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitClassDeclaration(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitMethodDeclaration(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitFieldDeclaration(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitBlock(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitStatement(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitExpression(antlr4::tree::ParseTree* tree);
    std::unique_ptr<ASTNode> visitLiteral(antlr4::tree::ParseTree* tree);

    std::string getTextFromParseTree(antlr4::tree::ParseTree* tree);
};

#endif // JAVA_FRONTEND_H
