#include "PythonFrontend.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <stdexcept>

// Check if Tree-sitter is available
#ifdef ENABLE_TREESITTER
extern "C" {
    #include <tree_sitter/api.h>
    TSLanguage* tree_sitter_python();
}
#endif

PythonFrontend::PythonFrontend() {
    language_ = Language::PYTHON;
    parser_ = nullptr;
    tree_ = nullptr;
    ast_ = nullptr;

#ifdef ENABLE_TREESITTER
    // Initialize Tree-sitter parser
    parser_ = ts_parser_new();
    if (parser_) {
        ts_parser_set_language(parser_, tree_sitter_python());
    }
#else
    // Without Tree-sitter, we can still create an empty AST
    ast_ = std::make_unique<ProgramNode>();
#endif
}

PythonFrontend::~PythonFrontend() {
#ifdef ENABLE_TREESITTER
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

#ifdef ENABLE_TREESITTER
    if (!parser_) {
        std::cerr << "Error: Tree-sitter parser not initialized" << std::endl;
        return false;
    }
    // Use Tree-sitter parser
    return parseWithTreeSitter(sourceCode);
#else
    // Enhanced fallback parser for when Tree-sitter is not available
    std::cout << "Using enhanced fallback Python parser (Tree-sitter not available)" << std::endl;
    
    ast_ = std::make_unique<ProgramNode>();
    
    // Parse functions and statements
    parsePythonSource(sourceCode);
    
    return true;
#endif
}

#ifdef ENABLE_TREESITTER
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

std::unique_ptr<ASTNode> PythonFrontend::getAST() const {
    // Use the clone method for proper deep copy
    if (ast_) {
        return ast_->clone();
    }
    return nullptr;
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
#ifdef ENABLE_TREESITTER
    return std::string(ts_node_type(node));
#else
    (void)node; // Suppress unused parameter warning
    return "unknown";
#endif
}

std::string PythonFrontend::getNodeText(TSNode node, const std::string& source) {
#ifdef ENABLE_TREESITTER
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    return source.substr(start, end - start);
#else
    (void)node;    // Suppress unused parameter warning
    (void)source;  // Suppress unused parameter warning
    return "";
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitModule(TSNode node) {
    auto program = std::make_unique<ProgramNode>();

#ifdef ENABLE_TREESITTER
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            program->addChild(std::move(childAST));
        }
    }
#else
    (void)node; // Suppress unused parameter warning
#endif

    return program;
}

std::unique_ptr<ASTNode> PythonFrontend::visitFunctionDefinition(TSNode node) {
    // Extract function name
#ifdef ENABLE_TREESITTER
    TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
    std::string funcName = getNodeText(nameNode, sourceCode_);
#else
    (void)node; // Suppress unused parameter warning
    std::string funcName = "unknown_function";
#endif

    auto funcNode = std::make_unique<FunctionDeclNode>(funcName, "def");

    // Process function body
#ifdef ENABLE_TREESITTER
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
#ifdef ENABLE_TREESITTER
    TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
    std::string className = getNodeText(nameNode, sourceCode_);
#else
    (void)node; // Suppress unused parameter warning
    std::string className = "unknown_class";
#endif

    auto classNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, className);

    // Process class body
#ifdef ENABLE_TREESITTER
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
#ifdef ENABLE_TREESITTER
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
#else
    (void)node; // Suppress unused parameter warning
#endif

    return assignNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitExpressionStatement(TSNode node) {
    // Get the expression child
#ifdef ENABLE_TREESITTER
    TSNode exprNode = ts_node_child(node, 0);
    return buildASTFromTSNode(exprNode);
#else
    (void)node; // Suppress unused parameter warning
    return nullptr;
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitIfStatement(TSNode node) {
    auto ifNode = std::make_unique<ASTNode>(ASTNodeType::IF_STMT);

    // Process condition and body
#ifdef ENABLE_TREESITTER
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            ifNode->addChild(std::move(childAST));
        }
    }
#else
    (void)node; // Suppress unused parameter warning
#endif

    return ifNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitForStatement(TSNode node) {
    auto forNode = std::make_unique<ASTNode>(ASTNodeType::FOR_LOOP);

    // Process target, iterable, and body
#ifdef ENABLE_TREESITTER
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            forNode->addChild(std::move(childAST));
        }
    }
#else
    (void)node; // Suppress unused parameter warning
