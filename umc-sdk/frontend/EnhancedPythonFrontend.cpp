#include "EnhancedPythonFrontend.h"
#include "ASTNode.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>

EnhancedPythonFrontend::EnhancedPythonFrontend() 
#ifdef TREE_SITTER_AVAILABLE
    : parser_(nullptr), python_language_(nullptr), tree_(nullptr) 
#endif
{
#ifdef TREE_SITTER_AVAILABLE
    initializeTreeSitter();
#endif
    std::cout << "Enhanced Python Frontend initialized" << std::endl;
}

EnhancedPythonFrontend::~EnhancedPythonFrontend() {
#ifdef TREE_SITTER_AVAILABLE
    cleanupTreeSitter();
#endif
    ast_.reset();
}

#ifdef TREE_SITTER_AVAILABLE
bool EnhancedPythonFrontend::initializeTreeSitter() {
    // Initialize Tree-sitter
    parser_ = ts_parser_new();
    if (!parser_) {
        std::cerr << "Failed to create Tree-sitter parser" << std::endl;
        return false;
    }
    
    // For now, we'll use fallback since we don't have Python grammar loaded
    // In a real implementation, we would load the Python grammar
    std::cout << "Tree-sitter Python grammar not available, using fallback" << std::endl;
    return false;
}

void EnhancedPythonFrontend::cleanupTreeSitter() {
    if (tree_) {
        ts_tree_delete(tree_);
        tree_ = nullptr;
    }
    
    if (parser_) {
        ts_parser_delete(parser_);
        parser_ = nullptr;
    }
}
#endif

bool EnhancedPythonFrontend::parse(const std::string& sourceCode) {
    if (sourceCode.empty()) {
        std::cerr << "Empty source code" << std::endl;
        return false;
    }
    
    // Clean up previous AST
    ast_.reset();
    
#ifdef TREE_SITTER_AVAILABLE
    if (parser_ && python_language_) {
        std::cout << "Using Tree-sitter for Python parsing" << std::endl;
        ast_ = parseWithTreeSitter(sourceCode);
    }
#endif
    
    if (!ast_) {
        std::cout << "Using enhanced fallback parser for Python" << std::endl;
        parseWithEnhancedFallback(sourceCode);
    }
    
    return ast_ != nullptr;
}

#ifdef TREE_SITTER_AVAILABLE
std::unique_ptr<ASTNode> EnhancedPythonFrontend::parseWithTreeSitter(const std::string& sourceCode) {
    // Clean up previous tree
    if (tree_) {
        ts_tree_delete(tree_);
        tree_ = nullptr;
    }
    
    // Parse the source code
    tree_ = ts_parser_parse_string(parser_, nullptr, sourceCode.c_str(), sourceCode.length());
    if (!tree_) {
        std::cerr << "Tree-sitter parsing failed" << std::endl;
        return nullptr;
    }
    
    // Convert Tree-sitter tree to our AST
    TSNode rootNode = ts_tree_root_node(tree_);
    return convertTreeSitterNode(rootNode, sourceCode);
}

std::unique_ptr<ASTNode> EnhancedPythonFrontend::convertTreeSitterNode(TSNode node, const std::string& sourceCode) {
    if (ts_node_is_null(node)) {
        return nullptr;
    }
    
    const char* type = ts_node_type(node);
    std::string nodeTypeStr = type ? type : "unknown";
    ASTNodeType nodeType = mapTreeSitterNodeType(nodeTypeStr);
    
    // Extract text from source code
    uint32_t startByte = ts_node_start_byte(node);
    uint32_t endByte = ts_node_end_byte(node);
    std::string nodeText = "";
    if (startByte < sourceCode.length() && endByte <= sourceCode.length()) {
        nodeText = sourceCode.substr(startByte, endByte - startByte);
    }
    
    // Create AST node
    auto astNode = std::make_unique<ASTNode>(nodeType, nodeText);
    
    // Process children
    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        auto childAST = convertTreeSitterNode(child, sourceCode);
        if (childAST) {
            astNode->addChild(std::move(childAST));
        }
    }
    
    return astNode;
}

