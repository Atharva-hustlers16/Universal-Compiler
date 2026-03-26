#include "CppFrontend.h"
#include "ASTNode.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

CppFrontend::CppFrontend() : FrontendBase() {
    language_ = Language::CPP;
}

bool CppFrontend::parse(const std::string& sourceCode) {
    sourceCode_ = sourceCode;
    
    auto program = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "C++ Program");
    
    // Preprocess C++ code - remove comments and preprocessor directives
    std::string processedCode = preprocessCppCode(sourceCode);
    
    // Remove preprocessor directives
    std::string cleanCode;
    std::istringstream codeStream(processedCode);
    std::string line;
    while (std::getline(codeStream, line)) {
        if (!isCppPreprocessor(line) && !isCppIncludeDirective(line)) {
            cleanCode += line + "\n";
        }
    }
    
    // For functions with bodies, we need to extract them differently
    size_t pos = 0;
    while (pos < cleanCode.length()) {
        // Skip whitespace and comments
        while (pos < cleanCode.length() && isspace(cleanCode[pos])) {
            pos++;
        }
        
        if (pos >= cleanCode.length()) break;
        
        // Check if this is a class declaration
        size_t classKeywordPos = cleanCode.find("class ", pos);
        size_t nextOpenBrace = cleanCode.find('{', pos);
        
        if (classKeywordPos != std::string::npos && classKeywordPos < nextOpenBrace) {
            // This looks like a class declaration
            std::string classDeclaration = cleanCode.substr(pos, nextOpenBrace - pos);
            
            // Find the matching closing brace
            size_t braceStart = nextOpenBrace;
            size_t braceEnd = findMatchingCppBrace(cleanCode, braceStart);
            
            if (braceEnd != std::string::npos) {
                std::string classBody = cleanCode.substr(braceStart, braceEnd - braceStart + 1);
                std::string fullClass = classDeclaration + classBody;
                
                auto classNode = parseCppClass(fullClass, pos);
                if (classNode) {
                    program->addChild(std::move(classNode));
                }
                
                pos = braceEnd + 1;
                continue;
            }
        }
        
        // Check if this is a function declaration
        size_t nextSemicolon = cleanCode.find(';', pos);
        nextOpenBrace = cleanCode.find('{', pos);
        
        if (nextOpenBrace != std::string::npos && (nextSemicolon == std::string::npos || nextOpenBrace < nextSemicolon)) {
            // This looks like a function with a body
            std::string funcDeclaration = cleanCode.substr(pos, nextOpenBrace - pos);
            
            // Find the matching closing brace
            size_t braceStart = nextOpenBrace;
            size_t braceEnd = findMatchingCppBrace(cleanCode, braceStart);
            
            if (braceEnd != std::string::npos) {
                std::string funcBody = cleanCode.substr(braceStart, braceEnd - braceStart + 1);
                std::string fullFunction = funcDeclaration + funcBody;
                
                // Parse the complete function
                if (isCppFunctionDeclaration(funcDeclaration) || isCppMethodDeclaration(funcDeclaration)) {
                    auto funcNode = parseCppFunction(fullFunction, pos);
                    if (funcNode) {
                        program->addChild(std::move(funcNode));
                    }
                } else if (isCppConstructorDeclaration(funcDeclaration)) {
                    auto ctorNode = parseCppConstructor(fullFunction, pos);
                    if (ctorNode) {
                        program->addChild(std::move(ctorNode));
                    }
                }
                
                pos = braceEnd + 1;
                continue;
            }
        }
        
        // Handle other statements (variable declarations, etc.)
        if (nextSemicolon != std::string::npos) {
            std::string statement = cleanCode.substr(pos, nextSemicolon - pos + 1);
            
            // Parse variable declarations
            if (isCppVariableDeclaration(statement)) {
                auto varNode = parseCppVariableDeclaration(statement);
                if (varNode) {
                    program->addChild(std::move(varNode));
                }
            }
            
            pos = nextSemicolon + 1;
        } else {
            // No more semicolons or braces, we're done
            break;
        }
    }
    
    ast_ = std::move(program);
    return ast_ != nullptr;
}