#endif

    return forNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitWhileStatement(TSNode node) {
    auto whileNode = std::make_unique<ASTNode>(ASTNodeType::WHILE_LOOP);

    // Process condition and body
#ifdef ENABLE_TREESITTER
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            whileNode->addChild(std::move(childAST));
        }
    }
#else
    (void)node; // Suppress unused parameter warning
#endif

    return whileNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitReturnStatement(TSNode node) {
    auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT);

    // Process return value if present
#ifdef ENABLE_TREESITTER
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
#else
    (void)node; // Suppress unused parameter warning
#endif

    return returnNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitBinaryOperator(TSNode node) {
#ifdef ENABLE_TREESITTER
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
    (void)node; // Suppress unused parameter warning
    return std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");  // Placeholder
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitIdentifier(TSNode node) {
#ifdef ENABLE_TREESITTER
    std::string name = getNodeText(node, sourceCode_);
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::IDENTIFIER, name));
    return result;
#else
    (void)node; // Suppress unused parameter warning
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::IDENTIFIER, std::string("unknown")));
    return result;
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitString(TSNode node) {
#ifdef ENABLE_TREESITTER
    std::string value = getNodeText(node, sourceCode_);
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, value));
    return result;
#else
    (void)node; // Suppress unused parameter warning
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, std::string("\"string\"")));
    return result;
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitInteger(TSNode node) {
#ifdef ENABLE_TREESITTER
    std::string value = getNodeText(node, sourceCode_);
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, value));
    return result;
#else
    (void)node; // Suppress unused parameter warning
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, std::string("42")));
    return result;
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitFloat(TSNode node) {
#ifdef ENABLE_TREESITTER
    std::string value = getNodeText(node, sourceCode_);
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, value));
    return result;
#else
    (void)node; // Suppress unused parameter warning
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, std::string("3.14")));
    return result;
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitBoolean(TSNode node) {
#ifdef ENABLE_TREESITTER
    std::string value = getNodeText(node, sourceCode_);
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, value));
    return result;
#else
    (void)node; // Suppress unused parameter warning
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, std::string("True")));
    return result;
#endif
}

std::unique_ptr<ASTNode> PythonFrontend::visitNone(TSNode node) {
    (void)node; // Suppress unused parameter warning
    std::unique_ptr<ASTNode> result(new ASTNode(ASTNodeType::LITERAL, std::string("None")));
    return result;
}

std::unique_ptr<ASTNode> PythonFrontend::visitList(TSNode node) {
    std::unique_ptr<ASTNode> listNode(new ASTNode(ASTNodeType::LITERAL, std::string("[]")));

    // Process list elements
#ifdef ENABLE_TREESITTER
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            listNode->addChild(std::move(childAST));
        }
    }
#else
    (void)node; // Suppress unused parameter warning
#endif

    return listNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitDictionary(TSNode node) {
    std::unique_ptr<ASTNode> dictNode(new ASTNode(ASTNodeType::LITERAL, std::string("{}")));

    // Process dictionary key-value pairs
#ifdef ENABLE_TREESITTER
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = buildASTFromTSNode(child);
        if (childAST) {
            dictNode->addChild(std::move(childAST));
        }
    }
#else
    (void)node; // Suppress unused parameter warning
#endif

    return dictNode;
}

std::unique_ptr<ASTNode> PythonFrontend::visitCall(TSNode node) {
    std::unique_ptr<ASTNode> callNode(new ASTNode(ASTNodeType::FUNCTION_CALL));

    // Get function name
#ifdef ENABLE_TREESITTER
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
#else
    (void)node; // Suppress unused parameter warning
#endif

    return callNode;
}

// Factory function for creating Python frontend
std::unique_ptr<FrontendBase> createPythonFrontend() {
    return std::make_unique<PythonFrontend>();
}

// Enhanced fallback parser implementations
void PythonFrontend::parsePythonSource(const std::string& sourceCode) {
    // Preprocess the source code to remove comments and handle imports
    std::string cleanCode = preprocessPythonCode(sourceCode);
    
    // Parse functions first
    parsePythonFunctions(cleanCode);
    
    // Parse remaining global statements (skip function bodies)
    parseGlobalStatements(cleanCode);
}

