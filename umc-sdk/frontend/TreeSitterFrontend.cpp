#include "TreeSitterFrontend.h"
#include "ASTNode.h"
#include <iostream>
#include <fstream>
#include <sstream>

TreeSitterFrontend::TreeSitterFrontend(const std::string& language) 
    : language_(language), parser_(nullptr), tree_sitter_language_(nullptr), tree_(nullptr) {
    initializeParser();
}

TreeSitterFrontend::~TreeSitterFrontend() {
    cleanupParser();
}

bool TreeSitterFrontend::initializeParser() {
    // Initialize Tree-sitter
    // Note: ts_tree_external_scanner_reset might not be available in all versions
    // ts_tree_external_scanner_reset(nullptr);
    
    // Create parser
    parser_ = ts_parser_new();
    if (!parser_) {
        std::cerr << "Failed to create Tree-sitter parser" << std::endl;
        return false;
    }
    
    // Get language for the specified language
    tree_sitter_language_ = getLanguageForName(language_);
    if (!tree_sitter_language_) {
        std::cerr << "Unsupported language: " << language_ << std::endl;
        return false;
    }
    
    // Set language
    ts_parser_set_language(parser_, tree_sitter_language_);
    
    std::cout << "Tree-sitter parser initialized for " << language_ << std::endl;
    return true;
}

void TreeSitterFrontend::cleanupParser() {
    if (tree_) {
        ts_tree_delete(tree_);
        tree_ = nullptr;
    }
    
    if (parser_) {
        ts_parser_delete(parser_);
        parser_ = nullptr;
    }
    
    ast_.reset();
}

TSLanguage* TreeSitterFrontend::getLanguageForName(const std::string& name) {
    // For now, only C is supported via Tree-sitter
    // In a real implementation, we would dynamically load language libraries
    if (name == "c" || name == "cpp") {
        // Tree-sitter C language would be loaded here
        // For now, return nullptr as we need to implement the actual loading
        std::cout << "Tree-sitter C language support not yet implemented" << std::endl;
        return nullptr;
    }
    
    return nullptr;
}

bool TreeSitterFrontend::parse(const std::string& sourceCode) {
    if (!parser_ || !tree_sitter_language_) {
        std::cerr << "Parser not initialized" << std::endl;
        return false;
    }
    
    // Clean up previous tree
    if (tree_) {
        ts_tree_delete(tree_);
        tree_ = nullptr;
    }
    
    // Parse the source code
    tree_ = ts_parser_parse_string(parser_, nullptr, sourceCode.c_str(), sourceCode.length());
    if (!tree_) {
        std::cerr << "Failed to parse source code" << std::endl;
        return false;
    }
    
    // Convert Tree-sitter tree to our AST
    TSNode rootNode = ts_tree_root_node(tree_);
    ast_ = convertTreeToAST(rootNode);
    
    if (!ast_) {
        std::cerr << "Failed to convert Tree-sitter tree to AST" << std::endl;
        return false;
    }
    
    std::cout << "Successfully parsed with Tree-sitter" << std::endl;
    return true;
}

std::unique_ptr<ASTNode> TreeSitterFrontend::getAST() const {
    return ast_ ? ast_->clone() : nullptr;
}

std::unique_ptr<ASTNode> TreeSitterFrontend::convertTreeToAST(TSNode node) {
    if (ts_node_is_null(node)) {
        return nullptr;
    }
    
    std::string nodeText = getNodeText(node, ""); // We'll need source code here
    std::string nodeTypeStr = getNodeTypeString(node);
    ASTNodeType nodeType = mapNodeType(ts_node_symbol(node), nodeTypeStr);
    
    // Create AST node
    auto astNode = std::make_unique<ASTNode>(nodeType, nodeText);
    
    // Process children
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = convertTreeToAST(child);
        if (childAST) {
            astNode->addChild(std::move(childAST));
        }
    }
    
    return astNode;
}

ASTNodeType TreeSitterFrontend::mapNodeType(TSSymbol symbol, const std::string& typeName) {
    // Map Tree-sitter node types to our AST node types
    if (typeName == "translation_unit" || typeName == "source_file") {
        return ASTNodeType::PROGRAM;
    }
    else if (typeName == "function_definition" || typeName == "function_declaration") {
        return ASTNodeType::FUNCTION_DECL;
    }
    else if (typeName == "declaration" || typeName.find("declaration") != std::string::npos) {
        return ASTNodeType::VARIABLE_DECL;
    }
    else if (typeName == "call_expression") {
        return ASTNodeType::FUNCTION_CALL;
    }
    else if (typeName == "return_statement") {
        return ASTNodeType::RETURN_STMT;
    }
    else if (typeName == "binary_expression") {
        return ASTNodeType::BINARY_OP;
    }
    else if (typeName == "number_literal" || typeName == "string_literal" || 
             typeName == "character_literal" || typeName == "identifier") {
        return ASTNodeType::LITERAL;
    }
    
    // Default to program node for unknown types
    return ASTNodeType::PROGRAM;
}

std::string TreeSitterFrontend::getNodeText(TSNode node, const std::string& sourceCode) {
    // Extract text from source code using node's byte range
    uint32_t startByte = ts_node_start_byte(node);
    uint32_t endByte = ts_node_end_byte(node);
    
    if (startByte < sourceCode.length() && endByte <= sourceCode.length()) {
        return sourceCode.substr(startByte, endByte - startByte);
    }
    
    // Fallback to node type name
    return getNodeTypeString(node);
}

std::string TreeSitterFrontend::getNodeTypeString(TSNode node) {
    const char* type = ts_node_type(node);
    return type ? std::string(type) : "unknown";
}

uint32_t TreeSitterFrontend::getChildCount(TSNode node) {
    return ts_node_child_count(node);
}

TSNode TreeSitterFrontend::getChild(TSNode node, uint32_t index) {
    return ts_node_child(node, index);
}
