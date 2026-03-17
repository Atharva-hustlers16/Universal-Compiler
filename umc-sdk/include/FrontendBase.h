#ifndef FRONTEND_BASE_H
#define FRONTEND_BASE_H

#include <string>
#include <memory>
#include "LanguageDetector.h"
#include "ASTNode.h"

class FrontendBase {
public:
    virtual ~FrontendBase() = default;

    virtual bool parse(const std::string& sourceCode) = 0;
    virtual std::unique_ptr<ASTNode> getAST() const = 0;
    virtual Language getLanguage() const = 0;

protected:
    Language language_;
    std::string sourceCode_;
    std::unique_ptr<ASTNode> ast_;
};

#endif // FRONTEND_BASE_H