void PythonFrontend::parsePythonFunctions(const std::string& sourceCode) {
    size_t pos = 0;
    while (pos < sourceCode.length()) {
        // Look for function definitions
        size_t defPos = sourceCode.find("def ", pos);
        if (defPos != std::string::npos) {
            // Make sure this is not inside another function (check indentation)
            if (!isInsidePythonFunction(sourceCode, defPos)) {
                size_t nameStart = defPos + 4;
                size_t nameEnd = sourceCode.find("(", nameStart);
                if (nameEnd != std::string::npos) {
                    std::string funcName = sourceCode.substr(nameStart, nameEnd - nameStart);
                    
                    // Trim whitespace from function name
                    size_t funcNameStart = funcName.find_first_not_of(" \t");
                    size_t funcNameEnd = funcName.find_last_not_of(" \t");
                    if (funcNameStart != std::string::npos && funcNameEnd != std::string::npos) {
                        funcName = funcName.substr(funcNameStart, funcNameEnd - funcNameStart + 1);
                    }
                    
                    auto funcNode = std::make_unique<FunctionDeclNode>(funcName, "def");
                    
                    // Parse function parameters
                    size_t paramEnd = sourceCode.find(")", nameEnd);
                    if (paramEnd != std::string::npos) {
                        std::string paramsStr = sourceCode.substr(nameEnd + 1, paramEnd - nameEnd - 1);
                        // Parse parameters (simplified - just add them as variable declarations)
                        if (!paramsStr.empty()) {
                            size_t paramPos = 0;
                            while (paramPos < paramsStr.length()) {
                                size_t commaPos = paramsStr.find(",", paramPos);
                                if (commaPos == std::string::npos) commaPos = paramsStr.length();
                                
                                std::string param = paramsStr.substr(paramPos, commaPos - paramPos);
                                size_t start = param.find_first_not_of(" \t");
                                size_t end = param.find_last_not_of(" \t");
                                if (start != std::string::npos && end != std::string::npos) {
                                    param = param.substr(start, end - start + 1);
                                }
                                
                                if (!param.empty()) {
                                    auto paramNode = std::make_unique<VariableDeclNode>(param, "param");
                                    funcNode->addChild(std::move(paramNode));
                                }
                                
                                paramPos = commaPos + 1;
                            }
                        }
                    }
                    
                    // Look for function body
                    size_t colonPos = sourceCode.find(":", paramEnd);
                    if (colonPos != std::string::npos) {
                        size_t bodyStart = sourceCode.find_first_not_of(" \n\r", colonPos + 1);
                        if (bodyStart != std::string::npos) {
                            // Find the end of the function body (next dedented line or end of file)
                            size_t bodyEnd = bodyStart;
                            
                            // Calculate the base indentation level (first non-empty line after colon)
                            size_t firstLineStart = bodyStart;
                            while (firstLineStart < sourceCode.length() && isspace(sourceCode[firstLineStart])) {
                                firstLineStart++;
                            }
                            
                            // Count the actual indentation of the first line
                            size_t baseIndent = firstLineStart - bodyStart;
                            
                            // Skip to the end of the first line
                            while (bodyEnd < sourceCode.length() && sourceCode[bodyEnd] != '\n') {
                                bodyEnd++;
                            }
                            
                            // Continue parsing until we find a line with less indentation
                            while (bodyEnd < sourceCode.length()) {
                                if (sourceCode[bodyEnd] == '\n') {
                                    size_t nextLineStart = bodyEnd + 1;
                                    
                                    // Skip empty lines and whitespace-only lines
                                    while (nextLineStart < sourceCode.length() && 
                                           (sourceCode[nextLineStart] == '\n' || sourceCode[nextLineStart] == '\r')) {
                                        nextLineStart++;
                                    }
                                    
                                    if (nextLineStart >= sourceCode.length()) break;
                                    
                                    // Count indentation of next line
                                    size_t lineIndent = 0;
                                    size_t checkPos = nextLineStart;
                                    while (checkPos < sourceCode.length() && sourceCode[checkPos] == ' ') {
                                        lineIndent++;
                                        checkPos++;
                                    }
                                    
                                    // If we find a line with less indentation and it's not empty, function ends
                                    if (checkPos < sourceCode.length() && 
                                        sourceCode[checkPos] != '\n' && sourceCode[checkPos] != '\r') {
                                        // Check if this is a new function definition
                                        if (checkPos + 3 < sourceCode.length() && 
                                            sourceCode.substr(checkPos, 4) == "def ") {
                                            break;
                                        }
                                        // If it's not a function but has less indentation, also break
                                        if (lineIndent < baseIndent) {
                                            break;
                                        }
                                        // Also break if we find a line with 0 indentation (global scope)
                                        if (lineIndent == 0) {
                                            break;
                                        }
                                    }
                                    
                                    // Skip to end of this line
                                    while (nextLineStart < sourceCode.length() && sourceCode[nextLineStart] != '\n') {
                                        nextLineStart++;
                                    }
                                    bodyEnd = nextLineStart;
                                } else {
                                    bodyEnd++;
                                }
                            }
                            
                            if (bodyEnd > bodyStart) {
                                std::string body = sourceCode.substr(bodyStart, bodyEnd - bodyStart);
                                
                                // Create a block node for function body
                                auto blockNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
                                
                                // Parse statements in the function body
                                parsePythonStatements(body, *blockNode);
                                
                                funcNode->addChild(std::move(blockNode));
                                
                                // Update position to skip the entire function
                                pos = bodyEnd;
                            } else {
                                pos = colonPos + 1;
                            }
                        } else {
                            pos = colonPos + 1;
                        }
                    } else {
                        pos = nameEnd + 1;
                    }
                    
                    ast_->addChild(std::move(funcNode));
                } else {
                    pos = defPos + 1;
                }
            } else {
                pos = defPos + 1;
            }
        } else {
            pos = sourceCode.length();
        }
    }
}