std::unique_ptr<ASTNode> CppFrontend::parseCppClass(const std::string& classSignature, size_t& pos) {
    std::string className = extractCppClassName(classSignature);
    auto classNode = std::make_unique<ASTNode>(ASTNodeType::CLASS_DECL, className);
    
    // Find the class body
    size_t bracePos = classSignature.find("{");
    if (bracePos != std::string::npos) {
        size_t matchingBrace = findMatchingCppBrace(classSignature, bracePos);
        if (matchingBrace != std::string::npos) {
            std::string classBody = classSignature.substr(bracePos + 1, matchingBrace - bracePos - 1);
            
            // Parse class members
            auto members = splitCppStatements(classBody);
            for (const auto& member : members) {
                std::string trimmedMember = trimCppString(member);
                if (trimmedMember.empty()) continue;
                
                // Parse constructors
                if (isCppConstructorDeclaration(trimmedMember)) {
                    auto ctorNode = parseCppConstructor(trimmedMember, pos);
                    if (ctorNode) {
                        classNode->addChild(std::move(ctorNode));
                    }
                }
                // Parse methods
                else if (isCppMethodDeclaration(trimmedMember)) {
                    auto methodNode = parseCppMethod(trimmedMember, pos);
                    if (methodNode) {
                        classNode->addChild(std::move(methodNode));
                    }
                }
                // Parse member variables
                else if (isCppVariableDeclaration(trimmedMember)) {
                    auto varNode = parseCppVariableDeclaration(trimmedMember);
                    if (varNode) {
                        classNode->addChild(std::move(varNode));
                    }
                }
            }
        }
    }
    
    return classNode;
}

std::unique_ptr<ASTNode> CppFrontend::parseCppConstructor(const std::string& ctorSignature, size_t& pos) {
    std::string ctorName = extractCppFunctionName(ctorSignature);
    auto ctorNode = std::make_unique<ASTNode>(ASTNodeType::CONSTRUCTOR_DECL, ctorName);
    
    // Add parameters
    auto params = extractCppParameters(ctorSignature);
    for (const auto& param : params) {
        auto paramNode = std::make_unique<ASTNode>(ASTNodeType::PARAMETER, param);
        ctorNode->addChild(std::move(paramNode));
    }
    
    return ctorNode;
}

std::unique_ptr<ASTNode> CppFrontend::parseCppMethod(const std::string& methodSignature, size_t& pos) {
    std::string methodName = extractCppMethodName(methodSignature);
    auto methodNode = std::make_unique<ASTNode>(ASTNodeType::METHOD_DECL, methodName);
    
    // Add return type
    std::string returnType = extractCppReturnType(methodSignature);
    if (!returnType.empty()) {
        auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_TYPE, returnType);
        methodNode->addChild(std::move(returnNode));
    }
    
    // Add parameters
    auto params = extractCppParameters(methodSignature);
    for (const auto& param : params) {
        auto paramNode = std::make_unique<ASTNode>(ASTNodeType::PARAMETER, param);
        methodNode->addChild(std::move(paramNode));
    }
    
    // Add a simple block body for now
    auto blockNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
    methodNode->addChild(std::move(blockNode));
    
    return methodNode;
}

