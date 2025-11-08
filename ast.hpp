#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ast {

struct SourceSpan {
    int lineStart{0};
    int columnStart{0};
    int lineEnd{0};
    int columnEnd{0};
};

class ASTNode {
public:
    using Ptr = std::shared_ptr<ASTNode>;

    ASTNode(std::string type,
            std::optional<std::string> value = std::nullopt,
            std::optional<SourceSpan> span = std::nullopt);

    void addChild(const Ptr& child);
    [[nodiscard]] bool isLeaf() const;
    void prettyPrint(int indent = 0) const;

    [[nodiscard]] const std::string& getType() const noexcept { return type; }
    [[nodiscard]] const std::optional<std::string>& getValue() const noexcept { return value; }
    [[nodiscard]] const std::optional<SourceSpan>& getSourceSpan() const noexcept { return span; }
    [[nodiscard]] const std::vector<Ptr>& getChildren() const noexcept { return children; }

private:
    [[nodiscard]] std::string makeLabel() const;

    std::string type;
    std::optional<std::string> value;
    std::optional<SourceSpan> span;
    std::vector<Ptr> children;
};

void writeText(const ASTNode& root, const std::string& filename);
void writeDot(const ASTNode& root, const std::string& filename);

}  // namespace ast
