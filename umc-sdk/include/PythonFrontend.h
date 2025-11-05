#ifndef PYTHON_FRONTEND_H
#define PYTHON_FRONTEND_H

#include "FrontendBase.h"
#include "ASTNode.h"
#include <string>
#include <memory>

// Forward declarations for Tree-sitter
extern "C" {
    typedef struct TSLanguage TSLanguage;
    typedef struct TSTree TSTree;
    typedef struct TSNode TSNode;
    typedef struct TSParser TSParser;
}

class PythonFrontend : public FrontendBase {
public:
    PythonFrontend();
    ~PythonFrontend();

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override;
    Language getLanguage() const override { return Language::PYTHON; }

private:
    TSLanguage* tree_sitter_python();
    std::unique_ptr<ProgramNode> ast_;
    TSParser* parser_;
    TSTree* tree_;

    std::unique_ptr<ASTNode> buildASTFromTSNode(TSNode node);
    std::unique_ptr<ASTNode> createNodeFromTSNode(TSNode node);
    std::string getNodeType(TSNode node);
    std::string getNodeText(TSNode node, const std::string& source);

    std::unique_ptr<ASTNode> visitModule(TSNode node);
    std::unique_ptr<ASTNode> visitFunctionDefinition(TSNode node);
    std::unique_ptr<ASTNode> visitClassDefinition(TSNode node);
    std::unique_ptr<ASTNode> visitAssignment(TSNode node);
    std::unique_ptr<ASTNode> visitExpressionStatement(TSNode node);
    std::unique_ptr<ASTNode> visitIfStatement(TSNode node);
    std::unique_ptr<ASTNode> visitForStatement(TSNode node);
    std::unique_ptr<ASTNode> visitWhileStatement(TSNode node);
    std::unique_ptr<ASTNode> visitReturnStatement(TSNode node);
    std::unique_ptr<ASTNode> visitBinaryOperator(TSNode node);
    std::unique_ptr<ASTNode> visitIdentifier(TSNode node);
    std::unique_ptr<ASTNode> visitString(TSNode node);
    std::unique_ptr<ASTNode> visitInteger(TSNode node);
    std::unique_ptr<ASTNode> visitFloat(TSNode node);
    std::unique_ptr<ASTNode> visitBoolean(TSNode node);
    std::unique_ptr<ASTNode> visitNone(TSNode node);
    std::unique_ptr<ASTNode> visitList(TSNode node);
    std::unique_ptr<ASTNode> visitDictionary(TSNode node);
    std::unique_ptr<ASTNode> visitCall(TSNode node);
};

#endif // PYTHON_FRONTEND_H
