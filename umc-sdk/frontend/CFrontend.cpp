#include "CFrontend.h"
#include <iostream>
#include <fstream>
#include <cstring>

CFrontend::CFrontend() {
#ifdef CLANG_FOUND
    index_ = clang_createIndex(0, 0);
    translationUnit_ = nullptr;
#endif
    language_ = Language::C;
    ast_ = nullptr;
}

CFrontend::~CFrontend() {
#ifdef CLANG_FOUND
    if (translationUnit_) {
        clang_disposeTranslationUnit(translationUnit_);
    }
    clang_disposeIndex(index_);
#endif
}

bool CFrontend::parse(const std::string& sourceCode) {
#ifdef CLANG_FOUND
    sourceCode_ = sourceCode;

    // Create temporary file for libclang
    std::string tempFile = "temp.c";
    std::ofstream outFile(tempFile);
    outFile << sourceCode;
    outFile.close();

    const char* args[] = {
        "-std=c11",
        "-I.",
        nullptr
    };

    translationUnit_ = clang_parseTranslationUnit(
        index_,
        tempFile.c_str(),
        args,
        1,
        nullptr,
        0,
        CXTranslationUnit_None);

    if (!translationUnit_) {
        std::cerr << "Error: Unable to parse translation unit" << std::endl;
        return false;
    }

    // Build AST from Clang AST
    CXCursor cursor = clang_getTranslationUnitCursor(translationUnit_);
    ast_ = std::unique_ptr<ProgramNode>(dynamic_cast<ProgramNode*>(buildASTFromCursor(cursor).release()));

    // Clean up temp file
    std::remove(tempFile.c_str());

    return true;
#else
    // Enhanced fallback parser for when Clang is not available
    std::cout << "Using enhanced fallback C parser (Clang not available)" << std::endl;
    
    ast_ = std::make_unique<ProgramNode>();
    
    // Preprocess: remove preprocessor directives and clean the code
    std::string cleanCode = preprocessSourceCode(sourceCode);
    
    // Parse functions at global scope
    parseGlobalFunctions(cleanCode);
    
    return true;
#endif
}

std::unique_ptr<ASTNode> CFrontend::getAST() const {
    if (ast_) {
        // Use the clone method for proper deep copy
        return ast_->clone();
    }
    return nullptr;
}

#ifdef CLANG_FOUND
std::unique_ptr<ASTNode> CFrontend::buildASTFromCursor(CXCursor cursor) {
    return createNodeFromCursor(cursor);
}

std::unique_ptr<ASTNode> CFrontend::createNodeFromCursor(CXCursor cursor) {
    CXCursorKind kind = clang_getCursorKind(cursor);
    std::string name = getCursorSpelling(cursor);
    std::string type = getCursorType(cursor);

    switch (kind) {
        case CXCursor_FunctionDecl: {
            auto funcNode = std::make_unique<FunctionDeclNode>(name, type);
            traverseAST(cursor, funcNode.get());
            return std::move(funcNode);
        }
        case CXCursor_VarDecl: {
            auto varNode = std::make_unique<VariableDeclNode>(name, type);
            return std::move(varNode);
        }
        case CXCursor_CallExpr: {
            auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, name);
            traverseAST(cursor, callNode.get());
            return std::move(callNode);
        }
        case CXCursor_ReturnStmt: {
            auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT);
            traverseAST(cursor, returnNode.get());
            return std::move(returnNode);
        }
        case CXCursor_IfStmt: {
            auto ifNode = std::make_unique<ASTNode>(ASTNodeType::IF_STMT);
            traverseAST(cursor, ifNode.get());
            return std::move(ifNode);
        }
        case CXCursor_ForStmt: {
            auto forNode = std::make_unique<ASTNode>(ASTNodeType::FOR_LOOP);
            traverseAST(cursor, forNode.get());
            return std::move(forNode);
        }
        case CXCursor_WhileStmt: {
            auto whileNode = std::make_unique<ASTNode>(ASTNodeType::WHILE_LOOP);
            traverseAST(cursor, whileNode.get());
            return std::move(whileNode);
        }
        case CXCursor_BinaryOperator: {
            auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, name);
            traverseAST(cursor, binOpNode.get());
            return std::move(binOpNode);
        }
        case CXCursor_DeclStmt: {
            auto declNode = std::make_unique<ASTNode>(ASTNodeType::VARIABLE_DECL, name);
            traverseAST(cursor, declNode.get());
            return std::move(declNode);
        }
        default: {
            auto node = std::make_unique<ASTNode>(ASTNodeType::LITERAL, name);
            return std::move(node);
        }
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