void PythonFrontend::parseGlobalStatements(const std::string& sourceCode) {
    size_t pos = 0;
    while (pos < sourceCode.length()) {
        // Skip whitespace and blank lines
        while (pos < sourceCode.length() && isspace(sourceCode[pos])) {
            pos++;
        }
        if (pos >= sourceCode.length()) break;
        
        // Skip function definitions (already parsed)
        if (pos + 3 < sourceCode.length() && sourceCode.substr(pos, 4) == "def ") {
            // Find the end of this function and skip it
            size_t defPos = pos;
            size_t nameEnd = sourceCode.find("(", defPos + 4);
            if (nameEnd != std::string::npos) {
                size_t paramEnd = sourceCode.find(")", nameEnd);
                if (paramEnd != std::string::npos) {
                    size_t colonPos = sourceCode.find(":", paramEnd);
                    if (colonPos != std::string::npos) {
                        size_t bodyStart = sourceCode.find_first_not_of(" \n\r", colonPos + 1);
                        if (bodyStart != std::string::npos) {
                            // Calculate base indentation
                            size_t baseIndent = 0;
                            while (bodyStart < sourceCode.length() && sourceCode[bodyStart] == ' ') {
                                baseIndent++;
                                bodyStart++;
                            }
                            
                            // Skip to end of function body
                            size_t bodyEnd = bodyStart;
                            while (bodyEnd < sourceCode.length()) {
                                if (sourceCode[bodyEnd] == '\n') {
                                    size_t nextLineStart = bodyEnd + 1;
                                    size_t lineIndent = 0;
                                    while (nextLineStart < sourceCode.length() && sourceCode[nextLineStart] == ' ') {
                                        lineIndent++;
                                        nextLineStart++;
                                    }
                                    
                                    if (lineIndent < baseIndent && nextLineStart < sourceCode.length() && 
                                        sourceCode[nextLineStart] != '\n') {
                                        break;
                                    }
                                    bodyEnd = nextLineStart;
                                } else {
                                    bodyEnd++;
                                }
                            }
                            pos = bodyEnd;
                            continue;
                        }
                    }
                }
            }
        }
        
        // Find the end of the current statement (newline or end of string)
        size_t stmtEnd = pos;
        while (stmtEnd < sourceCode.length() && sourceCode[stmtEnd] != '\n') {
            stmtEnd++;
        }
        
        std::string statement = sourceCode.substr(pos, stmtEnd - pos);
        
        // Trim whitespace
        size_t start = statement.find_first_not_of(" \t");
        size_t end = statement.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            statement = statement.substr(start, end - start + 1);
        }
        
        if (!statement.empty() && statement[0] != '#') {
            auto stmtNode = parsePythonStatement(statement);
            if (stmtNode) {
                ast_->addChild(std::move(stmtNode));
            }
        }
        
        pos = stmtEnd + 1;
    }
}

void PythonFrontend::parsePythonStatements(const std::string& body, ASTNode& blockNode) {
    size_t pos = 0;
    while (pos < body.length()) {
        // Skip whitespace and blank lines
        while (pos < body.length() && isspace(body[pos])) {
            pos++;
        }
        if (pos >= body.length()) break;
        
        // Find the end of the current statement (newline or end of string)
        size_t stmtEnd = pos;
        while (stmtEnd < body.length() && body[stmtEnd] != '\n') {
            stmtEnd++;
        }
        
        std::string statement = body.substr(pos, stmtEnd - pos);
        
        // Trim whitespace
        size_t start = statement.find_first_not_of(" \t");
        size_t end = statement.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            statement = statement.substr(start, end - start + 1);
        }
        
        if (!statement.empty() && statement[0] != '#') {
            auto stmtNode = parsePythonStatement(statement);
            if (stmtNode) {
                blockNode.addChild(std::move(stmtNode));
            }
        }
        
        pos = stmtEnd + 1;
    }
}