ASTNodeType EnhancedPythonFrontend::mapTreeSitterNodeType(const std::string& type) {
    if (type == "module") return ASTNodeType::PROGRAM;
    if (type == "function_definition") return ASTNodeType::FUNCTION_DECL;
    if (type == "class_definition") return ASTNodeType::FUNCTION_DECL;
    if (type == "import_statement" || type == "import_from_statement") return ASTNodeType::FUNCTION_DECL;
    if (type == "call") return ASTNodeType::FUNCTION_CALL;
    if (type == "return_statement") return ASTNodeType::RETURN_STMT;
    if (type == "assignment") return ASTNodeType::VARIABLE_DECL;
    if (type == "binary_operator") return ASTNodeType::BINARY_OP;
    if (type == "string" || type == "integer" || type == "float" || type == "identifier") return ASTNodeType::LITERAL;
    
    return ASTNodeType::PROGRAM;
}
#endif

void EnhancedPythonFrontend::parseWithEnhancedFallback(const std::string& source) {
    // Create root program node
    ast_ = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "PythonProgram");
    
    // Extract imports
    extractImports(source);
    
    // Extract classes
    extractClasses(source);
    
    // Extract functions
    extractFunctions(source);
    
    // Extract control flow statements
    extractControlFlow(source);
    
    std::cout << "Enhanced Python fallback parsing completed" << std::endl;
}

void EnhancedPythonFrontend::extractImports(const std::string& source) {
    std::regex importRegex(R"(import\s+([\w\.]+)|from\s+([\w\.]+)\s+import\s+(.+))");
    std::sregex_iterator iter(source.begin(), source.end(), importRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        std::string importName;
        
        if (!match[1].str().empty()) {
            importName = match[1].str();
        } else if (!match[2].str().empty()) {
            importName = match[2].str() + "." + match[3].str();
        }
        
        auto importNode = createImportNode(importName);
        ast_->addChild(std::move(importNode));
    }
}

void EnhancedPythonFrontend::extractClasses(const std::string& source) {
    std::regex classRegex(R"(class\s+(\w+)\s*(?:\(([^)]+)\))?\s*:([\s\S]*?)(?=\n\s*[a-zA-Z_#]|\Z))");
    std::sregex_iterator iter(source.begin(), source.end(), classRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        std::string className = match[1].str();
        std::string classBody = match[3].str();
        
        auto classNode = createClassNode(className);
        
        // Extract methods from class body
        std::string cleanBody = removeIndentation(classBody);
        extractFunctions(cleanBody, classNode.get());
        
        ast_->addChild(std::move(classNode));
    }
}

void EnhancedPythonFrontend::extractFunctions(const std::string& source) {
    extractFunctions(source, ast_.get());
}

void EnhancedPythonFrontend::extractFunctions(const std::string& source, ASTNode* parent) {
    std::regex functionRegex(R"(def\s+(\w+)\s*\(([^)]*)\)\s*:([\s\S]*?)(?=\n\s*[a-zA-Z_#]|\Z))");
    std::sregex_iterator iter(source.begin(), source.end(), functionRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        std::string functionName = match[1].str();
        std::string paramsStr = match[2].str();
        std::string functionBody = match[3].str();
        
        std::vector<std::string> params = parsePythonList(paramsStr);
        auto functionNode = createFunctionNode(functionName, params);
        
        // Extract function body elements
        std::string cleanBody = removeIndentation(functionBody);
        
        // Look for print statements
        std::regex printRegex(R"(print\s*\(([^)]+)\))");
        std::sregex_iterator printIter(cleanBody.begin(), cleanBody.end(), printRegex);
        std::sregex_iterator printEnd;
        
        for (; printIter != printEnd; ++printIter) {
            std::smatch printMatch = *printIter;
            std::string printArg = printMatch[1].str();
            auto printCall = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, "print(" + printArg + ")");
            functionNode->addChild(std::move(printCall));
        }
        
        // Look for return statements
        std::regex returnRegex(R"(return\s+(.+))");
        std::sregex_iterator returnIter(cleanBody.begin(), cleanBody.end(), returnRegex);
        std::sregex_iterator returnEnd;
        
        for (; returnIter != returnEnd; ++returnIter) {
            std::smatch returnMatch = *returnIter;
            std::string returnValue = returnMatch[1].str();
            auto returnStmt = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT, "return " + returnValue);
            functionNode->addChild(std::move(returnStmt));
        }
        
        parent->addChild(std::move(functionNode));
    }
}