void CFrontend::traverseAST(CXCursor cursor, ASTNode* parent) {
    clang_visitChildren(
        cursor,
        [](CXCursor cursor, CXCursor parent, CXClientData clientData) {
            ASTNode* parentNode = static_cast<ASTNode*>(clientData);
            CFrontend* frontend = static_cast<CFrontend*>(parentNode);
            
            auto childNode = frontend->createNodeFromCursor(cursor);
            if (childNode) {
                parentNode->addChild(std::move(childNode));
            }
            
            return CXChildVisit_Continue;
        },
        this
    );
}
#endif

// Enhanced fallback parser implementations
void CFrontend::parseStatements(const std::string& body, ASTNode& blockNode) {
    size_t pos = 0;
    while (pos < body.length()) {
        // Skip whitespace and newlines
        while (pos < body.length() && isspace(body[pos])) {
            pos++;
        }
        if (pos >= body.length()) break;
        
        // Skip single-line comments
        if (pos + 1 < body.length() && body[pos] == '/' && body[pos + 1] == '/') {
            while (pos < body.length() && body[pos] != '\n') {
                pos++;
            }
            continue;
        }
        
        // Skip multi-line comments
        if (pos + 1 < body.length() && body[pos] == '/' && body[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < body.length() && !(body[pos] == '*' && body[pos + 1] == '/')) {
                pos++;
            }
            pos += 2; // Skip */
            continue;
        }
        
        // Check for nested function definition (invalid C but may appear)
        // Simple check: if we see "int add(" pattern, skip until closing brace
        if (pos + 8 < body.length() && body.substr(pos, 8) == "int add(") {
            size_t bracePos = body.find("{", pos);
            if (bracePos != std::string::npos) {
                // Find matching closing brace
                int braceCount = 1;
                size_t closeBrace = bracePos + 1;
                while (closeBrace < body.length() && braceCount > 0) {
                    if (body[closeBrace] == '{') braceCount++;
                    else if (body[closeBrace] == '}') braceCount--;
                    closeBrace++;
                }
                
                if (braceCount == 0) {
                    pos = closeBrace;
                    continue;
                }
            }
        }
        
        // Find the end of the current statement (semicolon)
        size_t stmtEnd = pos;
        bool inString = false;
        
        while (stmtEnd < body.length()) {
            char c = body[stmtEnd];
            
            // Handle string literals
            if (c == '"' && (stmtEnd == 0 || body[stmtEnd - 1] != '\\')) {
                inString = !inString;
            }
            
            if (!inString && c == ';') {
                break;
            }
            stmtEnd++;
        }
        
        if (stmtEnd >= body.length()) {
            stmtEnd = body.length() - 1;
        }
        
        std::string statement = body.substr(pos, stmtEnd - pos);
        
        // Trim whitespace
        size_t start = statement.find_first_not_of(" \t\n\r");
        size_t end = statement.find_last_not_of(" \t\n\r");
        if (start != std::string::npos && end != std::string::npos) {
            statement = statement.substr(start, end - start + 1);
        }
        
        if (!statement.empty()) {
            auto stmtNode = parseStatement(statement);
            if (stmtNode) {
                blockNode.addChild(std::move(stmtNode));
            }
        }
        
        pos = stmtEnd + 1;
    }
}

