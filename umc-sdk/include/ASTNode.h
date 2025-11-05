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
    BLOCK
};

class ASTNode {
public:
    ASTNode(ASTNodeType type, const std::string& value = "");
    virtual ~ASTNode() = default;

    ASTNodeType getType() const { return type_; }
    std::string getValue() const { return value_; }
    void setValue(const std::string& value) { value_ = value; }

    void addChild(std::unique_ptr<ASTNode> child);
    const std::vector<std::unique_ptr<ASTNode>>& getChildren() const { return children_; }

    std::string toString(int indent = 0) const;

protected:
    ASTNodeType type_;
    std::string value_;
    std::vector<std::unique_ptr<ASTNode>> children_;
};

class ProgramNode : public ASTNode {
public:
    ProgramNode() : ASTNode(ASTNodeType::PROGRAM) {}
};

class FunctionDeclNode : public ASTNode {
public:
    FunctionDeclNode(const std::string& name, const std::string& returnType);
    std::string getName() const { return name_; }
    std::string getReturnType() const { return returnType_; }

private:
    std::string name_;
    std::string returnType_;
};

class VariableDeclNode : public ASTNode {
public:
    VariableDeclNode(const std::string& name, const std::string& type);
    std::string getName() const { return name_; }
    std::string getType() const { return type_; }

private:
    std::string name_;
    std::string type_;
};

class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(const std::string& op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right);
    std::string getOperator() const { return op_; }

private:
    std::string op_;
};

#endif // AST_NODE_H
