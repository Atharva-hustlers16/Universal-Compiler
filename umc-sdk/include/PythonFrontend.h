#ifndef PYTHON_FRONTEND_H
#define PYTHON_FRONTEND_H

#include "FrontendBase.h"
#include "ASTNode.h"
#include <string>
#include <memory>

// Forward declare FrontendBase's ASTNode to avoid conflicts
struct FrontendASTNode {
    virtual ~FrontendASTNode() = default;
    virtual std::string toString() const = 0;
};

// Check if Tree-sitter is available
#ifdef ENABLE_TREESITTER
// Forward declarations for Tree-sitter
extern "C" {
    typedef struct TSLanguage TSLanguage;
    typedef struct TSTree TSTree;
    typedef struct TSParser TSParser;
    // Define TSNode structure if not available
    typedef struct {
        uint32_t context[4];
        const void* id;
        const TSTree* tree;
    } TSNode;
}
#else
// Dummy definitions when Tree-sitter is not available
typedef void* TSLanguage;
typedef void* TSTree;
typedef void* TSParser;
typedef void* TSNode;
#endif

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
    
    // Enhanced fallback parser methods
    void parsePythonSource(const std::string& sourceCode);
    void parsePythonFunctions(const std::string& sourceCode);
    void parseGlobalStatements(const std::string& sourceCode);
    void parsePythonStatements(const std::string& body, ASTNode& blockNode);
    std::unique_ptr<ASTNode> parsePythonStatement(const std::string& statement);
    std::unique_ptr<ASTNode> parsePythonExpression(const std::string& expr);
    void parsePythonFunctionArguments(const std::string& argsStr, ASTNode& callNode);
    std::string preprocessPythonCode(const std::string& sourceCode);
    bool isInsidePythonFunction(const std::string& code, size_t position);
};

#endif // PYTHON_FRONTEND_H
