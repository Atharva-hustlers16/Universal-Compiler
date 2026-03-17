#include "JavaFrontend.h"
#include <iostream>
#include <sstream>

// Check if ANTLR is available
#ifdef ANTLR4Runtime_FOUND
#include "JavaLexer.h"
#include "JavaParser.h"
#include "JavaParserVisitor.h"
#endif

JavaFrontend::JavaFrontend() {
#ifdef ANTLR4Runtime_FOUND
    // Initialize ANTLR components
    lexer_ = nullptr;
    parser_ = nullptr;
    parseTree_ = nullptr;
    visitor_ = nullptr;
#endif
    language_ = Language::JAVA;
}

JavaFrontend::~JavaFrontend() {
    // Cleanup handled by smart pointers
}

bool JavaFrontend::parse(const std::string& sourceCode) {
#ifdef ANTLR4Runtime_FOUND
    if (!parser_) {
        std::cerr << "Error: ANTLR parser not initialized" << std::endl;
        return false;
    }
    
    // Use ANTLR parser
    input_ = std::make_unique<antlr4::ANTLRInputStream>(sourceCode);
    lexer_ = std::make_unique<JavaLexer>(input_.get());
    tokens_ = std::make_unique<antlr4::CommonTokenStream>(lexer_.get());
    parser_ = std::make_unique<JavaParser>(tokens_.get());
    
    parseTree_ = parser_->compilationUnit();
    visitor_ = std::make_unique<JavaASTVisitor>(this);
    
    ast_ = visitor_->visitCompilationUnit(parseTree_);
    return true;
#else
    // Enhanced fallback parser for when ANTLR is not available
    std::cout << "Using enhanced fallback Java parser (ANTLR4 not available)" << std::endl;
    
    ast_ = std::make_unique<ProgramNode>();
    
    // Parse classes and methods
    size_t pos = 0;
    while (pos < sourceCode.length()) {
        // Look for class declarations
        size_t classPos = sourceCode.find("class ", pos);
        if (classPos != std::string::npos) {
            size_t classNameStart = classPos + 6;
            size_t classNameEnd = sourceCode.find_first_of(" {", classNameStart);
            if (classNameEnd != std::string::npos) {
                std::string className = sourceCode.substr(classNameStart, classNameEnd - classNameStart);
                
                auto classNode = std::make_unique<FunctionDeclNode>(className, "class");
                
                // Look for class body
                size_t openBrace = sourceCode.find("{", classNameEnd);
                if (openBrace != std::string::npos) {
                    int braceCount = 1;
                    size_t closeBrace = openBrace + 1;
                    
                    // Find matching closing brace for class block
                    while (closeBrace < sourceCode.length() && braceCount > 0) {
                        if (sourceCode[closeBrace] == '{') braceCount++;
                        else if (sourceCode[closeBrace] == '}') braceCount--;
                        closeBrace++;
                    }
                    
                    if (braceCount == 0) {
                        closeBrace--; // Point to the actual '}' character
                        std::string classBody = sourceCode.substr(openBrace + 1, closeBrace - openBrace - 1);
                        
                        size_t methodPos = 0;
                        while (methodPos < classBody.length()) {
                            // Find next possible method by looking for "public " or "private "
                            size_t pubPos = classBody.find("public ", methodPos);
                            size_t privPos = classBody.find("private ", methodPos);
                            
                            size_t methodStart = std::string::npos;
                            if (pubPos != std::string::npos && privPos != std::string::npos) {
                                methodStart = std::min(pubPos, privPos);
                            } else if (pubPos != std::string::npos) {
                                methodStart = pubPos;
                            } else if (privPos != std::string::npos) {
                                methodStart = privPos;
                            }
                            
                            if (methodStart != std::string::npos) {
                                // Find the opening parenthesis of the method
                                size_t parenPos = classBody.find("(", methodStart);
                                // Ensure this is a method by checking for parenthesis before the brace
                                size_t nextBrace = classBody.find("{", methodStart);
                                
                                if (parenPos != std::string::npos && (nextBrace == std::string::npos || parenPos < nextBrace)) {
                                    // Extract the method name (word just before '(')
                                    size_t nameStart = classBody.find_last_of(" \t", parenPos - 1);
                                    if (nameStart != std::string::npos) {
                                        nameStart++; // Skip space
                                        std::string methodName = classBody.substr(nameStart, parenPos - nameStart);
                                        
                                        // Ignore 'class' declarations if they slipped in
                                        if (methodName != "class" && !methodName.empty()) {
                                            auto methodNode = std::make_unique<FunctionDeclNode>(methodName, "void");
                                            
                                            // Parse method parameters
                                            size_t closeParenPos = classBody.find(")", parenPos);
                                            if (closeParenPos != std::string::npos && closeParenPos > parenPos + 1) {
                                                std::string paramsStr = classBody.substr(parenPos + 1, closeParenPos - parenPos - 1);
                                                size_t paramStart = 0;
                                                while (paramStart < paramsStr.length()) {
                                                    while (paramStart < paramsStr.length() && isspace(paramsStr[paramStart])) paramStart++;
                                                    if (paramStart >= paramsStr.length()) break;
                                                    
                                                    size_t paramEnd = paramsStr.find(",", paramStart);
                                                    if (paramEnd == std::string::npos) paramEnd = paramsStr.length();
                                                    
                                                    std::string param = paramsStr.substr(paramStart, paramEnd - paramStart);
                                                    // Clean up parameter (e.g., "int x" -> "x", but keeping full string for AST)
                                                    size_t trimStart = param.find_first_not_of(" \t");
                                                    size_t trimEnd = param.find_last_not_of(" \t");
                                                    if (trimStart != std::string::npos && trimEnd != std::string::npos) {
                                                        param = param.substr(trimStart, trimEnd - trimStart + 1);
                                                    }
                                                    
                                                    if (!param.empty()) {
                                                        methodNode->addChild(std::make_unique<ASTNode>(ASTNodeType::VARIABLE_DECL, param));
                                                    }
                                                    
                                                    paramStart = paramEnd + 1;
                                                }
                                            }
                                            
                                            size_t methodOpenBrace = classBody.find("{", parenPos);
                                            if (methodOpenBrace != std::string::npos) {
                                                int methodBraceCount = 1;
                                                size_t methodCloseBrace = methodOpenBrace + 1;
                                                
                                                while (methodCloseBrace < classBody.length() && methodBraceCount > 0) {
                                                    if (classBody[methodCloseBrace] == '{') methodBraceCount++;
                                                    else if (classBody[methodCloseBrace] == '}') methodBraceCount--;
                                                    methodCloseBrace++;
                                                }
                                                
                                                if (methodBraceCount == 0) {
                                                    std::string methodBody = classBody.substr(methodOpenBrace + 1, methodCloseBrace - methodOpenBrace - 2);
                                                    
                                                    auto blockNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
                                                    parseStatements(methodBody, *blockNode);
                                                    methodNode->addChild(std::move(blockNode));
                                                    
                                                    methodPos = methodCloseBrace;
                                                } else {
                                                    methodPos = methodOpenBrace + 1;
                                                }
                                            } else {
                                                methodPos = parenPos + 1;
                                            }
                                            
                                            classNode->addChild(std::move(methodNode));
                                        } else {
                                            // Step past this keyword match to avoid infinite loop
                                            methodPos = methodStart + 6;
                                        }
                                    } else {
                                        methodPos = methodStart + 6;
                                    }
                                } else {
                                    // Not a method (e.g., class field declaration)
                                    methodPos = methodStart + 6;
                                }
                            } else {
                                break; // No more methods found
                            }
                        }
                    }
                }
                
                ast_->addChild(std::move(classNode));
                pos = openBrace + 1;
            } else {
                pos = classPos + 1;
            }
        } else {
            pos = sourceCode.length();
        }
    }
    
    return true;
#endif
}