std::unique_ptr<ASTNode> CppFrontend::parseCppFunction(const std::string& funcSignature, size_t& pos) {
    std::string funcName = extractCppFunctionName(funcSignature);
    auto funcNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, funcName);
    
    // Add return type
    std::string returnType = extractCppReturnType(funcSignature);
    if (!returnType.empty()) {
        auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_TYPE, returnType);
        funcNode->addChild(std::move(returnNode));
    }
    
    // Add parameters
    auto params = extractCppParameters(funcSignature);
    for (const auto& param : params) {
        auto paramNode = std::make_unique<ASTNode>(ASTNodeType::PARAMETER, param);
        funcNode->addChild(std::move(paramNode));
    }
    
    // Find the function body
    size_t braceStart = funcSignature.find("{");
    if (braceStart != std::string::npos) {
        size_t braceEnd = findMatchingCppBrace(funcSignature, braceStart);
        if (braceEnd != std::string::npos) {
            std::string funcBody = funcSignature.substr(braceStart + 1, braceEnd - braceStart - 1);
            
            // Parse function body content
            auto bodyNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
            
            // Simple parsing of function body statements
            auto bodyStatements = splitCppStatements(funcBody);
            for (const auto& stmt : bodyStatements) {
                std::string trimmedStmt = trimCppString(stmt);
                if (trimmedStmt.empty()) continue;
                
                // Parse variable declarations
                if (isCppVariableDeclaration(trimmedStmt)) {
                    auto varNode = parseCppVariableDeclaration(trimmedStmt);
                    if (varNode) {
                        bodyNode->addChild(std::move(varNode));
                    }
                }
                // Parse function calls
                else if (trimmedStmt.find("(") != std::string::npos && trimmedStmt.find(")") != std::string::npos) {
                    auto callNode = parseCppFunctionCall(trimmedStmt);
                    if (callNode) {
                        bodyNode->addChild(std::move(callNode));
                    }
                }
                // Parse stream operations (cout << ...)
                else if (trimmedStmt.find("<<") != std::string::npos) {
                    auto callNode = parseCppFunctionCall(trimmedStmt);
                    if (callNode) {
                        bodyNode->addChild(std::move(callNode));
                    }
                }
                // Parse return statements
                else if (trimmedStmt.find("return") != std::string::npos) {
                    auto returnNode = std::make_unique<ASTNode>(ASTNodeType::RETURN_STMT, "return");
                    bodyNode->addChild(std::move(returnNode));
                }
            }
            
            funcNode->addChild(std::move(bodyNode));
        }
    } else {
        // No function body found, add empty block
        auto blockNode = std::make_unique<ASTNode>(ASTNodeType::BLOCK, "");
        funcNode->addChild(std::move(blockNode));
    }
    
    return funcNode;
}

std::unique_ptr<ASTNode> CppFrontend::parseCppVariableDeclaration(const std::string& decl) {
    // Simple variable declaration parsing
    size_t spacePos = decl.find(" ");
    if (spacePos != std::string::npos) {
        std::string varType = decl.substr(0, spacePos);
        std::string varNameAndValue = decl.substr(spacePos + 1);
        
        // Remove semicolon
        size_t semiPos = varNameAndValue.find(";");
        if (semiPos != std::string::npos) {
            varNameAndValue = varNameAndValue.substr(0, semiPos);
        }
        
        // Split variable name and value
        size_t equalPos = varNameAndValue.find("=");
        std::string varName = varNameAndValue;
        std::string varValue = "";
        
        if (equalPos != std::string::npos) {
            varName = varNameAndValue.substr(0, equalPos);
            varValue = varNameAndValue.substr(equalPos + 1);
        }
        
        varName = trimCppString(varName);
        varValue = trimCppString(varValue);
        
        auto varNode = std::make_unique<ASTNode>(ASTNodeType::VARIABLE_DECL, varName);
        auto typeNode = std::make_unique<ASTNode>(ASTNodeType::TYPE, varType);
        varNode->addChild(std::move(typeNode));
        
        // Add value node if there's an initialization
        if (!varValue.empty()) {
            auto valueNode = std::make_unique<ASTNode>(ASTNodeType::LITERAL, varValue);
            varNode->addChild(std::move(valueNode));
        }
        
        return varNode;
    }
    
    return nullptr;
}