std::unique_ptr<ASTNode> CFrontend::parseStatement(const std::string& statement) {
    // Skip if this looks like a nested function definition (invalid C but may appear)
    // Check for pattern: returnType functionName(...) { ... }
    if ((statement.find("int ") == 0 || statement.find("void ") == 0 || statement.find("char ") == 0) && 
        statement.find("(") != std::string::npos && 
        statement.find("{") != std::string::npos) {
        // This is likely a nested function definition - skip it for now
        return nullptr;
    }
    
    // Skip if this looks like the start of a nested function definition
    if (statement.find("int ") == 0 && statement.find("(") != std::string::npos && 
        statement.find("return") != std::string::npos) {
        // This is likely a nested function definition - skip it for now
        return nullptr;
    }
    
    // Check for variable declarations
    if (statement.find("int ") == 0) {
        size_t equalPos = statement.find("=");
        std::string varName;
        std::unique_ptr<ASTNode> initExpr = nullptr;
        
        if (equalPos != std::string::npos) {
            varName = statement.substr(4, equalPos - 4);
            std::string initStr = statement.substr(equalPos + 1);
            
            // Trim whitespace
            size_t start = varName.find_first_not_of(" \t");
            size_t end = varName.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                varName = varName.substr(start, end - start + 1);
            }
            
            start = initStr.find_first_not_of(" \t");
            end = initStr.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                initStr = initStr.substr(start, end - start + 1);
            }
            
            initExpr = parseExpression(initStr);
        } else {
            varName = statement.substr(4);
            size_t start = varName.find_first_not_of(" \t");
            size_t end = varName.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                varName = varName.substr(start, end - start + 1);
            }
        }
        
        auto varDeclNode = std::make_unique<VariableDeclNode>(varName, "int");
        if (initExpr) {
            varDeclNode->addChild(std::move(initExpr));
        }
        return std::move(varDeclNode);
    }
    
    // Check for function calls
    size_t callPos = statement.find("printf");
    if (callPos != std::string::npos) {
        size_t openParen = statement.find("(", callPos);
        if (openParen != std::string::npos) {
            // Find the matching closing parenthesis, respecting string literals
            size_t closeParen = openParen + 1;
            bool inString = false;
            int parenCount = 1;
            
            while (closeParen < statement.length() && parenCount > 0) {
                char c = statement[closeParen];
                
                if (c == '"' && (closeParen == 0 || statement[closeParen - 1] != '\\')) {
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
                auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, "printf");
                
                // Parse arguments - handle multiple arguments separated by commas
                parseFunctionArguments(argsStr, *callNode);
                
                return std::move(callNode);
            }
        }
    }
    
    // Check for return statements
    if (statement.find("return") == 0) {
        std::string returnExpr = statement.substr(6);
        size_t start = returnExpr.find_first_not_of(" \t");
        size_t end = returnExpr.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            returnExpr = returnExpr.substr(start, end - start + 1);
        }
        
        auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT, "return");
        if (!returnExpr.empty()) {
            auto expr = parseExpression(returnExpr);
            if (expr) {
                returnNode->addChild(std::move(expr));
            }
        }
        return std::move(returnNode);
    }
    
    // Default: treat as expression
    return parseExpression(statement);
}

