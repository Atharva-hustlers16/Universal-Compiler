#ifndef CFLANG_FRONTEND_H
#define CFLANG_FRONTEND_H

#include "FrontendBase.h"
#include "ASTNode.h"
#include <clang-c/Index.h>
#include <memory>
#include <string>

class CFrontend : public FrontendBase {
public:
    CFrontend();
    ~CFrontend();

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override;
    Language getLanguage() const override { return Language::C; }

private:
    CXIndex index_;
    CXTranslationUnit translationUnit_;
    std::unique_ptr<ProgramNode> ast_;

    std::unique_ptr<ASTNode> buildASTFromCursor(CXCursor cursor);
    std::unique_ptr<ASTNode> createNodeFromCursor(CXCursor cursor);
    std::string getCursorSpelling(CXCursor cursor);
    std::string getCursorType(CXCursor cursor);

    void traverseAST(CXCursor cursor, ASTNode* parent);
};

#endif // CFLANG_FRONTEND_H
