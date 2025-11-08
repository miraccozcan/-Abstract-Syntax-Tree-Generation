///////////////////////////////////////////////////////////////////////////
// File Name: semantic_stack.cpp
// Group Number: 2
// Group Members Names: Mirac Ozcan, Sidhant Sharma, Arvin, Paschal
// Group Members Seneca Email: mozkan1@myseneca.ca, ssharma471@myseneca.ca, aarmand1@myseneca.ca, Pibeh@myseneca.ca
// Date: November 8, 2025
// Authenticity Declaration:
// I declare this submission is the result of our group work and has not
// been shared with any other groups/students or 3rd party content provider.
///////////////////////////////////////////////////////////////////////////

#include "semantic_stack.hpp"

#include <utility>

namespace ast {

void SemanticStack::push(const NodePtr& node) {
    storage.push_back(node);
}

SemanticStack::NodePtr SemanticStack::pop() {
    if (storage.empty()) {
        throw std::runtime_error("Attempted to pop from an empty semantic stack");
    }
    auto node = storage.back();
    storage.pop_back();
    return node;
}

SemanticStack::NodePtr& SemanticStack::peek() {
    if (storage.empty()) {
        throw std::runtime_error("Attempted to peek an empty semantic stack");
    }
    return storage.back();
}

const SemanticStack::NodePtr& SemanticStack::peek() const {
    if (storage.empty()) {
        throw std::runtime_error("Attempted to peek an empty semantic stack");
    }
    return storage.back();
}

bool SemanticStack::empty() const noexcept {
    return storage.empty();
}

std::size_t SemanticStack::size() const noexcept {
    return storage.size();
}

SemanticStack::NodeGuard::NodeGuard(SemanticStack& stack, NodePtr nodePtr)
    : owner(&stack), node(std::move(nodePtr)), active(true) {
    if (!node) {
        throw std::invalid_argument("Cannot create NodeGuard with null AST node");
    }
    owner->push(node);
}

SemanticStack::NodeGuard::~NodeGuard() {
    if (!active || owner == nullptr) {
        return;
    }

    auto child = owner->pop();
    if (child && !owner->empty()) {
        auto& parent = owner->peek();
        if (parent) {
            parent->addChild(child);
        }
    }
}

SemanticStack::NodeGuard::NodeGuard(NodeGuard&& other) noexcept
    : owner(other.owner), node(std::move(other.node)), active(other.active) {
    other.dismiss();
}

SemanticStack::NodeGuard& SemanticStack::NodeGuard::operator=(NodeGuard&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    if (active && owner != nullptr) {
        auto child = owner->pop();
        if (child && !owner->empty()) {
            auto& parent = owner->peek();
            if (parent) {
                parent->addChild(child);
            }
        }
    }

    owner = other.owner;
    node = std::move(other.node);
    active = other.active;
    other.dismiss();

    return *this;
}

SemanticStack::NodePtr SemanticStack::NodeGuard::release() {
    if (!active || owner == nullptr) {
        return node;
    }

    active = false;
    auto heldNode = owner->pop();
    dismiss();
    return heldNode;
}

void SemanticStack::NodeGuard::dismiss() noexcept {
    owner = nullptr;
    active = false;
}

SemanticStack::NodeGuard SemanticStack::makeGuard(NodePtr node) {
    return NodeGuard(*this, std::move(node));
}

namespace example {

ASTNode::Ptr makeBinaryExprTree() {
    SemanticStack stack;
    auto root = std::make_shared<ASTNode>("AddExpr");
    stack.push(root);

    stack.peek()->addChild(std::make_shared<ASTNode>("IntLiteral", std::string("1")));
    stack.peek()->addChild(std::make_shared<ASTNode>("Plus"));
    stack.peek()->addChild(std::make_shared<ASTNode>("IntLiteral", std::string("2")));

    stack.pop();
    return root;
}

ASTNode::Ptr makeNodeGuardTree() {
    SemanticStack stack;
    auto root = std::make_shared<ASTNode>("CallExpr");
    stack.push(root);

    {
        auto guard = stack.makeGuard(std::make_shared<ASTNode>("Arguments"));
        guard.get()->addChild(std::make_shared<ASTNode>("IntLiteral", std::string("42")));
        guard.get()->addChild(std::make_shared<ASTNode>("IntLiteral", std::string("7")));
        // guard destructor attaches "Arguments" to "CallExpr".
    }

    stack.pop();
    return root;
}

}  // namespace example

}  // namespace ast