std::unique_ptr<ASTNode> PythonFrontend::parsePythonStatement(const std::string& statement) {
    // Check for return statements first
    if (statement.find("return") == 0) {
        size_t returnPos = 6;
        if (returnPos < statement.length()) {
            std::string expr = statement.substr(returnPos);
            auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT, "return");
            auto exprNode = parsePythonExpression(expr);
            if (exprNode) {
                returnNode->addChild(std::move(exprNode));
            }
            return std::move(returnNode);
        } else {
            return std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT, "return");
        }
    }
    
    // Check for variable assignments FIRST (but be careful about equals signs)
    size_t assignPos = statement.find("=");
    if (assignPos != std::string::npos && assignPos > 0) {
        // Make sure this is not inside a string literal
        bool inString = false;
        bool isValidAssignment = true;
        
        for (size_t i = 0; i < assignPos; i++) {
            char c = statement[i];
            if ((c == '"' || c == '\'') && (i == 0 || statement[i - 1] != '\\')) {
                inString = !inString;
            }
        }
        
        // If we're inside a string, this is not a simple assignment
        if (inString) {
            isValidAssignment = false;
        }
        
        if (isValidAssignment) {
            std::string varName = statement.substr(0, assignPos);
            std::string expr = statement.substr(assignPos + 1);
            
            // Trim whitespace
            size_t nameStart = varName.find_first_not_of(" \t");
            size_t nameEnd = varName.find_last_not_of(" \t");
            if (nameStart != std::string::npos && nameEnd != std::string::npos) {
                varName = varName.substr(nameStart, nameEnd - nameStart + 1);
            }
            
            size_t exprStart = expr.find_first_not_of(" \t");
            size_t exprEnd = expr.find_last_not_of(" \t");
            if (exprStart != std::string::npos && exprEnd != std::string::npos) {
                expr = expr.substr(exprStart, exprEnd - exprStart + 1);
            }
            
            auto varDeclNode = std::make_unique<VariableDeclNode>(varName, "auto");
            auto exprNode = parsePythonExpression(expr);
            if (exprNode) {
                varDeclNode->addChild(std::move(exprNode));
            }
            return std::move(varDeclNode);
        }
    }
    
    // Check for binary operations BEFORE function calls (to handle expressions like "text" + str(x))
    size_t plusPos = statement.find('+');
    if (plusPos != std::string::npos && plusPos > 0 && plusPos < statement.length() - 1) {
        // Make sure this is not inside a string literal
        bool inString = false;
        bool isValidBinaryOp = true;
        
        for (size_t i = 0; i < plusPos; i++) {
            char c = statement[i];
            if ((c == '"' || c == '\'') && (i == 0 || statement[i - 1] != '\\')) {
                inString = !inString;
            }
        }
        
        // If we're inside a string, this is not a binary operation
        if (inString) {
            isValidBinaryOp = false;
        }
        
        if (isValidBinaryOp) {
            std::string left = statement.substr(0, plusPos);
            std::string right = statement.substr(plusPos + 1);
            
            auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
            auto leftExpr = parsePythonExpression(left);
            auto rightExpr = parsePythonExpression(right);
            
            if (leftExpr) binOpNode->addChild(std::move(leftExpr));
            if (rightExpr) binOpNode->addChild(std::move(rightExpr));
            
            return std::move(binOpNode);
        }
    }
    
    // Check for function calls AFTER assignments
    size_t callPos = statement.find("print");
    if (callPos != std::string::npos) {
        size_t openParen = statement.find("(", callPos);
        if (openParen != std::string::npos) {
            // Find the matching closing parenthesis, respecting string literals
            size_t closeParen = openParen + 1;
            bool inString = false;
            int parenCount = 1;
            
            while (closeParen < statement.length() && parenCount > 0) {
                char c = statement[closeParen];
                
                if ((c == '"' || c == '\'') && (closeParen == 0 || statement[closeParen - 1] != '\\')) {
                    inString = !inString;
                }
                
                if (!inString) {
                    if (c == '(') parenCount++;
                    else if (c == ')') parenCount--;
                }
                closeParen++;
            }
            
            if (parenCount == 0) {
                closeParen--; // Go back to the actual closing parenthesis
                std::string argsStr = statement.substr(openParen + 1, closeParen - openParen - 1);
                auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, "print");
                parsePythonFunctionArguments(argsStr, *callNode);
                return std::move(callNode);
            }
        }
    }
    
    // Check for other function calls (like main())
    size_t otherCallPos = statement.find("(");
    if (otherCallPos != std::string::npos && otherCallPos > 0) {
        size_t closeParen = statement.find(")", otherCallPos);
        if (closeParen != std::string::npos) {
            std::string funcName = statement.substr(0, otherCallPos);
            std::string argsStr = statement.substr(otherCallPos + 1, closeParen - otherCallPos - 1);
            
            // Trim whitespace from function name
            size_t nameStart = funcName.find_first_not_of(" \t");
            size_t nameEnd = funcName.find_last_not_of(" \t");
            if (nameStart != std::string::npos && nameEnd != std::string::npos) {
                funcName = funcName.substr(nameStart, nameEnd - nameStart + 1);
            }
            
            auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, funcName);
            parsePythonFunctionArguments(argsStr, *callNode);
            return std::move(callNode);
        }
    }
    
    // Default: treat as expression
    return parsePythonExpression(statement);
}

