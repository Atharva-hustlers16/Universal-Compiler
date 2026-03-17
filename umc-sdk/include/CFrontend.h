#ifndef CFLANG_FRONTEND_H
#define CFLANG_FRONTEND_H

#include "FrontendBase.h"
#include "ASTNode.h"
#include <memory>
#include <string>

// Check if Clang is available
#ifdef CLANG_FOUND
#include <clang-c/Index.h>
#endif

class CFrontend : public FrontendBase {
public:
    CFrontend();
    ~CFrontend();

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override;
    Language getLanguage() const override { return Language::C; }

private:
#ifdef CLANG_FOUND
    CXIndex index_;
    CXTranslationUnit translationUnit_;
#endif
    std::unique_ptr<ProgramNode> ast_;

#ifdef CLANG_FOUND
    std::unique_ptr<ASTNode> buildASTFromCursor(CXCursor cursor);
    std::unique_ptr<ASTNode> createNodeFromCursor(CXCursor cursor);
    std::string getCursorSpelling(CXCursor cursor);
    std::string getCursorType(CXCursor cursor);

    void traverseAST(CXCursor cursor, ASTNode* parent);
#endif

    // Enhanced fallback parser methods
    void parseStatements(const std::string& body, ASTNode& blockNode);
    std::unique_ptr<ASTNode> parseStatement(const std::string& statement);
    std::unique_ptr<ASTNode> parseExpression(const std::string& expr);
    void parseFunctionArguments(const std::string& argsStr, ASTNode& callNode);
    void parseFunctionParameters(const std::string& paramsStr, ASTNode& funcNode);
    size_t findOperatorAtLevel(const std::string& expr, const std::string& operators);
    std::string preprocessSourceCode(const std::string& sourceCode);
    void parseGlobalFunctions(const std::string& cleanCode);
    bool isInsideFunction(const std::string& code, size_t position);
};

#endif // CFLANG_FRONTEND_H