std::unique_ptr<ASTNode> JavaFrontend::getAST() const {
    if (ast_) {
        return ast_->clone();
    }
    return nullptr;
}

// Fallback Parser Helper Methods
void JavaFrontend::parseStatements(const std::string& body, ASTNode& blockNode) {
    size_t pos = 0;
    while (pos < body.length()) {
        while (pos < body.length() && isspace(body[pos])) pos++;
        if (pos >= body.length()) break;
        
        // Skip single-line comments
        if (pos + 1 < body.length() && body[pos] == '/' && body[pos + 1] == '/') {
            while (pos < body.length() && body[pos] != '\n') pos++;
            continue;
        }
        
        // Skip multi-line comments
        if (pos + 1 < body.length() && body[pos] == '/' && body[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < body.length() && !(body[pos] == '*' && body[pos + 1] == '/')) pos++;
            pos += 2; // Skip */
            continue;
        }
        
        // Check for block statements like if
        if (pos + 2 < body.length() && body.substr(pos, 2) == "if") {
            size_t openParen = body.find("(", pos);
            if (openParen != std::string::npos) {
                int parenCount = 1;
                size_t closeParen = openParen + 1;
                while (closeParen < body.length() && parenCount > 0) {
                    if (body[closeParen] == '(') parenCount++;
                    else if (body[closeParen] == ')') parenCount--;
                    closeParen++;
                }
                
                if (parenCount == 0) {
                    size_t openBrace = body.find("{", closeParen);
                    if (openBrace != std::string::npos) {
                        int braceCount = 1;
                        size_t closeBrace = openBrace + 1;
                        while (closeBrace < body.length() && braceCount > 0) {
                            if (body[closeBrace] == '{') braceCount++;
                            else if (body[closeBrace] == '}') braceCount--;
                            closeBrace++;
                        }
                        
                        if (braceCount == 0) {
                            std::string conditionStr = body.substr(openParen + 1, closeParen - openParen - 2);
                            std::string ifBody = body.substr(openBrace + 1, closeBrace - openBrace - 2);
                            
                            auto ifNode = std::make_unique<ASTNode>(ASTNodeType::IF_STMT, "if");
                            
                            auto condNode = parseExpression(conditionStr);
                            if (condNode) ifNode->addChild(std::move(condNode));
                            
                            auto bodyNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
                            parseStatements(ifBody, *bodyNode);
                            ifNode->addChild(std::move(bodyNode));
                            
                            blockNode.addChild(std::move(ifNode));
                            pos = closeBrace;
                            
                            // Check for else branch
                            size_t nextPos = pos;
                            while (nextPos < body.length() && isspace(body[nextPos])) nextPos++;
                            if (nextPos + 4 <= body.length() && body.substr(nextPos, 4) == "else") {
                                size_t elseOpenBrace = body.find("{", nextPos);
                                if (elseOpenBrace != std::string::npos) {
                                    int elseBraceCount = 1;
                                    size_t elseCloseBrace = elseOpenBrace + 1;
                                    while (elseCloseBrace < body.length() && elseBraceCount > 0) {
                                        if (body[elseCloseBrace] == '{') elseBraceCount++;
                                        else if (body[elseCloseBrace] == '}') elseBraceCount--;
                                        elseCloseBrace++;
                                    }
                                    if (elseBraceCount == 0) {
                                        std::string elseBody = body.substr(elseOpenBrace + 1, elseCloseBrace - elseOpenBrace - 2);
                                        auto elseBlockNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
                                        parseStatements(elseBody, *elseBlockNode);
                                        // Add else block as the 3rd child of the if statement node
                                        blockNode.getChildren().back()->addChild(std::move(elseBlockNode));
                                        pos = elseCloseBrace;
                                    }
                                }
                            }
                            
                            continue;
                        }
                    }
                }
            }
        }
        
        // Find end of current statement (semicolon)
        size_t stmtEnd = pos;
        bool inString = false;
        
        while (stmtEnd < body.length()) {
            char c = body[stmtEnd];
            if (c == '"' && (stmtEnd == 0 || body[stmtEnd - 1] != '\\')) {
                inString = !inString;
            }
            if (!inString && c == ';') break;
            stmtEnd++;
        }
        
        if (stmtEnd >= body.length()) stmtEnd = body.length() - 1;
        std::string statement = body.substr(pos, stmtEnd - pos);
        
        // Trim statement
        size_t start = statement.find_first_not_of(" \t\n\r");
        size_t end = statement.find_last_not_of(" \t\n\r");
        if (start != std::string::npos && end != std::string::npos) {
            statement = statement.substr(start, end - start + 1);
            auto stmtNode = parseStatement(statement);
            if (stmtNode) blockNode.addChild(std::move(stmtNode));
        }
        
        pos = stmtEnd + 1;
    }
}