std::unique_ptr<ASTNode> PythonFrontend::parsePythonExpression(const std::string& expr) {
    std::string trimmedExpr = expr;
    
    // Trim whitespace from expression
    size_t exprStart = trimmedExpr.find_first_not_of(" \t");
    size_t exprEnd = trimmedExpr.find_last_not_of(" \t");
    if (exprStart != std::string::npos && exprEnd != std::string::npos) {
        trimmedExpr = trimmedExpr.substr(exprStart, exprEnd - exprStart + 1);
    }
    
    // Handle string literals (including f-strings)
    if (trimmedExpr.length() >= 2 && ((trimmedExpr[0] == '"' && trimmedExpr[trimmedExpr.length() - 1] == '"') || 
                               (trimmedExpr[0] == '\'' && trimmedExpr[trimmedExpr.length() - 1] == '\''))) {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, trimmedExpr);
    }
    
    // Handle f-strings (enhanced implementation)
    if (trimmedExpr.length() >= 3 && ((trimmedExpr[0] == 'f' && trimmedExpr[1] == '"' && trimmedExpr[trimmedExpr.length() - 1] == '"') || 
                               (trimmedExpr[0] == 'f' && trimmedExpr[1] == '\'' && trimmedExpr[trimmedExpr.length() - 1] == '\''))) {
        return parseFString(trimmedExpr);
    }
    
    // Handle function calls in expressions (like add(x, y) and str(x))
    size_t parenPos = trimmedExpr.find("(");
    if (parenPos != std::string::npos && parenPos > 0) {
        size_t closeParen = findMatchingParenthesis(trimmedExpr, parenPos);
        if (closeParen != std::string::npos) {
            std::string funcName = trimmedExpr.substr(0, parenPos);
            std::string argsStr = trimmedExpr.substr(parenPos + 1, closeParen - parenPos - 1);
            
            // Trim whitespace from function name
            size_t nameStart = funcName.find_first_not_of(" \t");
            size_t nameEnd = funcName.find_last_not_of(" \t");
            if (nameStart != std::string::npos && nameEnd != std::string::npos) {
                funcName = funcName.substr(nameStart, nameEnd - nameStart + 1);
            }
            
            // Handle built-in string methods
            if (funcName == "str" || funcName == "int" || funcName == "float" || funcName == "len") {
                auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, funcName);
                parsePythonFunctionArguments(argsStr, *callNode);
                return std::move(callNode);
            }
            
            // Handle regular function calls
            auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, funcName);
            parsePythonFunctionArguments(argsStr, *callNode);
            return std::move(callNode);
        }
    }
    
    // Handle binary operations (check for +, -, *, /)
    size_t plusPos = trimmedExpr.find('+');
    if (plusPos != std::string::npos && plusPos > 0 && plusPos < trimmedExpr.length() - 1) {
        std::string left = trimmedExpr.substr(0, plusPos);
        std::string right = trimmedExpr.substr(plusPos + 1);
        
        auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
        auto leftExpr = parsePythonExpression(left);
        auto rightExpr = parsePythonExpression(right);
        
        if (leftExpr) binOpNode->addChild(std::move(leftExpr));
        if (rightExpr) binOpNode->addChild(std::move(rightExpr));
        
        return std::move(binOpNode);
    }
    
    size_t mulPos = trimmedExpr.find('*');
    if (mulPos != std::string::npos && mulPos > 0 && mulPos < trimmedExpr.length() - 1) {
        std::string left = trimmedExpr.substr(0, mulPos);
        std::string right = trimmedExpr.substr(mulPos + 1);
        
        auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "*");
        auto leftExpr = parsePythonExpression(left);
        auto rightExpr = parsePythonExpression(right);
        
        if (leftExpr) binOpNode->addChild(std::move(leftExpr));
        if (rightExpr) binOpNode->addChild(std::move(rightExpr));
        
        return std::move(binOpNode);
    }
    
    // Handle literals (numbers)
    if (isdigit(trimmedExpr[0]) || (trimmedExpr[0] == '-' && trimmedExpr.length() > 1 && isdigit(trimmedExpr[1]))) {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, trimmedExpr);
    }
    
    // Handle identifiers (variable names)
    if (!trimmedExpr.empty() && (isalpha(trimmedExpr[0]) || trimmedExpr[0] == '_')) {
        return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, trimmedExpr);
    }
    
    // Default: treat as literal if not empty
    if (!trimmedExpr.empty()) {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, trimmedExpr);
    }
    
    return nullptr;
}

