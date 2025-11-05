#include "CFrontend.h"
#include <iostream>
#include <fstream>
#include <cstring>

CFrontend::CFrontend() : language_(Language::C) {
    index_ = clang_createIndex(0, 0);
    translationUnit_ = nullptr;
    ast_ = nullptr;
}

CFrontend::~CFrontend() {
    if (translationUnit_) {
        clang_disposeTranslationUnit(translationUnit_);
    }
    clang_disposeIndex(index_);
}

bool CFrontend::parse(const std::string& sourceCode) {
    sourceCode_ = sourceCode;

    // Create temporary file for libclang
    std::string tempFile = "temp.c";
    std::ofstream outFile(tempFile);
    outFile << sourceCode;
    outFile.close();

    const char* args[] = {
        "-std=c11",
        "-I.",
        "-I/usr/include",
        "-I/usr/lib/clang/17/include"
    };

    CXErrorCode error = clang_parseTranslationUnit2(
        index_,
        tempFile.c_str(),
        args,
        sizeof(args) / sizeof(args[0]),
        nullptr,
        0,
        CXTranslationUnit_None,
        &translationUnit_
    );

    if (error != CXError_Success) {
        std::cerr << "Error parsing C code: " << error << std::endl;
        return false;
    }

    // Build AST from translation unit
    CXCursor cursor = clang_getTranslationUnitCursor(translationUnit_);
    ast_ = std::make_unique<ProgramNode>();
    traverseAST(cursor, ast_.get());

    // Clean up temp file
    remove(tempFile.c_str());

    return true;
}

std::unique_ptr<ASTNode> CFrontend::getAST() const {
    return std::make_unique<ProgramNode>(*ast_);
}

std::unique_ptr<ASTNode> CFrontend::buildASTFromCursor(CXCursor cursor) {
    return createNodeFromCursor(cursor);
}

std::unique_ptr<ASTNode> CFrontend::createNodeFromCursor(CXCursor cursor) {
    CXCursorKind kind = clang_getCursorKind(cursor);
    std::string cursorName = getCursorSpelling(cursor);

    switch (kind) {
        case CXCursor_FunctionDecl: {
            std::string returnType = getCursorType(cursor);
            auto node = std::make_unique<FunctionDeclNode>(cursorName, returnType);
            traverseAST(cursor, node.get());
            return node;
        }

        case CXCursor_VarDecl: {
            std::string varType = getCursorType(cursor);
            return std::make_unique<VariableDeclNode>(cursorName, varType);
        }

        case CXCursor_IntegerLiteral:
        case CXCursor_FloatingLiteral:
        case CXCursor_StringLiteral: {
            return std::make_unique<ASTNode>(ASTNodeType::LITERAL, cursorName);
        }

        case CXCursor_BinaryOperator: {
            CXCursor left = clang_getCursorReferenced(cursor);
            CXCursor right = clang_getCursorReferenced(cursor);

            auto leftNode = buildASTFromCursor(left);
            auto rightNode = buildASTFromCursor(right);

            std::string op = getBinaryOperator(cursor);
            return std::make_unique<BinaryOpNode>(op, std::move(leftNode), std::move(rightNode));
        }

        case CXCursor_IfStmt: {
            auto node = std::make_unique<ASTNode>(ASTNodeType::IF_STMT);
            traverseAST(cursor, node.get());
            return node;
        }

        case CXCursor_ForStmt: {
            auto node = std::make_unique<ASTNode>(ASTNodeType::FOR_LOOP);
            traverseAST(cursor, node.get());
            return node;
        }

        case CXCursor_WhileStmt: {
            auto node = std::make_unique<ASTNode>(ASTNodeType::WHILE_LOOP);
            traverseAST(cursor, node.get());
            return node;
        }

        case CXCursor_ReturnStmt: {
            auto node = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT);
            traverseAST(cursor, node.get());
            return node;
        }

        case CXCursor_CompoundStmt: {
            auto node = std::make_unique<ASTNode>(ASTNodeType::BLOCK);
            traverseAST(cursor, node.get());
            return node;
        }

        default:
            if (!clang_isDeclaration(kind) && !cursorName.empty()) {
                return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, cursorName);
            }
            return nullptr;
    }
}

std::string CFrontend::getCursorSpelling(CXCursor cursor) {
    CXString spelling = clang_getCursorSpelling(cursor);
    std::string result = clang_getCString(spelling);
    clang_disposeString(spelling);
    return result;
}

std::string CFrontend::getCursorType(CXCursor cursor) {
    CXType type = clang_getCursorType(cursor);
    CXString typeSpelling = clang_getTypeSpelling(type);
    std::string result = clang_getCString(typeSpelling);
    clang_disposeString(typeSpelling);
    return result;
}

std::string CFrontend::getBinaryOperator(CXCursor cursor) {
    // This is a simplified implementation
    // In a full implementation, you'd analyze the operator kind
    return "+";  // Placeholder
}

void CFrontend::traverseAST(CXCursor cursor, ASTNode* parent) {
    clang_visitChildren(cursor, [](CXCursor child, CXCursor parent, CXClientData clientData) {
        CFrontend* frontend = static_cast<CFrontend*>(clientData);
        ASTNode* parentNode = static_cast<ASTNode*>(clientData);

        auto childNode = frontend->buildASTFromCursor(child);
        if (childNode) {
            parentNode->addChild(std::move(childNode));
        }

        return CXChildVisit_Continue;
    }, this);
}

// Factory function for creating C frontend
std::unique_ptr<FrontendBase> createCFrontend() {
    return std::make_unique<CFrontend>();
}
