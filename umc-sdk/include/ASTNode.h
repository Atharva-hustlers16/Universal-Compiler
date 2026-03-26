#ifndef AST_NODE_H
#define AST_NODE_H

#include <string>
#include <vector>
#include <memory>

enum class ASTNodeType {
    PROGRAM,
    FUNCTION_DECL,
    VARIABLE_DECL,
    FUNCTION_CALL,
    BINARY_OP,
    LITERAL,
    IDENTIFIER,
    IF_STMT,
    FOR_LOOP,
    WHILE_LOOP,
    RETURN_STMT,
    BLOCK,
    // C++ specific types
    CLASS_DECL,
    CONSTRUCTOR_DECL,
    METHOD_DECL,
    PARAMETER,
    RETURN_TYPE,
    TYPE,
    ARGUMENT
};

class ASTNode {
public:
    ASTNode(ASTNodeType type, const std::string& value = "");
    virtual ~ASTNode() = default;
    
    // Delete copy constructor and copy assignment
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    
    // Allow move constructor and move assignment
    ASTNode(ASTNode&&) = default;
    ASTNode& operator=(ASTNode&&) = default;

    ASTNodeType getType() const { return type_; }
    std::string getValue() const { return value_; }
    void setValue(const std::string& value) { value_ = value; }

    void addChild(std::unique_ptr<ASTNode> child);
    const std::vector<std::unique_ptr<ASTNode>>& getChildren() const { return children_; }
    std::vector<std::unique_ptr<ASTNode>>& getChildren() { return children_; }

    std::string toString(int indent = 0) const;
    
    // Clone method for deep copying
    std::unique_ptr<ASTNode> clone() const;

protected:
    ASTNodeType type_;
    std::string value_;
    std::vector<std::unique_ptr<ASTNode>> children_;
};

// Helper function to get node type name
std::string getNodeTypeName(ASTNodeType type);

// Derived node classes
class ProgramNode : public ASTNode {
public:
    ProgramNode(const std::string& value = "") : ASTNode(ASTNodeType::PROGRAM, value) {}
};

class FunctionDeclNode : public ASTNode {
public:
    FunctionDeclNode(const std::string& name, const std::string& returnType = "")
        : ASTNode(ASTNodeType::FUNCTION_DECL, name), returnType_(returnType) {}
    std::string getName() const { return getValue(); }
    std::string getReturnType() const { return returnType_; }

private:
    std::string returnType_;
};

class VariableDeclNode : public ASTNode {
public:
    VariableDeclNode(const std::string& name, const std::string& varType = "")
        : ASTNode(ASTNodeType::VARIABLE_DECL, name), varType_(varType) {}
    std::string getName() const { return getValue(); }
    std::string getVarType() const { return varType_; }

private:
    std::string varType_;
};

class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(const std::string& op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right);
    std::string getOperator() const { return op_; }

private:
    std::string op_;
};

#endif // AST_NODE_H