void PythonFrontend::parsePythonFunctionArguments(const std::string& argsStr, ASTNode& callNode) {
    if (argsStr.empty()) return;
    
    size_t pos = 0;
    while (pos < argsStr.length()) {
        // Skip whitespace and commas
        while (pos < argsStr.length() && (isspace(argsStr[pos]) || argsStr[pos] == ',')) {
            pos++;
        }
        if (pos >= argsStr.length()) break;
        
        // Find the next comma or end of string, but respect string literals and nested parentheses
        size_t argEnd = pos;
        bool inString = false;
        char stringChar = '\0';
        int parenCount = 0;
        int bracketCount = 0;
        int braceCount = 0;
        
        while (argEnd < argsStr.length()) {
            char c = argsStr[argEnd];
            
            // Handle string literals
            if (!inString && (c == '"' || c == '\'')) {
                inString = true;
                stringChar = c;
            } else if (inString && c == stringChar) {
                // Check for escaped quotes
                if (argEnd == 0 || argsStr[argEnd - 1] != '\\') {
                    inString = false;
                    stringChar = '\0';
                }
            }
            
            // Handle nested structures when not in string
            if (!inString) {
                if (c == '(') parenCount++;
                else if (c == ')') parenCount--;
                else if (c == '[') bracketCount++;
                else if (c == ']') bracketCount--;
                else if (c == '{') braceCount++;
                else if (c == '}') braceCount--;
                
                // Break on comma only if we're not in nested structures
                if (c == ',' && parenCount == 0 && bracketCount == 0 && braceCount == 0) {
                    argEnd--;
                    break;
                }
            }
            
            argEnd++;
        }
        
        std::string arg = argsStr.substr(pos, argEnd - pos + 1);
        
        // Trim whitespace
        size_t start = arg.find_first_not_of(" \t");
        size_t end = arg.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            arg = arg.substr(start, end - start + 1);
        }
        
        if (!arg.empty()) {
            auto argExpr = parsePythonExpression(arg);
            if (argExpr) {
                callNode.addChild(std::move(argExpr));
            }
        }
        
        pos = argEnd + 1;
    }
}

std::string PythonFrontend::preprocessPythonCode(const std::string& sourceCode) {
    std::string result;
    size_t pos = 0;
    
    while (pos < sourceCode.length()) {
        // Skip single-line comments
        if (pos + 1 < sourceCode.length() && sourceCode[pos] == '#' && sourceCode[pos + 1] != '#') {
            while (pos < sourceCode.length() && sourceCode[pos] != '\n') {
                pos++;
            }
            continue;
        }
        
        // Skip multi-line comments (docstrings)
        if (pos + 2 < sourceCode.length() && sourceCode[pos] == '"' && sourceCode[pos + 1] == '"' && sourceCode[pos + 2] == '"') {
            pos += 3;
            while (pos + 2 < sourceCode.length() && !(sourceCode[pos] == '"' && sourceCode[pos + 1] == '"' && sourceCode[pos + 2] == '"')) {
                pos++;
            }
            pos += 3; // Skip """
            continue;
        }
        
        if (pos + 2 < sourceCode.length() && sourceCode[pos] == '\'' && sourceCode[pos + 1] == '\'' && sourceCode[pos + 2] == '\'') {
            pos += 3;
            while (pos + 2 < sourceCode.length() && !(sourceCode[pos] == '\'' && sourceCode[pos + 1] == '\'' && sourceCode[pos + 2] == '\'')) {
                pos++;
            }
            pos += 3; // Skip '''
            continue;
        }
        
        result += sourceCode[pos];
        pos++;
    }
    
    return result;
}