std::unique_ptr<ASTNode> CppFrontend::parseCppFunctionCall(const std::string& call) {
    // Handle stream operations like cout << "text"
    if (call.find("<<") != std::string::npos) {
        // This is a stream operation, treat it as a cout call
        auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, "cout");
        
        // Extract the arguments from the stream operation
        std::string argsStr = call;
        
        // Remove "cout <<" and any leading/trailing whitespace
        size_t coutPos = argsStr.find("cout");
        if (coutPos != std::string::npos) {
            argsStr = argsStr.substr(coutPos + 4);
        }
        
        // Remove all "<<" operators and split arguments
        std::vector<std::string> args;
        std::string current;
        size_t pos = 0;
        
        while (pos < argsStr.length()) {
            if (argsStr.substr(pos, 2) == "<<") {
                if (!current.empty()) {
                    args.push_back(trimCppString(current));
                    current.clear();
                }
                pos += 2;
            } else if (argsStr[pos] == ';') {
                if (!current.empty()) {
                    args.push_back(trimCppString(current));
                    current.clear();
                }
                break;
            } else {
                current += argsStr[pos];
                pos++;
            }
        }
        
        if (!current.empty()) {
            args.push_back(trimCppString(current));
        }
        
        for (const auto& arg : args) {
            if (!arg.empty()) {
                // Check if this argument is a method call (obj.method())
                bool isMethodCall = false;
                if (arg.find('.') != std::string::npos && 
                    arg.find('(') != std::string::npos && 
                    arg.find(')') != std::string::npos) {
                    // This looks like a method call
                    size_t dotPos = arg.find('.');
                    size_t parenStart = arg.find('(');
                    if (dotPos < parenStart) {
                        isMethodCall = true;
                    }
                }
                
                // Check if this argument is a variable reference (single word, no quotes)
                bool isVariable = false;
                if (!isMethodCall && arg.find(' ') == std::string::npos && 
                    arg.front() != '"' && arg.back() != '"' &&
                    arg.find('(') == std::string::npos && arg.find(')') == std::string::npos &&
                    !isdigit(arg.front()) && arg != "endl" && arg != "std::endl") {
                    // This looks like a variable reference
                    isVariable = true;
                }
                
                std::unique_ptr<ASTNode> argNode;
                if (isMethodCall) {
                    // Parse as method call
                    size_t dotPos = arg.find('.');
                    size_t parenStart = arg.find('(');
                    size_t parenEnd = arg.find(')', parenStart);
                    
                    std::string objName = arg.substr(0, dotPos);
                    std::string methodName = arg.substr(dotPos + 1, parenStart - dotPos - 1);
                    std::string methodCall = objName + "." + methodName;
                    
                    argNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, methodCall);
                    
                    // Add object as first argument
                    auto objNode = std::make_unique<ASTNode>(ASTNodeType::ARGUMENT, objName);
                    argNode->addChild(std::move(objNode));
                } else if (isVariable) {
                    argNode = std::make_unique<ASTNode>(ASTNodeType::IDENTIFIER, arg);
                } else {
                    argNode = std::make_unique<ASTNode>(ASTNodeType::ARGUMENT, arg);
                }
                
                callNode->addChild(std::move(argNode));
            }
        }
        
        return callNode;
    }
    
    // Regular function call parsing - extract function name
    size_t parenStart = call.find('(');
    if (parenStart == std::string::npos) return nullptr;
    
    std::string funcName = call.substr(0, parenStart);
    funcName = trimCppString(funcName);
    
    auto callNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_CALL, funcName);
    
    // Extract arguments
    size_t parenEnd = call.find(')', parenStart);
    if (parenEnd != std::string::npos) {
        std::string argsStr = call.substr(parenStart + 1, parenEnd - parenStart - 1);
        // Simple argument parsing - split by commas
        std::vector<std::string> args;
        std::string current;
        int parenLevel = 0;
        
        for (char c : argsStr) {
            if (c == '(') parenLevel++;
            else if (c == ')') parenLevel--;
            else if (c == ',' && parenLevel == 0) {
                args.push_back(trimCppString(current));
                current.clear();
            } else {
                current += c;
            }
        }
        
        if (!current.empty()) {
            args.push_back(trimCppString(current));
        }
        
        for (const auto& arg : args) {
            if (!arg.empty()) {
                auto argNode = std::make_unique<ASTNode>(ASTNodeType::ARGUMENT, arg);
                callNode->addChild(std::move(argNode));
            }
        }
    }
    
    return callNode;
}