std::unique_ptr<ASTNode> JavaFrontend::parseStatement(const std::string& statement) {
    if (statement.find("int ") == 0 || statement.find("String ") == 0) {
        size_t spacePos = statement.find(" ");
        std::string varType = statement.substr(0, spacePos);
        std::string rightSide = statement.substr(spacePos + 1);
        
        size_t start = rightSide.find_first_not_of(" \t");
        if (start != std::string::npos) rightSide = rightSide.substr(start);
        
        size_t equalPos = rightSide.find("=");
        std::string varName;
        std::unique_ptr<ASTNode> initExpr = nullptr;
        
        if (equalPos != std::string::npos) {
            varName = rightSide.substr(0, equalPos);
            std::string initStr = rightSide.substr(equalPos + 1);
            
            size_t nStart = varName.find_first_not_of(" \t");
            size_t nEnd = varName.find_last_not_of(" \t");
            if (nStart != std::string::npos && nEnd != std::string::npos) varName = varName.substr(nStart, nEnd - nStart + 1);
            
            size_t iStart = initStr.find_first_not_of(" \t");
            size_t iEnd = initStr.find_last_not_of(" \t");
            if (iStart != std::string::npos && iEnd != std::string::npos) initStr = initStr.substr(iStart, iEnd - iStart + 1);
            
            initExpr = parseExpression(initStr);
        } else {
            varName = rightSide;
            size_t nStart = varName.find_first_not_of(" \t");
            size_t nEnd = varName.find_last_not_of(" \t");
            if (nStart != std::string::npos && nEnd != std::string::npos) varName = varName.substr(nStart, nEnd - nStart + 1);
        }
        
        auto varDeclNode = std::make_unique<VariableDeclNode>(varName, varType);
        if (initExpr) varDeclNode->addChild(std::move(initExpr));
        return std::move(varDeclNode);
    }
    
    // Check for System.out.println
    size_t callPos = statement.find("System.out.println");
    if (callPos != std::string::npos) {
        size_t openParen = statement.find("(", callPos);
        if (openParen != std::string::npos) {
            size_t closeParen = openParen + 1;
            bool inString = false;
            int parenCount = 1;
            while (closeParen < statement.length() && parenCount > 0) {
                char c = statement[closeParen];
                if (c == '"' && (closeParen == 0 || statement[closeParen - 1] != '\\')) inString = !inString;
                if (!inString) {
                    if (c == '(') parenCount++;
                    else if (c == ')') parenCount--;
                }
                closeParen++;
            }
            if (parenCount == 0) {
                closeParen--;
                std::string argsStr = statement.substr(openParen + 1, closeParen - openParen - 1);
                auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, "System.out.println"); // Mapped to our interpreter's println equivalent if provided, or handled by interpreter
                parseFunctionArguments(argsStr, *callNode);
                return std::move(callNode);
            }
        }
    }
    
    // Check for return
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
            if (expr) returnNode->addChild(std::move(expr));
        }
        return std::move(returnNode);
    }
    
    // Default assignment or other expression
    return parseExpression(statement);
}


