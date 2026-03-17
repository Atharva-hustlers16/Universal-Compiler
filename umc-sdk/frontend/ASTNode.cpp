#include "ASTNode.h"
#include <iostream>
#include <sstream>

ASTNode::ASTNode(ASTNodeType type, const std::string& value) : type_(type), value_(value) {}

void ASTNode::addChild(std::unique_ptr<ASTNode> child) {
    children_.push_back(std::move(child));
}

std::string ASTNode::toString(int indent) const {
    std::ostringstream oss;
    for (int i = 0; i < indent; ++i) {
        oss << "  ";
    }

    oss << getNodeTypeName(type_) << " (" << value_ << ")";
    if (!children_.empty()) {
        oss << " {" << std::endl;
        for (const auto& child : children_) {
            oss << child->toString(indent + 1);
        }
        for (int i = 0; i < indent; ++i) {
            oss << "  ";
        }
        oss << "}";
    }
    oss << std::endl;
    return oss.str();
}

std::string getNodeTypeName(ASTNodeType type) {
    switch (type) {
        case ASTNodeType::PROGRAM: return "Program";
        case ASTNodeType::FUNCTION_DECL: return "FunctionDecl";
        case ASTNodeType::VARIABLE_DECL: return "VariableDecl";
        case ASTNodeType::FUNCTION_CALL: return "FunctionCall";
        case ASTNodeType::BINARY_OP: return "BinaryOp";
        case ASTNodeType::LITERAL: return "Literal";
        case ASTNodeType::IDENTIFIER: return "Identifier";
        case ASTNodeType::IF_STMT: return "IfStmt";
        case ASTNodeType::FOR_LOOP: return "ForLoop";
        case ASTNodeType::WHILE_LOOP: return "WhileLoop";
        case ASTNodeType::RETURN_STMT: return "ReturnStmt";
        case ASTNodeType::BLOCK: return "Block";
        default: return "Unknown";
    }
}

std::unique_ptr<ASTNode> ASTNode::clone() const {
    auto cloned = std::make_unique<ASTNode>(type_, value_);
    for (const auto& child : children_) {
        cloned->addChild(child->clone());
    }
    return cloned;
}