std::unique_ptr<ASTNode> CppFrontend::parseCppExpression(const std::string& expr) {
    // Simple expression parsing
    if (expr.find("+") != std::string::npos) {
        auto opNode = std::make_unique<ASTNode>(ASTNodeType::BINARY_OP, "+");
        // Add operands (simplified)
        opNode->addChild(std::make_unique<ASTNode>(ASTNodeType::LITERAL, "left"));
        opNode->addChild(std::make_unique<ASTNode>(ASTNodeType::LITERAL, "right"));
        return opNode;
    }
    
    return std::make_unique<ASTNode>(ASTNodeType::LITERAL, expr);
}

std::unique_ptr<ASTNode> CppFrontend::parseCppStatement(const std::string& statement, size_t& pos) {
    std::string trimmedStmt = trimCppString(statement);
    
    if (isCppVariableDeclaration(trimmedStmt)) {
        return parseCppVariableDeclaration(trimmedStmt);
    }
    
    if (trimmedStmt.find("(") != std::string::npos && trimmedStmt.find(")") != std::string::npos) {
        return parseCppFunctionCall(trimmedStmt);
    }
    
    return nullptr;
}

// Helper functions
std::string CppFrontend::preprocessCppCode(const std::string& sourceCode) {
    return removeCppComments(sourceCode);
}

std::string CppFrontend::removeCppComments(const std::string& code) {
    std::string result;
    bool inLineComment = false;
    bool inBlockComment = false;
    
    for (size_t i = 0; i < code.length(); ++i) {
        if (inLineComment) {
            if (code[i] == '\n') {
                inLineComment = false;
                result += code[i];
            }
        } else if (inBlockComment) {
            if (i + 1 < code.length() && code[i] == '*' && code[i + 1] == '/') {
                inBlockComment = false;
                i++; // Skip '/'
            }
        } else {
            if (i + 1 < code.length() && code[i] == '/' && code[i + 1] == '/') {
                inLineComment = true;
                i++; // Skip second '/'
            } else if (i + 1 < code.length() && code[i] == '/' && code[i + 1] == '*') {
                inBlockComment = true;
                i++; // Skip '*'
            } else {
                result += code[i];
            }
        }
    }
    
    return result;
}

std::vector<std::string> CppFrontend::splitCppStatements(const std::string& sourceCode) {
    std::vector<std::string> statements;
    std::string current;
    int braceLevel = 0;
    
    for (char c : sourceCode) {
        current += c;
        
        if (c == '{') {
            braceLevel++;
        } else if (c == '}') {
            braceLevel--;
        } else if (c == ';' && braceLevel == 0) {
            statements.push_back(current);
            current.clear();
        }
    }
    
    if (!current.empty()) {
        statements.push_back(current);
    }
    
    return statements;
}