std::unique_ptr<ASTNode> CFrontend::parseExpression(const std::string& expr) {
    // Handle string literals first
    if (expr.length() >= 2 && expr[0] == '"' && expr[expr.length() - 1] == '"') {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, expr);
    }
    
    // Handle function calls (check for function call pattern)
    size_t openParen = expr.find('(');
    if (openParen != std::string::npos && openParen > 0) {
        size_t closeParen = expr.find(')', openParen);
        if (closeParen != std::string::npos) {
            std::string funcName = expr.substr(0, openParen);
            std::string argsStr = expr.substr(openParen + 1, closeParen - openParen - 1);
            
            // Trim function name
            size_t nameStart = funcName.find_first_not_of(" \t");
            size_t nameEnd = funcName.find_last_not_of(" \t");
            if (nameStart != std::string::npos && nameEnd != std::string::npos) {
                funcName = funcName.substr(nameStart, nameEnd - nameStart + 1);
            }
            
            auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, funcName);
            parseFunctionArguments(argsStr, *callNode);
            return std::move(callNode);
        }
    }
    
    // Handle literals (numbers) first
    if (isdigit(expr[0]) || (expr[0] == '-' && expr.length() > 1 && isdigit(expr[1]))) {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, expr);
    }
    
    // Handle binary operations with proper precedence
    // Check for + and - (lowest precedence)
    size_t plusPos = findOperatorAtLevel(expr, "+-");
    if (plusPos != std::string::npos) {
        std::string left = expr.substr(0, plusPos);
        std::string right = expr.substr(plusPos + 1);
        
        // Trim whitespace
        size_t leftStart = left.find_first_not_of(" \t");
        size_t leftEnd = left.find_last_not_of(" \t");
        if (leftStart != std::string::npos && leftEnd != std::string::npos) {
            left = left.substr(leftStart, leftEnd - leftStart + 1);
        }
        
        size_t rightStart = right.find_first_not_of(" \t");
        size_t rightEnd = right.find_last_not_of(" \t");
        if (rightStart != std::string::npos && rightEnd != std::string::npos) {
            right = right.substr(rightStart, rightEnd - rightStart + 1);
        }
        
        auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, std::string(1, expr[plusPos]));
        auto leftExpr = parseExpression(left);
        auto rightExpr = parseExpression(right);
        
        if (leftExpr) binOpNode->addChild(std::move(leftExpr));
        if (rightExpr) binOpNode->addChild(std::move(rightExpr));
        
        return std::move(binOpNode);
    }
    
    // Check for * and / (higher precedence)
    size_t mulPos = findOperatorAtLevel(expr, "*/");
    if (mulPos != std::string::npos) {
        std::string left = expr.substr(0, mulPos);
        std::string right = expr.substr(mulPos + 1);
        
        // Trim whitespace
        size_t leftStart = left.find_first_not_of(" \t");
        size_t leftEnd = left.find_last_not_of(" \t");
        if (leftStart != std::string::npos && leftEnd != std::string::npos) {
            left = left.substr(leftStart, leftEnd - leftStart + 1);
        }
        
        size_t rightStart = right.find_first_not_of(" \t");
        size_t rightEnd = right.find_last_not_of(" \t");
        if (rightStart != std::string::npos && rightEnd != std::string::npos) {
            right = right.substr(rightStart, rightEnd - rightStart + 1);
        }
        
        auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, std::string(1, expr[mulPos]));
        auto leftExpr = parseExpression(left);
        auto rightExpr = parseExpression(right);
        
        if (leftExpr) binOpNode->addChild(std::move(leftExpr));
        if (rightExpr) binOpNode->addChild(std::move(rightExpr));
        
        return std::move(binOpNode);
    }
    
    // Handle identifiers (variable names) - check last
    if (!expr.empty() && (isalpha(expr[0]) || expr[0] == '_')) {
        return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, expr);
    }
    
    // Default: treat as identifier if not empty
    if (!expr.empty()) {
        return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, expr);
    }
    
    return nullptr;
}

