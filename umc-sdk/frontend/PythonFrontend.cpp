#include "PythonFrontend.h"
#include <iostream>
#include <cstring>
#include <vector>

// Check if Tree-sitter is available
#ifdef TREESITTER_AVAILABLE
extern "C" {
    #include <tree_sitter/api.h>
    TSLanguage* tree_sitter_python();
}
#endif

PythonFrontend::PythonFrontend() : language_(Language::PYTHON) {
    parser_ = nullptr;
    tree_ = nullptr;
    ast_ = nullptr;

#ifdef TREESITTER_AVAILABLE
    // Initialize Tree-sitter parser
    parser_ = ts_parser_new();
    if (parser_) {
        ts_parser_set_language(parser_, tree_sitter_python());
    }
#endif
}

PythonFrontend::~PythonFrontend() {
#ifdef TREESITTER_AVAILABLE
    if (tree_) {
        ts_tree_delete(tree_);
    }
    if (parser_) {
        ts_parser_delete(parser_);
    }
#endif
}

bool PythonFrontend::parse(const std::string& sourceCode) {
    sourceCode_ = sourceCode;

    if (!parser_) {
        std::cerr << "Error: Tree-sitter parser not initialized" << std::endl;
        return false;
    }

#ifdef TREESITTER_AVAILABLE
    // Use Tree-sitter parser
    return parseWithTreeSitter(sourceCode);
#else
    // Use fallback parser
    return parseWithFallback(sourceCode);
#endif
}

#ifdef TREESITTER_AVAILABLE
bool PythonFrontend::parseWithTreeSitter(const std::string& sourceCode) {
    // Parse the source code
    TSTree* newTree = ts_parser_parse_string(
        parser_,
        nullptr,  // old_tree
        sourceCode.c_str(),
        sourceCode.length()
    );

    if (!newTree) {
        std::cerr << "Error: Failed to parse Python code" << std::endl;
        return false;
    }

    // Clean up old tree
    if (tree_) {
        ts_tree_delete(tree_);
    }
    tree_ = newTree;

    // Build AST from Tree-sitter tree
    TSNode rootNode = ts_tree_root_node(tree_);
    ast_ = std::unique_ptr<ProgramNode>(dynamic_cast<ProgramNode*>(visitModule(rootNode).release()));

    return true;
}
#endif

bool PythonFrontend::parseWithFallback(const std::string& sourceCode) {
    // Fallback parser for when Tree-sitter is not available
    std::cout << "Using fallback Python parser (Tree-sitter not available)" << std::endl;

    // Simple heuristic-based parsing for basic Python structures
    ast_ = std::make_unique<ProgramNode>();

    // Look for function definitions
    size_t defPos = sourceCode.find("def ");
    if (defPos != std::string::npos) {
        size_t funcNameStart = defPos + 4;
        size_t funcNameEnd = sourceCode.find_first_of(" (", funcNameStart);
        std::string funcName = sourceCode.substr(funcNameStart, funcNameEnd - funcNameStart);

        auto funcNode = std::make_unique<FunctionDeclNode>(funcName, "def");

        // Look for function body
        size_t colonPos = sourceCode.find(":", funcNameEnd);
        if (colonPos != std::string::npos) {
            size_t bodyStart = colonPos + 1;
            // Simple body detection (just check for indentation)
            if (sourceCode.length() > bodyStart) {
                auto bodyNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK);
                funcNode->addChild(std::move(bodyNode));
            }
        }

        ast_->addChild(std::move(funcNode));
    }

    return true;
}

std::unique_ptr<ASTNode> PythonFrontend::getAST() const {
    return std::make_unique<ProgramNode>(*ast_);
}

std::unique_ptr<ASTNode> PythonFrontend::buildASTFromTSNode(TSNode node) {
    return createNodeFromTSNode(node);
}

