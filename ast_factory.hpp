#pragma once

#include "ast.hpp"

#include <vector>

namespace ast {

ASTNode::Ptr makeBinary(const std::string& op,
                        ASTNode::Ptr left,
                        ASTNode::Ptr right);

ASTNode::Ptr makeUnary(const std::string& op,
                       ASTNode::Ptr child);

ASTNode::Ptr makeCall(ASTNode::Ptr funcNameNode,
                      const std::vector<ASTNode::Ptr>& argListNodes);

ASTNode::Ptr makeVarDecl(ASTNode::Ptr idNode,
                         ASTNode::Ptr typeNode,
                         ASTNode::Ptr initExprNode = nullptr);

ASTNode::Ptr makeAssign(ASTNode::Ptr lhsNode,
                        ASTNode::Ptr rhsNode);

ASTNode::Ptr makeIf(ASTNode::Ptr condNode,
                    ASTNode::Ptr thenBlockNode,
                    ASTNode::Ptr elseBlockNode = nullptr);

ASTNode::Ptr makeWhile(ASTNode::Ptr condNode,
                       ASTNode::Ptr bodyNode);

ASTNode::Ptr makeFuncDecl(ASTNode::Ptr nameNode,
                          const std::vector<ASTNode::Ptr>& params,
                          ASTNode::Ptr returnTypeNode);

ASTNode::Ptr makeFuncDef(ASTNode::Ptr declNode,
                         ASTNode::Ptr bodyBlockNode);

}  // namespace ast
