#pragma once

#include "ast.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace ast {

class SemanticStack {
public:
    using NodePtr = ASTNode::Ptr;

    void push(const NodePtr& node);
    NodePtr pop();
    NodePtr& peek();
    const NodePtr& peek() const;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    class NodeGuard {
    public:
        NodeGuard(SemanticStack& stack, NodePtr node);
        ~NodeGuard();
        NodeGuard(const NodeGuard&) = delete;
        NodeGuard& operator=(const NodeGuard&) = delete;
        NodeGuard(NodeGuard&& other) noexcept;
        NodeGuard& operator=(NodeGuard&& other) noexcept;

        [[nodiscard]] const NodePtr& get() const noexcept { return node; }
        NodePtr release();

    private:
        void dismiss() noexcept;

        SemanticStack* owner{nullptr};
        NodePtr node;
        bool active{false};
    };

    NodeGuard makeGuard(NodePtr node);

private:
    std::vector<NodePtr> storage;
};

namespace example {

ASTNode::Ptr makeBinaryExprTree();
ASTNode::Ptr makeNodeGuardTree();

}  // namespace example

}  // namespace ast