std::unique_ptr<ASTNode> JavaFrontend::parseExpression(const std::string& expr) {
    if (expr.length() >= 2 && expr[0] == '"' && expr[expr.length() - 1] == '"') {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, expr);
    }
    
    // Check > and < (lowest precedence)
    size_t cmpPos = findOperatorAtLevel(expr, "><");
    if (cmpPos != std::string::npos) {
        std::string left = expr.substr(0, cmpPos);
        std::string right = expr.substr(cmpPos + 1);
        
        size_t leftStart = left.find_first_not_of(" \t");
        size_t leftEnd = left.find_last_not_of(" \t");
        if (leftStart != std::string::npos && leftEnd != std::string::npos) left = left.substr(leftStart, leftEnd - leftStart + 1);
        
        size_t rightStart = right.find_first_not_of(" \t");
        size_t rightEnd = right.find_last_not_of(" \t");
        if (rightStart != std::string::npos && rightEnd != std::string::npos) right = right.substr(rightStart, rightEnd - rightStart + 1);
        
        auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, std::string(1, expr[cmpPos]));
        auto leftExpr = parseExpression(left);
        auto rightExpr = parseExpression(right);
        
        if (leftExpr) binOpNode->addChild(std::move(leftExpr));
        if (rightExpr) binOpNode->addChild(std::move(rightExpr));
        return std::move(binOpNode);
    }

    size_t plusPos = findOperatorAtLevel(expr, "+-");
    if (plusPos != std::string::npos) {
        std::string left = expr.substr(0, plusPos);
        std::string right = expr.substr(plusPos + 1);
        
        size_t leftStart = left.find_first_not_of(" \t");
        size_t leftEnd = left.find_last_not_of(" \t");
        if (leftStart != std::string::npos && leftEnd != std::string::npos) left = left.substr(leftStart, leftEnd - leftStart + 1);
        
        size_t rightStart = right.find_first_not_of(" \t");
        size_t rightEnd = right.find_last_not_of(" \t");
        if (rightStart != std::string::npos && rightEnd != std::string::npos) right = right.substr(rightStart, rightEnd - rightStart + 1);
        
        auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, std::string(1, expr[plusPos]));
        auto leftExpr = parseExpression(left);
        auto rightExpr = parseExpression(right);
        
        if (leftExpr) binOpNode->addChild(std::move(leftExpr));
        if (rightExpr) binOpNode->addChild(std::move(rightExpr));
        return std::move(binOpNode);
    }
    
    size_t mulPos = findOperatorAtLevel(expr, "*/");
    if (mulPos != std::string::npos) {
        std::string left = expr.substr(0, mulPos);
        std::string right = expr.substr(mulPos + 1);
        
        size_t leftStart = left.find_first_not_of(" \t");
        size_t leftEnd = left.find_last_not_of(" \t");
        if (leftStart != std::string::npos && leftEnd != std::string::npos) left = left.substr(leftStart, leftEnd - leftStart + 1);
        
        size_t rightStart = right.find_first_not_of(" \t");
        size_t rightEnd = right.find_last_not_of(" \t");
        if (rightStart != std::string::npos && rightEnd != std::string::npos) right = right.substr(rightStart, rightEnd - rightStart + 1);
        
        auto binOpNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, std::string(1, expr[mulPos]));
        auto leftExpr = parseExpression(left);
        auto rightExpr = parseExpression(right);
        
        if (leftExpr) binOpNode->addChild(std::move(leftExpr));
        if (rightExpr) binOpNode->addChild(std::move(rightExpr));
        return std::move(binOpNode);
    }

    // Function calls
    if (expr.back() == ')' && expr.find('(') != std::string::npos) {
        size_t parenPos = expr.find('(');
        std::string funcName = expr.substr(0, parenPos);
        
        // Trim funcName
        size_t funcNameStart = funcName.find_first_not_of(" \t");
        size_t funcNameEnd = funcName.find_last_not_of(" \t");
        if (funcNameStart != std::string::npos && funcNameEnd != std::string::npos) {
            funcName = funcName.substr(funcNameStart, funcNameEnd - funcNameStart + 1);
            auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, funcName);
            std::string argsStr = expr.substr(parenPos + 1, expr.length() - parenPos - 2);
            parseFunctionArguments(argsStr, *callNode);
            return std::move(callNode);
        }
    }

    if (isdigit(expr[0]) || (expr[0] == '-' && expr.length() > 1 && isdigit(expr[1]))) {
        return std::make_unique<ASTNode>(ASTNodeType::LITERAL, expr);
    }
    if (!expr.empty() && (isalpha(expr[0]) || expr[0] == '_')) {
        return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, expr);
    }
    if (!expr.empty()) {
        return std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, expr);
    }
    return nullptr;
}

