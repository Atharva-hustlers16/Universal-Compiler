#include "FrontendBase.h"
#include "ASTNode.h"
#include <iostream>

// Forward declarations
std::unique_ptr<FrontendBase> createCFrontend();
std::unique_ptr<FrontendBase> createJavaFrontend();
std::unique_ptr<FrontendBase> createPythonFrontend();

// Simple AST Node implementation for demonstration
class SimpleASTNode : public ASTNode {
public:
    SimpleASTNode(const std::string& type, const std::string& value = "")
        : type_(type), value_(value) {}

    std::string toString() const override {
        return "ASTNode(" + type_ + ", " + value_ + ")";
    }

private:
    std::string type_;
    std::string value_;
};

// Base frontend implementation
class BasicFrontend : public FrontendBase {
public:
    BasicFrontend(Language lang) : language_(lang) {}

    bool parse(const std::string& sourceCode) override {
        sourceCode_ = sourceCode;

        // Placeholder: create a simple AST node
        ast_ = std::make_unique<SimpleASTNode>("program", "parsed");

        std::cout << "Parsed " << LanguageDetector::languageToString(language_)
                  << " code successfully" << std::endl;
        return true;
    }

    std::unique_ptr<ASTNode> getAST() const override {
        return std::make_unique<SimpleASTNode>(*ast_);
    }

    Language getLanguage() const override {
        return language_;
    }

private:
    Language language_;
    std::string sourceCode_;
    std::unique_ptr<SimpleASTNode> ast_;
};

// Factory function for creating frontends
std::unique_ptr<FrontendBase> createFrontend(Language lang) {
    switch (lang) {
        case Language::C:
            return createCFrontend();
        case Language::CPP:
            // For now, use same as C frontend
            return createCFrontend();
        case Language::JAVA:
            return createJavaFrontend();
        case Language::PYTHON:
            return createPythonFrontend();
        default:
            return std::make_unique<BasicFrontend>(Language::UNKNOWN);
    }
}
