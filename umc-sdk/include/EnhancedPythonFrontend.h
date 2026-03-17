#ifndef ENHANCED_PYTHON_FRONTEND_H
#define ENHANCED_PYTHON_FRONTEND_H

#include "FrontendBase.h"

#ifdef TREE_SITTER_AVAILABLE
#include <tree_sitter/api.h>
#endif

class EnhancedPythonFrontend : public FrontendBase {
public:
    EnhancedPythonFrontend();
    ~EnhancedPythonFrontend() override;

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override;

private:
    std::unique_ptr<ASTNode> ast_;
    
#ifdef TREE_SITTER_AVAILABLE
    TSParser* parser_;
    TSLanguage* python_language_;
    TSTree* tree_;
    
    bool initializeTreeSitter();
    void cleanupTreeSitter();
    std::unique_ptr<ASTNode> parseWithTreeSitter(const std::string& sourceCode);
    std::unique_ptr<ASTNode> convertTreeSitterNode(TSNode node, const std::string& sourceCode);
    ASTNodeType mapTreeSitterNodeType(const std::string& type);
#endif
    
    void parseWithEnhancedFallback(const std::string& sourceCode);
    
    // Enhanced fallback parsing methods
    void extractFunctions(const std::string& source);
    void extractFunctions(const std::string& source, ASTNode* parent);
    void extractClasses(const std::string& source);
    void extractImports(const std::string& source);
    void extractDecorators(const std::string& source);
    void extractControlFlow(const std::string& source);
    
    // AST construction helpers
    std::unique_ptr<ASTNode> createFunctionNode(const std::string& functionName, 
                                              const std::vector<std::string>& params);
    std::unique_ptr<ASTNode> createClassNode(const std::string& className);
    std::unique_ptr<ASTNode> createImportNode(const std::string& importName);
    std::unique_ptr<ASTNode> createControlFlowNode(const std::string& type, const std::string& condition);
    
    // Python-specific parsing utilities
    std::vector<std::string> extractFunctionParams(const std::string& functionDef);
    std::string extractFunctionBody(const std::string& functionDef);
    std::string extractClassBody(const std::string& classDef);
    std::vector<std::string> parsePythonList(const std::string& listStr);
    
    // String utilities
    std::string trim(const std::string& str);
    std::string removeIndentation(const std::string& code);
    bool isPythonComment(const std::string& line);
    std::string extractExpression(const std::string& line);
};

#endif // ENHANCED_PYTHON_FRONTEND_H
