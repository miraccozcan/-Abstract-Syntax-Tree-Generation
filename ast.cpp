///////////////////////////////////////////////////////////////////////////
// File Name: ast.cpp
// Group Number: 2
// Group Members Names: Mirac Ozcan, Sidhant Sharma, Arvin, Paschal
// Group Members Seneca Email: mozkan1@myseneca.ca, ssharma471@myseneca.ca, aarmand1@myseneca.ca, Pibeh@myseneca.ca
// Date: November 8, 2025
// Authenticity Declaration:
// I declare this submission is the result of our group work and has not
// been shared with any other groups/students or 3rd party content provider.
///////////////////////////////////////////////////////////////////////////

#include "ast.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace ast {

ASTNode::ASTNode(std::string type,
                 std::optional<std::string> value,
                 std::optional<SourceSpan> span)
    : type(std::move(type)), value(std::move(value)), span(std::move(span)) {}

void ASTNode::addChild(const Ptr& child) {
    children.push_back(child);
}

bool ASTNode::isLeaf() const {
    return children.empty();
}

void ASTNode::prettyPrint(int indent) const {
    const std::string padding(static_cast<std::size_t>(indent) * 2U, ' ');
    std::cout << padding << makeLabel();
    if (span) {
        std::cout << " [" << span->lineStart << ':' << span->columnStart << '-' << span->lineEnd << ':'
                  << span->columnEnd << ']';
    }
    std::cout << '\n';
    for (const auto& child : children) {
        if (child) {
            child->prettyPrint(indent + 1);
        } else {
            std::cout << padding << "  (null)\n";
        }
    }
}

std::string ASTNode::makeLabel() const {
    if (value) {
        return type + ": " + *value;
    }
    return type;
}

namespace {

void writeTextImpl(const ASTNode& node, std::ostream& out, int depth) {
    const std::string padding(static_cast<std::size_t>(depth) * 2U, ' ');
    out << padding << node.getType();
    if (const auto& value = node.getValue()) {
        out << ": " << *value;
    }
    out << '\n';

    for (const auto& child : node.getChildren()) {
        if (!child) {
            continue;
        }
        writeTextImpl(*child, out, depth + 1);
    }
}

std::string escapeDotLabel(const std::string& label) {
    std::string escaped;
    escaped.reserve(label.size());
    for (const char ch : label) {
        if (ch == '"' || ch == '\\') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }
    return escaped;
}

using IdMap = std::unordered_map<const ASTNode*, std::size_t>;

std::size_t assignIds(const ASTNode& node, IdMap& ids, std::size_t nextId) {
    ids.emplace(&node, nextId);
    ++nextId;
    for (const auto& child : node.getChildren()) {
        if (child) {
            nextId = assignIds(*child, ids, nextId);
        }
    }
    return nextId;
}

void writeDotImpl(const ASTNode& node, std::ostream& out, const IdMap& ids) {
    const auto nodeId = ids.at(&node);
    std::string label = node.getType();
    if (node.getValue()) {
        label += "\\n" + *node.getValue();
    }
    if (node.getSourceSpan()) {
        const auto& nodeSpan = *node.getSourceSpan();
        label += "\\n[" + std::to_string(nodeSpan.lineStart) + ':' + std::to_string(nodeSpan.columnStart) +
                 '-' + std::to_string(nodeSpan.lineEnd) + ':' + std::to_string(nodeSpan.columnEnd) + ']';
    }
    out << "  node" << nodeId << " [label=\"" << escapeDotLabel(label) << "\"];\n";

    for (const auto& child : node.getChildren()) {
        if (!child) {
            continue;
        }
        const auto childId = ids.at(child.get());
        out << "  node" << nodeId << " -> node" << childId << ";\n";
        writeDotImpl(*child, out, ids);
    }
}

}  // namespace

void writeText(const ASTNode& root, const std::string& filename) {
    std::ofstream out(filename + ".outast");
    if (!out) {
        throw std::runtime_error("Failed to open " + filename + ".outast for writing");
    }
    writeTextImpl(root, out, 0);
}

void writeDot(const ASTNode& root, const std::string& filename) {
    std::ofstream out(filename + ".dot");
    if (!out) {
        throw std::runtime_error("Failed to open " + filename + ".dot for writing");
    }

    out << "digraph AST {\n";
    out << "  rankdir=TB;\n";
    out << "  node [shape=box];\n";

    IdMap ids;
    assignIds(root, ids, 0);
    writeDotImpl(root, out, ids);

    out << "}\n";
}

}  // namespace ast