std::unique_ptr<ASTNode> PythonFrontend::createNodeFromTSNode(TSNode node) {
    std::string nodeType = getNodeType(node);

    if (nodeType == "module") {
        return visitModule(node);
    } else if (nodeType == "function_definition") {
        return visitFunctionDefinition(node);
    } else if (nodeType == "class_definition") {
        return visitClassDefinition(node);
    } else if (nodeType == "assignment") {
        return visitAssignment(node);
    } else if (nodeType == "expression_statement") {
        return visitExpressionStatement(node);
    } else if (nodeType == "if_statement") {
        return visitIfStatement(node);
    } else if (nodeType == "for_statement") {
        return visitForStatement(node);
    } else if (nodeType == "while_statement") {
        return visitWhileStatement(node);
    } else if (nodeType == "return_statement") {
        return visitReturnStatement(node);
    } else if (nodeType == "binary_operator") {
        return visitBinaryOperator(node);
    } else if (nodeType == "identifier") {
        return visitIdentifier(node);
    } else if (nodeType == "string") {
        return visitString(node);
    } else if (nodeType == "integer") {
        return visitInteger(node);
    } else if (nodeType == "float") {
        return visitFloat(node);
    } else if (nodeType == "true" || nodeType == "false") {
        return visitBoolean(node);
    } else if (nodeType == "none") {
        return visitNone(node);
    } else if (nodeType == "list") {
        return visitList(node);
    } else if (nodeType == "dictionary") {
        return visitDictionary(node);
    } else if (nodeType == "call") {
        return visitCall(node);
    }

    return nullptr;
}

std::string PythonFrontend::getNodeType(TSNode node) {
#ifdef TREESITTER_AVAILABLE
    return std::string(ts_node_type(node));
#else
    return "unknown";
#endif
}

std::string PythonFrontend::getNodeText(TSNode node, const std::string& source) {
#ifdef TREESITTER_AVAILABLE
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    return source.substr(start, end - start);
#else
    return "";
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitModule(TSNode node) {
    auto program = std::make_unique<ProgramNode>();

#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childNode = buildASTFromTSNode(child);
        if (childNode) {
            program->addChild(std::move(childNode));
        }
    }
#endif

    return program;
}

std::unique_ptr<ASTNode> PythonFrontend::visitFunctionDefinition(TSNode node) {
    // Extract function name
#ifdef TREESITTER_AVAILABLE
    TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
    std::string funcName = getNodeText(nameNode, sourceCode_);
#else
    std::string funcName = "unknown_function";
#endif

    auto funcNode = std::make_unique<FunctionDeclNode>(funcName, "def");

    // Process function body
#ifdef TREESITTER_AVAILABLE
    TSNode bodyNode = ts_node_child_by_field_name(node, "body", 4);
    if (!ts_node_is_null(bodyNode)) {
        auto bodyAST = buildASTFromTSNode(bodyNode);
        if (bodyAST) {
            funcNode->addChild(std::move(bodyAST));
        }
    }
#endif

    return funcNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitClassDefinition(TSNode node) {
    // Extract class name
#ifdef TREESITTER_AVAILABLE
    TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
    std::string className = getNodeText(nameNode, sourceCode_);
#else
    std::string className = "unknown_class";
#endif

    auto classNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, className);

    // Process class body
#ifdef TREESITTER_AVAILABLE
    TSNode bodyNode = ts_node_child_by_field_name(node, "body", 4);
    if (!ts_node_is_null(bodyNode)) {
        auto bodyAST = buildASTFromTSNode(bodyNode);
        if (bodyAST) {
            classNode->addChild(std::move(bodyAST));
        }
    }
#endif

    return classNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitAssignment(TSNode node) {
    auto assignNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "=");

    // Process left and right sides
#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        std::string childType = getNodeType(child);

        if (childType != "=") {  // Skip the operator itself
            auto childAST = buildASTFromTSNode(child);
            if (childAST) {
                assignNode->addChild(std::move(childAST));
            }
        }
    }