bool PythonFrontend::isInsidePythonFunction(const std::string& code, size_t position) {
    // Check if the position is inside a function by looking at indentation
    size_t lineStart = position;
    while (lineStart > 0 && code[lineStart - 1] != '\n') {
        lineStart--;
    }
    
    // Count leading spaces
    int indent = 0;
    while (lineStart < position && code[lineStart] == ' ') {
        indent++;
        lineStart++;
    }
    
    // If indented, we're likely inside a function
    return indent > 0;
}

std::unique_ptr<ASTNode> PythonFrontend::parseFString(const std::string& fstring) {
    // Extract the content inside f"..." or f'...'
    if (fstring.length() < 3) {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, fstring);
    }
    
    char quoteChar = fstring[1]; // Either " or '
    std::string content = fstring.substr(2, fstring.length() - 3);
    
    // For now, create a simple string concatenation node
    // This is a simplified implementation - a full implementation would parse {expressions}
    auto concatNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
    
    // Parse the f-string content for {variable} patterns
    size_t pos = 0;
    std::string result;
    bool hasVariables = false;
    
    while (pos < content.length()) {
        if (content[pos] == '{' && pos + 1 < content.length()) {
            // Found a variable reference
            size_t closeBrace = content.find('}', pos);
            if (closeBrace != std::string::npos) {
                // Add the text before the variable
                if (pos > 0) {
                    std::string textPart = content.substr(0, pos);
                    if (!textPart.empty()) {
                        auto textNode = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "\"" + textPart + "\"");
                        concatNode->addChild(std::move(textNode));
                    }
                }
                
                // Parse the variable/expression inside braces
                std::string varExpr = content.substr(pos + 1, closeBrace - pos - 1);
                auto varNode = parsePythonExpression(varExpr);
                if (varNode) {
                    concatNode->addChild(std::move(varNode));
                    hasVariables = true;
                }
                
                // Move past the closing brace
                pos = closeBrace + 1;
                content = content.substr(pos);
                pos = 0;
            } else {
                break; // No closing brace found
            }
        } else {
            pos++;
        }
    }
    
    // Add remaining text
    if (!content.empty()) {
        auto textNode = std::make_unique<ASTNode>(ASTNodeType::LITERAL, "\"" + content + "\"");
        if (hasVariables) {
            concatNode->addChild(std::move(textNode));
        } else {
            // No variables found, return as simple string literal
            return std::make_unique<ASTNode>(ASTNodeType::LITERAL, "\"" + content + "\"");
        }
    }
    
    // If we have variables, return the concatenation node
    if (hasVariables && concatNode->getChildren().size() > 1) {
        return std::move(concatNode);
    } else if (concatNode->getChildren().size() == 1) {
        // Only one child, return it directly
        return std::move(const_cast<ASTNode&>(*concatNode->getChildren()[0]).clone());
    } else {
        // Fallback to literal
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, fstring);
    }
}

size_t PythonFrontend::findMatchingParenthesis(const std::string& expr, size_t openPos) {
    if (openPos >= expr.length() || expr[openPos] != '(') {
        return std::string::npos;
    }
    
    int parenCount = 1;
    bool inString = false;
    char stringChar = '\0';
    
    for (size_t i = openPos + 1; i < expr.length(); i++) {
        char c = expr[i];
        
        // Handle string literals
        if (!inString && (c == '"' || c == '\'')) {
            inString = true;
            stringChar = c;
        } else if (inString && c == stringChar) {
            // Check for escaped quotes
            if (i == 0 || expr[i - 1] != '\\') {
                inString = false;
                stringChar = '\0';
            }
        }
        
        // Count parentheses only when not in string
        if (!inString) {
            if (c == '(') {
                parenCount++;
            } else if (c == ')') {
                parenCount--;
                if (parenCount == 0) {
                    return i;
                }
            }
        }
    }
    
    return std::string::npos; // No matching parenthesis found
}