void JavaFrontend::parseFunctionArguments(const std::string& argsStr, ASTNode& callNode) {
    if (argsStr.empty()) return;
    size_t pos = 0;
    while (pos < argsStr.length()) {
        while (pos < argsStr.length() && (isspace(argsStr[pos]) || argsStr[pos] == ',')) pos++;
        if (pos >= argsStr.length()) break;
        
        size_t argEnd = pos;
        bool inString = false;
        int parenCount = 0;
        
        while (argEnd < argsStr.length()) {
            char c = argsStr[argEnd];
            if (c == '"' && (argEnd == 0 || argsStr[argEnd - 1] != '\\')) inString = !inString;
            
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
        size_t start = arg.find_first_not_of(" \t");
        size_t end = arg.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) arg = arg.substr(start, end - start + 1);
        
        if (!arg.empty()) {
            auto argExpr = parseExpression(arg);
            if (argExpr) callNode.addChild(std::move(argExpr));
        }
        pos = argEnd + 1;
    }
}

size_t JavaFrontend::findOperatorAtLevel(const std::string& expr, const std::string& operators) {
    int parenCount = 0;
    bool inString = false;
    for (size_t i = 0; i < expr.length(); ++i) {
        char c = expr[i];
        if (c == '"' && (i == 0 || expr[i - 1] != '\\')) inString = !inString;
        if (!inString) {
            if (c == '(') parenCount++;
            else if (c == ')') parenCount--;
            else if (parenCount == 0 && operators.find(c) != std::string::npos) return i;
        }
    }
    return std::string::npos;
}

// Factory function for creating Java frontend
std::unique_ptr<FrontendBase> createJavaFrontend() {
    return std::make_unique<JavaFrontend>();
}