#endif

    return assignNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitExpressionStatement(TSNode node) {
    // Get the expression child
#ifdef TREESITTER_AVAILABLE
    TSNode exprNode = ts_node_child(node, 0);
    return buildASTFromTSNode(exprNode);
#else
    return nullptr;
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitIfStatement(TSNode node) {
    auto ifNode = std::make_unique<ASTNode>(ASTNodeType::IF_STMT);

    // Process condition and body
#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            ifNode->addChild(std::move(childAST));
        }
    }
#endif

    return ifNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitForStatement(TSNode node) {
    auto forNode = std::make_unique<ASTNode>(ASTNodeType::FOR_LOOP);

    // Process target, iterable, and body
#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            forNode->addChild(std::move(childAST));
        }
    }
#endif

    return forNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitWhileStatement(TSNode node) {
    auto whileNode = std::make_unique<ASTNode>(ASTNodeType::WHILE_LOOP);

    // Process condition and body
#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            whileNode->addChild(std::move(childAST));
        }
    }
#endif

    return whileNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitReturnStatement(TSNode node) {
    auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT);

    // Process return value if present
#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        if (getNodeType(child) != "return") {  // Skip the 'return' keyword
            auto childAST = buildASTFromTSNode(child);
            if (childAST) {
                returnNode->addChild(std::move(childAST));
            }
        }
    }
#endif

    return returnNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitBinaryOperator(TSNode node) {
#ifdef TREESITTER_AVAILABLE
    std::string op = getNodeText(node, sourceCode_);
    auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, op);

    // Process operands
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            binOpNode->addChild(std::move(childAST));
        }
    }

    return binOpNode;
#else
    return std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");  // Placeholder
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitIdentifier(TSNode node) {
#ifdef TREESITTER_AVAILABLE
    std::string name = getNodeText(node, sourceCode_);
    return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, name);
#else
    return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, "unknown");
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitString(TSNode node) {
#ifdef TREESITTER_AVAILABLE
    std::string value = getNodeText(node, sourceCode_);
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, value);
#else
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "\"string\"");
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitInteger(TSNode node) {
#ifdef TREESITTER_AVAILABLE
    std::string value = getNodeText(node, sourceCode_);
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, value);
#else
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "42");
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitFloat(TSNode node) {
#ifdef TREESITTER_AVAILABLE
    std::string value = getNodeText(node, sourceCode_);
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, value);
#else
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "3.14");
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitBoolean(TSNode node) {
#ifdef TREESITTER_AVAILABLE
    std::string value = getNodeText(node, sourceCode_);
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, value);
#else
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "True");
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitNone(TSNode node) {
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "None");
}

std::unique_ptr<ASTNode> PythonFrontend::visitList(TSNode node) {
    auto listNode = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "[]");

    // Process list elements
#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            listNode->addChild(std::move(childAST));
        }
    }
#endif

    return listNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitDictionary(TSNode node) {
    auto dictNode = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "{}");

    // Process dictionary key-value pairs
#ifdef TREESITTER_AVAILABLE
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            dictNode->addChild(std::move(childAST));
        }
    }
#endif

    return dictNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitCall(TSNode node) {
    auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL);

    // Get function name
#ifdef TREESITTER_AVAILABLE
    TSNode funcNode = ts_node_child_by_field_name(node, "function", 8);
    if (!ts_node_is_null(funcNode)) {
        auto funcAST = buildASTFromTSNode(funcNode);
        if (funcAST) {
            callNode->addChild(std::move(funcAST));
        }
    }

    // Process arguments
    TSNode argsNode = ts_node_child_by_field_name(node, "arguments", 9);
    if (!ts_node_is_null(argsNode)) {
        uint32_t argCount = ts_node_child_count(argsNode);
        for (uint32_t i = 0; i < argCount; ++i) {
            TSNode arg = ts_node_child(argsNode, i);
            auto argAST = buildASTFromTSNode(arg);
            if (argAST) {
                callNode->addChild(std::move(argAST));
            }
        }
    }
#endif

    return callNode;
}

// Factory function for creating Python frontend
std::unique_ptr<FrontendBase> createPythonFrontend() {
    return std::make_unique<PythonFrontend>();
}
