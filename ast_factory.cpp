#include "ast_factory.hpp"

#include <optional>
#include <utility>

namespace ast {

namespace {

ASTNode::Ptr makeNode(const std::string& type,
                      std::optional<std::string> value = std::nullopt) {
    return std::make_shared<ASTNode>(type, std::move(value));
}

void appendIfPresent(const ASTNode::Ptr& parent, const ASTNode::Ptr& child) {
    if (parent && child) {
        parent->addChild(child);
    }
}

}  // namespace

ASTNode::Ptr makeBinary(const std::string& op,
                        ASTNode::Ptr left,
                        ASTNode::Ptr right) {
    auto node = makeNode("BinaryExpr", op);
    appendIfPresent(node, left);
    appendIfPresent(node, right);
    return node;
}

ASTNode::Ptr makeUnary(const std::string& op,
                       ASTNode::Ptr child) {
    auto node = makeNode("UnaryExpr", op);
    appendIfPresent(node, child);
    return node;
}

ASTNode::Ptr makeCall(ASTNode::Ptr funcNameNode,
                      const std::vector<ASTNode::Ptr>& argListNodes) {
    auto node = makeNode("CallExpr");
    appendIfPresent(node, funcNameNode);
    for (const auto& arg : argListNodes) {
        appendIfPresent(node, arg);
    }
    return node;
}

ASTNode::Ptr makeVarDecl(ASTNode::Ptr idNode,
                         ASTNode::Ptr typeNode,
                         ASTNode::Ptr initExprNode) {
    auto node = makeNode("VarDecl");
    appendIfPresent(node, idNode);
    appendIfPresent(node, typeNode);
    appendIfPresent(node, initExprNode);
    return node;
}

ASTNode::Ptr makeAssign(ASTNode::Ptr lhsNode,
                        ASTNode::Ptr rhsNode) {
    auto node = makeNode("AssignStmt");
    appendIfPresent(node, lhsNode);
    appendIfPresent(node, rhsNode);
    return node;
}

ASTNode::Ptr makeIf(ASTNode::Ptr condNode,
                    ASTNode::Ptr thenBlockNode,
                    ASTNode::Ptr elseBlockNode) {
    auto node = makeNode("IfStmt");
    appendIfPresent(node, condNode);
    appendIfPresent(node, thenBlockNode);
    appendIfPresent(node, elseBlockNode);
    return node;
}

ASTNode::Ptr makeWhile(ASTNode::Ptr condNode,
                       ASTNode::Ptr bodyNode) {
    auto node = makeNode("WhileStmt");
    appendIfPresent(node, condNode);
    appendIfPresent(node, bodyNode);
    return node;
}

ASTNode::Ptr makeFuncDecl(ASTNode::Ptr nameNode,
                          const std::vector<ASTNode::Ptr>& params,
                          ASTNode::Ptr returnTypeNode) {
    auto node = makeNode("FunctionDecl");
    appendIfPresent(node, nameNode);
    for (const auto& param : params) {
        appendIfPresent(node, param);
    }
    appendIfPresent(node, returnTypeNode);
    return node;
}

ASTNode::Ptr makeFuncDef(ASTNode::Ptr declNode,
                         ASTNode::Ptr bodyBlockNode) {
    auto node = makeNode("FunctionDef");
    appendIfPresent(node, declNode);
    appendIfPresent(node, bodyBlockNode);
    return node;
}

}  // namespace ast
