#include "EnhancedJavaFrontend.h"
#include "ASTNode.h"
#include <iostream>
#include <sstream>
#include <regex>

EnhancedJavaFrontend::EnhancedJavaFrontend() {
    std::cout << "Enhanced Java Frontend initialized" << std::endl;
}

EnhancedJavaFrontend::~EnhancedJavaFrontend() {
    ast_.reset();
}

bool EnhancedJavaFrontend::parse(const std::string& sourceCode) {
    if (sourceCode.empty()) {
        std::cerr << "Empty source code" << std::endl;
        return false;
    }
    
    // Clean up previous AST
    ast_.reset();
    
#ifdef ANTLR_AVAILABLE
    std::cout << "Using ANTLR4 for Java parsing" << std::endl;
    parseWithANTLR(sourceCode);
#else
    std::cout << "Using enhanced fallback parser for Java" << std::endl;
    parseWithFallback(sourceCode);
#endif
    
    return ast_ != nullptr;
}

std::unique_ptr<ASTNode> EnhancedJavaFrontend::getAST() {
    return ast_ ? std::make_unique<ASTNode>(*ast_) : nullptr;
}

#ifdef ANTLR_AVAILABLE
void EnhancedJavaFrontend::parseWithANTLR(const std::string& sourceCode) {
    try {
        antlr4::ANTLRInputStream input(sourceCode);
        JavaLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        JavaParser parser(&tokens);
        
        JavaParser::CompilationUnitContext* tree = parser.compilationUnit();
        
        JavaASTVisitor visitor;
        visitor.visitCompilationUnit(tree);
        ast_ = visitor.getAST();
        
        std::cout << "ANTLR4 parsing completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ANTLR4 parsing failed: " << e.what() << std::endl;
        std::cout << "Falling back to enhanced parser" << std::endl;
        parseWithFallback(sourceCode);
    }
}
#else
void EnhancedJavaFrontend::parseWithANTLR(const std::string& sourceCode) {
    // Not available, use fallback
    parseWithFallback(sourceCode);
}
#endif

void EnhancedJavaFrontend::parseWithFallback(const std::string& source) {
    // Create root program node
    ast_ = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "JavaProgram");
    
    // Extract imports
    extractImports(source);
    
    // Extract classes
    extractClasses(source);
    
    // Extract standalone methods (if any)
    extractMethods(source);
    
    std::cout << "Enhanced fallback parsing completed" << std::endl;
}

void EnhancedJavaFrontend::extractClasses(const std::string& source) {
    std::regex classRegex(R"(class\s+(\w+)\s*(?:extends\s+(\w+))?\s*\{([^}]*)\})");
    std::sregex_iterator iter(source.begin(), source.end(), classRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        std::string className = match[1].str();
        std::string classBody = match[3].str();
        
        auto classNode = createClassNode(className);
        
        // Extract methods from class body
        std::regex methodRegex(R"((?:public|private|protected)?\s*(?:static)?\s*(\w+)\s+(\w+)\s*\([^)]*\)\s*\{)");
        std::sregex_iterator methodIter(classBody.begin(), classBody.end(), methodRegex);
        std::sregex_iterator methodEnd;
        
        for (; methodIter != methodEnd; ++methodIter) {
            std::smatch methodMatch = *methodIter;
            std::string returnType = methodMatch[1].str();
            std::string methodName = methodMatch[2].str();
            
            auto methodNode = createMethodNode(methodName, returnType, {});
            classNode->addChild(std::move(methodNode));
        }
        
        ast_->addChild(std::move(classNode));
    }
}

void EnhancedJavaFrontend::extractMethods(const std::string& source) {
    // Extract methods outside classes (rare in Java)
    std::regex methodRegex(R"((?:public|private|protected)?\s*(?:static)?\s*(\w+)\s+(\w+)\s*\([^)]*\)\s*\{)");
    std::sregex_iterator iter(source.begin(), source.end(), methodRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        std::string returnType = match[1].str();
        std::string methodName = match[2].str();
        
        auto methodNode = createMethodNode(methodName, returnType, {});
        ast_->addChild(std::move(methodNode));
    }
}

