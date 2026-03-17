#ifndef TREE_SITTER_FRONTEND_H
#define TREE_SITTER_FRONTEND_H

#include "FrontendBase.h"
#include <tree_sitter/api.h>

class TreeSitterFrontend : public FrontendBase {
public:
    TreeSitterFrontend(const std::string& language);
    ~TreeSitterFrontend() override;

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override;

private:
    std::string language_;
    TSParser* parser_;
    TSLanguage* tree_sitter_language_;
    TSTree* tree_;
    std::unique_ptr<ASTNode> ast_;

    bool initializeParser();
    void cleanupParser();
    
    // Language-specific initialization
    TSLanguage* getLanguageForName(const std::string& name);
    
    // AST conversion methods
    std::unique_ptr<ASTNode> convertTreeToAST(TSNode node);
    ASTNodeType mapNodeType(TSSymbol symbol, const std::string& typeName);
    std::string getNodeText(TSNode node, const std::string& sourceCode);
    
    // Helper methods
    std::string getNodeTypeString(TSNode node);
    uint32_t getChildCount(TSNode node);
    TSNode getChild(TSNode node, uint32_t index);
};

#endif // TREE_SITTER_FRONTEND_H
