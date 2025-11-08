///////////////////////////////////////////////////////////////////////////
// File Name: ast_printer.h
// Group Number: 2
// Group Members Names: Mirac Ozcan, Sidhant Sharma, Arvin, Paschal
// Group Members Seneca Email: mozkan1@myseneca.ca, ssharma471@myseneca.ca, aarmand1@myseneca.ca, Pibeh@myseneca.ca
// Date: November 8, 2025
// Authenticity Declaration:
// I declare this submission is the result of our group work and has not
// been shared with any other groups/students or 3rd party content provider.
///////////////////////////////////////////////////////////////////////////

#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct ASTNode {
    std::string type;                    // Describes the kind of syntax element.
    std::vector<ASTNode *> children;     // Owns child pointers elsewhere in the compiler.
};

class AST {
public:
    explicit AST(ASTNode *root) : root_(root) {}

    void printText(const std::string &filename) const {
        std::ofstream out(filename);
        if (!out.is_open()) {
            throw std::runtime_error("Unable to open AST text output file: " + filename);
        }

        if (!root_) {
            out << "<empty AST>\n";
            return;
        }

        writeText(*root_, out, 0U);
    }

    void printDot(const std::string &filename) const {
        std::ofstream out(filename);
        if (!out.is_open()) {
            throw std::runtime_error("Unable to open AST DOT output file: " + filename);
        }

        if (!root_) {
            out << "digraph AST {\n}\n";
            return;
        }

        out << "digraph AST {\n";
        out << "  node [shape=box];\n";

        std::unordered_map<const ASTNode *, std::size_t> ids;
        std::size_t nextId = 0U;
        assignIds(*root_, ids, nextId);

        emitDotNodes(*root_, ids, out);
        emitDotEdges(*root_, ids, out);

        out << "}\n";
    }

private:
    static void writeText(const ASTNode &node, std::ofstream &out, std::size_t depth) {
        out << std::string(depth * 2U, ' ') << node.type << '\n';
        for (const auto *child : node.children) {
            if (child) {
                writeText(*child, out, depth + 1U);
            }
        }
    }

    static void assignIds(const ASTNode &node,
                          std::unordered_map<const ASTNode *, std::size_t> &ids,
                          std::size_t &nextId) {
        ids.emplace(&node, nextId++);
        for (const auto *child : node.children) {
            if (child && ids.find(child) == ids.end()) {
                assignIds(*child, ids, nextId);
            }
        }
    }

    static void emitDotNodes(const ASTNode &node,
                             const std::unordered_map<const ASTNode *, std::size_t> &ids,
                             std::ofstream &out) {
        const auto id = ids.at(&node);
        out << "  node" << id << " [label=\"" << escapeLabel(node.type) << "\"];\n";
        for (const auto *child : node.children) {
            if (child) {
                emitDotNodes(*child, ids, out);
            }
        }
    }

    static void emitDotEdges(const ASTNode &node,
                             const std::unordered_map<const ASTNode *, std::size_t> &ids,
                             std::ofstream &out) {
        const auto parentId = ids.at(&node);
        for (const auto *child : node.children) {
            if (child) {
                const auto childId = ids.at(child);
                out << "  node" << parentId << " -> node" << childId << ";\n";
                emitDotEdges(*child, ids, out);
            }
        }
    }

    static std::string escapeLabel(const std::string &label) {
        std::string escaped;
        escaped.reserve(label.size());
        for (const auto ch : label) {
            if (ch == '\\' || ch == '"') {
                escaped.push_back('\\');
            }
            escaped.push_back(ch);
        }
        return escaped;
    }

    ASTNode *root_;
};

#endif