void EnhancedPythonFrontend::extractControlFlow(const std::string& source) {
    // Extract if statements
    std::regex ifRegex(R"(\bif\s+(.+):)");
    std::sregex_iterator ifIter(source.begin(), source.end(), ifRegex);
    std::sregex_iterator ifEnd;
    
    for (; ifIter != ifEnd; ++ifIter) {
        std::smatch match = *ifIter;
        std::string condition = match[1].str();
        auto ifNode = createControlFlowNode("if", condition);
        ast_->addChild(std::move(ifNode));
    }
    
    // Extract for loops
    std::regex forRegex(R"(\bfor\s+(\w+)\s+in\s+(.+):)");
    std::sregex_iterator forIter(source.begin(), source.end(), forRegex);
    std::sregex_iterator forEnd;
    
    for (; forIter != forEnd; ++forIter) {
        std::smatch match = *forIter;
        std::string variable = match[1].str();
        std::string iterable = match[2].str();
        auto forNode = createControlFlowNode("for", variable + " in " + iterable);
        ast_->addChild(std::move(forNode));
    }
    
    // Extract while loops
    std::regex whileRegex(R"(\bwhile\s+(.+):)");
    std::sregex_iterator whileIter(source.begin(), source.end(), whileRegex);
    std::sregex_iterator whileEnd;
    
    for (; whileIter != whileEnd; ++whileIter) {
        std::smatch match = *whileIter;
        std::string condition = match[1].str();
        auto whileNode = createControlFlowNode("while", condition);
        ast_->addChild(std::move(whileNode));
    }
}

std::unique_ptr<ASTNode> EnhancedPythonFrontend::createFunctionNode(const std::string& functionName, 
                                                                    const std::vector<std::string>& params) {
    std::string signature = functionName + "(";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) signature += ", ";
        signature += params[i];
    }
    signature += ")";
    
    return std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, signature);
}

std::unique_ptr<ASTNode> EnhancedPythonFrontend::createClassNode(const std::string& className) {
    return std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, className);
}

std::unique_ptr<ASTNode> EnhancedPythonFrontend::createImportNode(const std::string& importName) {
    return std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, importName);
}

std::unique_ptr<ASTNode> EnhancedPythonFrontend::createControlFlowNode(const std::string& type, const std::string& condition) {
    return std::make_unique<ASTNode>(ASTNodeType::IF_STMT, type + ": " + condition);
}

std::vector<std::string> EnhancedPythonFrontend::parsePythonList(const std::string& listStr) {
    std::vector<std::string> result;
    std::string clean = trim(listStr);
    
    if (clean.empty()) return result;
    
    // Simple parsing - split by commas and trim
    std::stringstream ss(clean);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        result.push_back(trim(item));
    }
    
    return result;
}

std::string EnhancedPythonFrontend::trim(const std::string& str) {
    const std::string whitespace = " \t\n\r";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

std::string EnhancedPythonFrontend::removeIndentation(const std::string& code) {
    std::vector<std::string> lines;
    std::stringstream ss(code);
    std::string line;
    
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    
    // Find minimum indentation (excluding empty lines)
    size_t minIndent = SIZE_MAX;
    for (const auto& line : lines) {
        if (!trim(line).empty()) {
            size_t indent = line.find_first_not_of(" \t");
            if (indent != std::string::npos) {
                minIndent = std::min(minIndent, indent);
            }
        }
    }
    
    // Remove minimum indentation from all lines
    std::string result;
    for (const auto& line : lines) {
        if (!trim(line).empty() && line.length() > minIndent) {
            result += line.substr(minIndent) + "\n";
        } else {
            result += line + "\n";
        }
    }
    
    return result;
}

bool EnhancedPythonFrontend::isPythonComment(const std::string& line) {
    std::string trimmed = trim(line);
    return !trimmed.empty() && trimmed[0] == '#';
}

std::string EnhancedPythonFrontend::extractExpression(const std::string& line) {
    // Remove leading/trailing whitespace and common delimiters
    std::string result = trim(line);
    
    // Remove trailing commas, semicolons
    if (!result.empty() && (result.back() == ',' || result.back() == ';')) {
        result.pop_back();
    }
    
    return trim(result);
}

std::unique_ptr<ASTNode> EnhancedPythonFrontend::getAST() const {
    return ast_ ? ast_->clone() : nullptr;
}