std::string CppFrontend::trimCppString(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

bool CppFrontend::isCppComment(const std::string& line) {
    return line.find("//") == 0 || line.find("/*") == 0;
}

bool CppFrontend::isCppPreprocessor(const std::string& line) {
    return line.find("#") == 0;
}

bool CppFrontend::isCppIncludeDirective(const std::string& line) {
    return line.find("#include") == 0;
}

bool CppFrontend::isCppNamespaceDeclaration(const std::string& line) {
    return line.find("namespace") == 0;
}

bool CppFrontend::isCppClassDeclaration(const std::string& line) {
    return line.find("class ") != std::string::npos && line.find("{") != std::string::npos;
}

bool CppFrontend::isCppConstructorDeclaration(const std::string& line) {
    // Simple check - can be enhanced
    return line.find("(") != std::string::npos && line.find(")") != std::string::npos &&
           line.find(";") == std::string::npos && line.find("class") == std::string::npos &&
           line.find("return") == std::string::npos && line.find("void") == std::string::npos &&
           line.find("int") == std::string::npos && line.find("float") == std::string::npos &&
           line.find("double") == std::string::npos && line.find("char") == std::string::npos &&
           line.find("bool") == std::string::npos;
}

bool CppFrontend::isCppMethodDeclaration(const std::string& line) {
    return (line.find("int ") != std::string::npos || 
            line.find("void ") != std::string::npos ||
            line.find("float ") != std::string::npos ||
            line.find("double ") != std::string::npos ||
            line.find("char ") != std::string::npos ||
            line.find("bool ") != std::string::npos ||
            line.find("string ") != std::string::npos ||
            line.find("int(") != std::string::npos ||  // Handle "int main(" without space
            line.find("void(") != std::string::npos ||
            line.find("float(") != std::string::npos ||
            line.find("double(") != std::string::npos ||
            line.find("char(") != std::string::npos ||
            line.find("bool(") != std::string::npos ||
            line.find("string(") != std::string::npos) &&
           line.find("(") != std::string::npos && line.find(")") != std::string::npos;
}

bool CppFrontend::isCppFunctionDeclaration(const std::string& line) {
    return isCppMethodDeclaration(line) && !isCppConstructorDeclaration(line);
}

bool CppFrontend::isCppVariableDeclaration(const std::string& line) {
    return (line.find("int ") != std::string::npos || 
            line.find("float ") != std::string::npos ||
            line.find("double ") != std::string::npos ||
            line.find("char ") != std::string::npos ||
            line.find("bool ") != std::string::npos ||
            line.find("string ") != std::string::npos) &&
           line.find("(") == std::string::npos && line.find(";") != std::string::npos;
}

std::string CppFrontend::extractCppReturnType(const std::string& signature) {
    size_t spacePos = signature.find(" ");
    if (spacePos != std::string::npos) {
        return signature.substr(0, spacePos);
    }
    return "";
}

std::string CppFrontend::extractCppFunctionName(const std::string& signature) {
    size_t spacePos = signature.find(" ");
    if (spacePos != std::string::npos) {
        size_t parenPos = signature.find("(", spacePos);
        if (parenPos != std::string::npos) {
            return signature.substr(spacePos + 1, parenPos - spacePos - 1);
        }
    }
    return "";
}

std::string CppFrontend::extractCppClassName(const std::string& signature) {
    size_t classPos = signature.find("class ");
    if (classPos != std::string::npos) {
        size_t bracePos = signature.find("{", classPos);
        if (bracePos != std::string::npos) {
            return signature.substr(classPos + 6, bracePos - classPos - 6);
        }
    }
    return "";
}

std::string CppFrontend::extractCppMethodName(const std::string& signature) {
    return extractCppFunctionName(signature);
}

std::vector<std::string> CppFrontend::extractCppParameters(const std::string& signature) {
    std::vector<std::string> params;
    size_t parenStart = signature.find("(");
    size_t parenEnd = signature.find(")", parenStart);
    
    if (parenStart != std::string::npos && parenEnd != std::string::npos) {
        std::string paramsStr = signature.substr(parenStart + 1, parenEnd - parenStart - 1);
        
        std::stringstream ss(paramsStr);
        std::string param;
        while (std::getline(ss, param, ',')) {
            param = trimCppString(param);
            if (!param.empty()) {
                params.push_back(param);
            }
        }
    }
    
    return params;
}

size_t CppFrontend::findMatchingCppBrace(const std::string& code, size_t openPos) {
    int count = 1;
    for (size_t i = openPos + 1; i < code.length(); ++i) {
        if (code[i] == '{') {
            count++;
        } else if (code[i] == '}') {
            count--;
            if (count == 0) {
                return i;
            }
        }
    }
    return std::string::npos;
}

size_t CppFrontend::findMatchingCppParenthesis(const std::string& expr, size_t openPos) {
    int count = 1;
    for (size_t i = openPos + 1; i < expr.length(); ++i) {
        if (expr[i] == '(') {
            count++;
        } else if (expr[i] == ')') {
            count--;
            if (count == 0) {
                return i;
            }
        }
    }
    return std::string::npos;
}

bool CppFrontend::isInsideCppFunction(const std::string& sourceCode, size_t pos) {
    // Simplified implementation
    return false;
}

bool CppFrontend::isInsideCppClass(const std::string& sourceCode, size_t pos) {
    // Simplified implementation
    return false;
}