void EnhancedJavaFrontend::extractImports(const std::string& source) {
    std::regex importRegex(R"(import\s+([\w\.]+);)");
    std::sregex_iterator iter(source.begin(), source.end(), importRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        std::string importName = match[1].str();
        
        auto importNode = createImportNode(importName);
        ast_->addChild(std::move(importNode));
    }
}

std::unique_ptr<ASTNode> EnhancedJavaFrontend::createClassNode(const std::string& className) {
    return std::make_unique<ASTNode>(ASTNodeType::CLASS_DECL, className);
}

std::unique_ptr<ASTNode> EnhancedJavaFrontend::createMethodNode(const std::string& methodName, 
                                                              const std::string& returnType, 
                                                              const std::vector<std::string>& params) {
    std::string signature = returnType + " " + methodName;
    return std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, signature);
}

std::unique_ptr<ASTNode> EnhancedJavaFrontend::createImportNode(const std::string& importName) {
    return std::make_unique<ASTNode>(ASTNodeType::IMPORT_DECL, importName);
}

std::string EnhancedJavaFrontend::extractBetween(const std::string& source, const std::string& start, const std::string& end) {
    size_t startPos = source.find(start);
    if (startPos == std::string::npos) return "";
    
    startPos += start.length();
    size_t endPos = source.find(end, startPos);
    if (endPos == std::string::npos) return "";
    
    return source.substr(startPos, endPos - startPos);
}

std::vector<std::string> EnhancedJavaFrontend::splitBy(const std::string& source, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = source.find(delimiter);
    
    while (end != std::string::npos) {
        result.push_back(source.substr(start, end - start));
        start = end + delimiter.length();
        end = source.find(delimiter, start);
    }
    
    result.push_back(source.substr(start));
    return result;
}

std::string EnhancedJavaFrontend::trim(const std::string& str) {
    const std::string whitespace = " \t\n\r";
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

#ifdef ANTLR_AVAILABLE
JavaASTVisitor::JavaASTVisitor() {
    ast_ = std::make_unique<ASTNode>(ASTNodeType::PROGRAM, "JavaProgram");
}

std::unique_ptr<ASTNode> JavaASTVisitor::getAST() {
    return ast_ ? std::make_unique<ASTNode>(*ast_) : nullptr;
}

antlrcpp::Any JavaASTVisitor::visitCompilationUnit(JavaParser::CompilationUnitContext* ctx) {
    // Visit children
    for (auto child : ctx->children) {
        visit(child);
    }
    return nullptr;
}

antlrcpp::Any JavaASTVisitor::visitClassDeclaration(JavaParser::ClassDeclarationContext* ctx) {
    std::string className = ctx->IDENTIFIER()->getText();
    auto classNode = std::make_unique<ASTNode>(ASTNodeType::CLASS_DECL, className);
    
    // Visit class body
    if (ctx->classBody()) {
        for (auto member : ctx->classBody()->classBodyDeclaration()) {
            if (member->methodDeclaration()) {
                auto methodNode = visitMethodDeclaration(member->methodDeclaration()).as<std::unique_ptr<ASTNode>>();
                if (methodNode) {
                    classNode->addChild(std::move(methodNode));
                }
            }
        }
    }
    
    ast_->addChild(std::move(classNode));
    return nullptr;
}

antlrcpp::Any JavaASTVisitor::visitMethodDeclaration(JavaParser::MethodDeclarationContext* ctx) {
    std::string returnType = ctx->typeTypeOrVoid()->getText();
    std::string methodName = ctx->IDENTIFIER()->getText();
    
    std::string signature = returnType + " " + methodName;
    auto methodNode = std::make_unique<ASTNode>(ASTNodeType::FUNCTION_DECL, signature);
    
    return methodNode;
}

antlrcpp::Any JavaASTVisitor::visitImportDeclaration(JavaParser::ImportDeclarationContext* ctx) {
    std::string importName = ctx->qualifiedName()->getText();
    auto importNode = std::make_unique<ASTNode>(ASTNodeType::IMPORT_DECL, importName);
    ast_->addChild(std::move(importNode));
    return nullptr;
}
#endif