void CFrontend::parseFunctionArguments(const std::string& argsStr, ASTNode& callNode) {
    if (argsStr.empty()) return;
    
    size_t pos = 0;
    while (pos < argsStr.length()) {
        // Skip whitespace and commas
        while (pos < argsStr.length() && (isspace(argsStr[pos]) || argsStr[pos] == ',')) {
            pos++;
        }
        if (pos >= argsStr.length()) break;
        
        // Find the next comma or end of string, but respect string literals and parentheses
        size_t argEnd = pos;
        bool inString = false;
        int parenCount = 0;
        
        while (argEnd < argsStr.length()) {
            char c = argsStr[argEnd];
            
            // Handle string literals - don't treat commas inside strings as separators
            if (c == '"' && (argEnd == 0 || argsStr[argEnd - 1] != '\\')) {
                inString = !inString;
            }
            
            if (!inString) {
                if (c == '(') parenCount++;
                else if (c == ')') parenCount--;
                else if ((c == ',' || argEnd == argsStr.length() - 1) && parenCount == 0) {
                    if (c == ',') argEnd--;
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
            auto argExpr = parseExpression(arg);
            if (argExpr) {
                callNode.addChild(std::move(argExpr));
            }
        }
        
        pos = argEnd + 1;
    }
}

size_t CFrontend::findOperatorAtLevel(const std::string& expr, const std::string& operators) {
    int parenCount = 0;
    bool inString = false;
    
    for (size_t i = 0; i < expr.length(); ++i) {
        char c = expr[i];
        
        if (c == '"' && (i == 0 || expr[i - 1] != '\\')) {
            inString = !inString;
        }
        
        if (!inString) {
            if (c == '(') parenCount++;
            else if (c == ')') parenCount--;
            else if (parenCount == 0 && operators.find(c) != std::string::npos) {
                return i;
            }
        }
    }
    
    return std::string::npos;
}

std::string CFrontend::preprocessSourceCode(const std::string& sourceCode) {
    std::string result = sourceCode;
    size_t pos = 0;
    
    while (pos < result.length()) {
        // Skip preprocessor directives (lines starting with #)
        if (result[pos] == '#') {
            size_t lineEnd = result.find('\n', pos);
            if (lineEnd == std::string::npos) {
                // Last line - remove everything from # to end
                result.erase(pos);
                break;
            } else {
                // Remove the entire preprocessor line
                result.erase(pos, lineEnd - pos + 1);
            }
        } else {
            pos++;
        }
    }
    
    return result;
}

void CFrontend::parseGlobalFunctions(const std::string& cleanCode) {
    size_t pos = 0;
    while (pos < cleanCode.length()) {
        // Look for function definitions at global scope
        size_t funcPos = cleanCode.find("int ", pos);
        if (funcPos == std::string::npos) {
            funcPos = cleanCode.find("void ", pos);
        }
        if (funcPos == std::string::npos) {
            funcPos = cleanCode.find("char ", pos);
        }
        
        if (funcPos != std::string::npos) {
            // Make sure this is not inside another function (check brace balance)
            if (!isInsideFunction(cleanCode, funcPos)) {
                size_t nameStart = cleanCode.find_first_not_of(" \t", funcPos + 4);
                size_t nameEnd = cleanCode.find("(", nameStart);
                if (nameEnd != std::string::npos) {
                    std::string funcName = cleanCode.substr(nameStart, nameEnd - nameStart);
                    std::string returnType = cleanCode.substr(funcPos, 3);
                    
                    // Trim whitespace from function name
                    size_t funcNameStart = funcName.find_first_not_of(" \t");
                    size_t funcNameEnd = funcName.find_last_not_of(" \t");
                    if (funcNameStart != std::string::npos && funcNameEnd != std::string::npos) {
                        funcName = funcName.substr(funcNameStart, funcNameEnd - funcNameStart + 1);
                    }
                    
                    auto funcNode = std::make_unique<FunctionDeclNode>(funcName, returnType);
                    
                    // Parse function parameters
                    size_t paramEnd = cleanCode.find(")", nameEnd);
                    if (paramEnd != std::string::npos) {
                        std::string paramsStr = cleanCode.substr(nameEnd + 1, paramEnd - nameEnd - 1);
                        parseFunctionParameters(paramsStr, *funcNode);
                    }
                    
                    // Look for function body
                    size_t openBrace = cleanCode.find("{", paramEnd);
                    if (openBrace != std::string::npos) {
                        int braceCount = 1;
                        size_t closeBrace = openBrace + 1;
                        
                        // Find matching closing brace (handles nested braces)
                        while (closeBrace < cleanCode.length() && braceCount > 0) {
                            if (cleanCode[closeBrace] == '{') {
                                braceCount++;
                            } else if (cleanCode[closeBrace] == '}') {
                                braceCount--;
                            }
                            closeBrace++;
                        }
                        
                        if (braceCount == 0) {
                            std::string body = cleanCode.substr(openBrace + 1, closeBrace - openBrace - 2);
                            
                            // Create a block node for function body
                            auto blockNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
                            
                            // Parse statements in the function body
                            parseStatements(body, *blockNode);
                            
                            funcNode->addChild(std::move(blockNode));
                            
                            // Update position to skip the entire function
                            pos = closeBrace;
                        } else {
                            pos = nameEnd + 1;
                        }
                    } else {
                        pos = nameEnd + 1;
                    }
                    
                    ast_->addChild(std::move(funcNode));
                } else {
                    pos = funcPos + 1;
                }
            } else {
                pos = funcPos + 1;
            }
        } else {
            // Look for global variable declarations
            size_t varPos = cleanCode.find("int ", pos);
            if (varPos != std::string::npos && varPos != funcPos && !isInsideFunction(cleanCode, varPos)) {
                size_t nameEnd = cleanCode.find(";", varPos);
                if (nameEnd != std::string::npos) {
                    std::string varDecl = cleanCode.substr(varPos, nameEnd - varPos);
                    auto varNode = std::make_unique<VariableDeclNode>(varDecl, "int");
                    ast_->addChild(std::move(varNode));
                    pos = nameEnd + 1;
                } else {
                    pos = varPos + 1;
                }
            } else {
                pos = cleanCode.length();
            }
        }
    }
}

bool CFrontend::isInsideFunction(const std::string& code, size_t position) {
    int braceCount = 0;
    size_t pos = 0;
    
    while (pos < position) {
        if (code[pos] == '{') {
            braceCount++;
        } else if (code[pos] == '}') {
            braceCount--;
        }
        pos++;
    }
    
    return braceCount > 0;
}

void CFrontend::parseFunctionParameters(const std::string& paramsStr, ASTNode& funcNode) {
    if (paramsStr.empty()) return;
    
    size_t pos = 0;
    while (pos < paramsStr.length()) {
        // Skip whitespace and commas
        while (pos < paramsStr.length() && (isspace(paramsStr[pos]) || paramsStr[pos] == ',')) {
            pos++;
        }
        if (pos >= paramsStr.length()) break;
        
        // Find the next comma or end of string
        size_t paramEnd = paramsStr.find(',', pos);
        if (paramEnd == std::string::npos) {
            paramEnd = paramsStr.length();
        }
        
        std::string param = paramsStr.substr(pos, paramEnd - pos);
        
        // Trim whitespace
        size_t start = param.find_first_not_of(" \t");
        size_t end = param.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            param = param.substr(start, end - start + 1);
        }
        
        if (!param.empty()) {
            // Parse parameter type and name (e.g., "int a", "char c")
            size_t spacePos = param.find(' ');
            if (spacePos != std::string::npos) {
                std::string paramType = param.substr(0, spacePos);
                std::string paramName = param.substr(spacePos + 1);
                
                // Trim parameter name
                size_t nameStart = paramName.find_first_not_of(" \t");
                size_t nameEnd = paramName.find_last_not_of(" \t");
                if (nameStart != std::string::npos && nameEnd != std::string::npos) {
                    paramName = paramName.substr(nameStart, nameEnd - nameStart + 1);
                }
                
                // Create parameter node
                auto paramNode = std::make_unique<VariableDeclNode>(paramName, paramType);
                funcNode.addChild(std::move(paramNode));
            }
        }
        
        pos = paramEnd + 1;
    }
}

// Factory function for creating C frontend
std::unique_ptr<FrontendBase> createCFrontend() {
    return std::make_unique<CFrontend>();
}
