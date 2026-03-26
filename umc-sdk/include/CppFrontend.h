#pragma once

#include "FrontendBase.h"
#include <memory>
#include <string>

class CppFrontend : public FrontendBase {
public:
    CppFrontend();
    ~CppFrontend() override = default;

    bool parse(const std::string& sourceCode) override;
    std::unique_ptr<ASTNode> getAST() const override { return ast_ ? ast_->clone() : nullptr; }
    Language getLanguage() const override { return Language::CPP; }

private:
    std::unique_ptr<ASTNode> parseCppStatement(const std::string& statement, size_t& pos);
    std::unique_ptr<ASTNode> parseCppFunction(const std::string& funcSignature, size_t& pos);
    std::unique_ptr<ASTNode> parseCppExpression(const std::string& expr);
    std::unique_ptr<ASTNode> parseCppVariableDeclaration(const std::string& decl);
    std::unique_ptr<ASTNode> parseCppFunctionCall(const std::string& call);
    std::unique_ptr<ASTNode> parseCppClass(const std::string& classSignature, size_t& pos);
    std::unique_ptr<ASTNode> parseCppConstructor(const std::string& ctorSignature, size_t& pos);
    std::unique_ptr<ASTNode> parseCppMethod(const std::string& methodSignature, size_t& pos);
    
    std::string preprocessCppCode(const std::string& sourceCode);
    std::string extractFunctionSignature(const std::string& line);
    std::string extractClassSignature(const std::string& line);
    std::string extractConstructorSignature(const std::string& line);
    std::string extractMethodSignature(const std::string& line);
    bool isCppComment(const std::string& line);
    bool isCppPreprocessor(const std::string& line);
    bool isCppClassDeclaration(const std::string& line);
    bool isCppConstructorDeclaration(const std::string& line);
    bool isCppMethodDeclaration(const std::string& line);
    bool isCppFunctionDeclaration(const std::string& line);
    bool isCppVariableDeclaration(const std::string& line);
    bool isCppIncludeDirective(const std::string& line);
    bool isCppNamespaceDeclaration(const std::string& line);
    
    std::vector<std::string> splitCppStatements(const std::string& sourceCode);
    std::string trimCppString(const std::string& str);
    std::string removeCppComments(const std::string& code);
    std::string extractCppReturnType(const std::string& signature);
    std::string extractCppFunctionName(const std::string& signature);
    std::vector<std::string> extractCppParameters(const std::string& signature);
    std::string extractCppClassName(const std::string& signature);
    std::string extractCppMethodName(const std::string& signature);
    
    bool isInsideCppFunction(const std::string& sourceCode, size_t pos);
    bool isInsideCppClass(const std::string& sourceCode, size_t pos);
    size_t findMatchingCppBrace(const std::string& code, size_t openPos);
    size_t findMatchingCppParenthesis(const std::string& expr, size_t openPos);
};
