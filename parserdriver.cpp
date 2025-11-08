///////////////////////////////////////////////////////////////////////////
// File Name: parserdriver.cpp
// Group Number: 2
// Group Members Names: Mirac Ozcan, Sidhant Sharma, Arvin, Paschal
// Group Members Seneca Email: mozkan1@myseneca.ca, ssharma471@myseneca.ca, aarmand1@myseneca.ca, Pibeh@myseneca.ca
// Date: November 8, 2025
// Authenticity Declaration:
// I declare this submission is the result of our group work and has not
// been shared with any other groups/students or 3rd party content provider.
///////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

#include "include/lexer.hpp"   // your A1 lexer
#include "ast.hpp"
#include "ast_factory.hpp"
#include "semantic_stack.hpp"
using namespace std;

// ---------- Token -> grammar terminal mapping ----------
// Your A1 token names vs grammar terminals differ in places.
// Map them here so the grammar/table stays as the prof expects.
static string tokToTerm(const Token& t) {
    const string& k = t.type;
    const string& lx = t.lexeme;

    // literals
    if (k == "intnum")   return "intlit";
    if (k == "floatnum") return "floatlit";

    // identifiers / keywords
    if (k == "id")       return "id";
    if (k == "integer")  return "integer";
    if (k == "float")    return "float";
    if (k == "void")     return "void";
    if (k == "class")    return "class";
    if (k == "return")   return "return";
    if (k == "while")    return "while";
    if (k == "if")       return "if";
    if (k == "then")     return "then";
    if (k == "else")     return "else";
    if (k == "function") return "function";
    if (k == "constructor") return "constructor";
    if (k == "attribute") return "attribute";
    if (k == "public")   return "public";
    if (k == "private")  return "private";
    if (k == "read")     return "read";
    if (k == "write")    return "write";
    if (k == "and")      return "and";
    if (k == "or")       return "or";
    if (k == "not")      return "not";
    if (k == "isa")      return "isa";
    if (k == "localvar") return "localvar";

    // operators / punctuation (by A1 token name → grammar spelling)
    if (k == "eq")       return "eq";     // ==
    if (k == "noteq")    return "neq";    // <>  (grammar says 'neq')
    if (k == "lt")       return "lt";
    if (k == "gt")       return "gt";
    if (k == "leq")      return "leq";
    if (k == "geq")      return "geq";
    if (k == "returntype") return "arrow"; // =>
    if (k == "scopeop")  return "sr";     // ::
    if (k == "assign")   return "equal";  // =

    // Single-char tokens mapped to their literal terminals:
    if (k == "openpar")   return "lpar";
    if (k == "closepar")  return "rpar";
    if (k == "opensqbr")  return "lsqbr";
    if (k == "closesqbr") return "rsqbr";
    if (k == "opencubr")  return "lcurbr";
    if (k == "closecubr") return "rcurbr";
    if (k == "semi")      return "semi";
    if (k == "comma")     return "comma";
    if (k == "colon")     return "colon";

    if (k == "plus")  return "plus";
    if (k == "minus") return "minus";
    if (k == "mult")  return "mult";
    if (k == "div")   return "div";

    // comments are ignored by syntax (the grammar doesn’t include them)
    if (k == "inlinecmt" || k == "blockcmt") return "__SKIP__";

    // Any lexical error should be surfaced as a non-matching terminal:
    if (k.rfind("invalid",0)==0 || k=="invalidcomment")
        return "__LEXERR__";

    // end-of-file sentinel (we’ll push $ at the end)
    if (k == "eof") return "$";

    // Fallback: try exact lexeme for symbols like "." if ever used
    if (k == "dot") return "dot";
    if (k == "self") return "id";

    return "__UNKNOWN__";
}

// ---------- CSV parsing table loader ----------
// CSV format: first row = header terminals (first cell empty).
// First column = nonterminals. Each cell = RHS symbol list separated by spaces,
// or "EPSILON", or "#" for empty/no-entry.
// Example cell: "term addOp arithExprTail"  (spaces separate symbols)
struct Table {
    vector<string> terms;           // header terminals (0..M-1)
    vector<string> nonterms;        // row nonterminals (0..N-1)
    // map[row][col] = RHS vector (empty means error).
    vector<vector<vector<string>>> map;
};

namespace {

using ast::ASTNode;
using ast::SemanticStack;

struct ActionFrame {
    string nonterminal;
    vector<string> rhs;
};

struct ChildValue {
    string symbol;
    ASTNode::Ptr node;
};

ASTNode::Ptr childAt(const vector<ChildValue>& children, size_t index) {
    if (index >= children.size()) {
        return nullptr;
    }
    return children[index].node;
}

ASTNode::Ptr findChildBySymbol(const vector<ChildValue>& children,
                               const string& symbol,
                               size_t occurrence = 0) {
    size_t count = 0;
    for (const auto& child : children) {
        if (child.symbol == symbol) {
            if (count == occurrence) {
                return child.node;
            }
            ++count;
        }
    }
    return nullptr;
}

bool isEpsilonSymbol(const string& symbol) {
    return symbol == "EPSILON" || symbol == "&epsilon";
}

string makeActionMarker(int index) {
    return string("@ACTION") + to_string(index);
}

bool isActionMarker(const string& symbol) {
    return symbol.rfind("@ACTION", 0) == 0;
}

int actionIndexFromMarker(const string& marker) {
    return stoi(marker.substr(7));
}

string normalizeTypeName(const string& type) {
    string out;
    out.reserve(type.size());
    for (char ch : type) {
        if (ch != '<' && ch != '>') {
            out.push_back(ch);
        }
    }
    return out;
}

ASTNode::Ptr makeListNode(const string& name,
                          const vector<ChildValue>& children,
                          const vector<string>& rhs) {
    auto list = make_shared<ASTNode>(name);
    for (size_t i = 0; i < children.size(); ++i) {
        const auto& child = children[i];
        if (!child.node) {
            continue;
        }
        if (child.node->getType() == name && (i + 1 == children.size() || rhs[i] == name || rhs[i].rfind("REPT", 0) == 0)) {
            for (const auto& grand : child.node->getChildren()) {
                list->addChild(grand);
            }
        } else {
            list->addChild(child.node);
        }
    }
    return list;
}

void logWarn(const string& message) {
    cerr << "[WARN] " << message << "\n";
}

ASTNode::Ptr makePlaceholder(const string& name,
                             const vector<ChildValue>& children) {
    auto node = make_shared<ASTNode>(name);
    for (const auto& child : children) {
        if (child.node) {
            node->addChild(child.node);
        }
    }
    return node;
}

ASTNode::Ptr makeOperatorNode(const string& opType, const string& lexeme, const Token& tok) {
    ast::SourceSpan span{tok.line, tok.col, tok.line, tok.col + static_cast<int>(lexeme.size())};
    return make_shared<ASTNode>(opType, lexeme, span);
}

ASTNode::Ptr createNodeForTerminal(const string& terminal, const Token& tok) {
    if (terminal == "id") {
        ast::SourceSpan span{tok.line, tok.col, tok.line, tok.col + static_cast<int>(tok.lexeme.size())};
        return make_shared<ASTNode>("Identifier", tok.lexeme, span);
    }
    if (terminal == "intlit" || terminal == "intLit") {
        ast::SourceSpan span{tok.line, tok.col, tok.line, tok.col + static_cast<int>(tok.lexeme.size())};
        return make_shared<ASTNode>("IntLiteral", tok.lexeme, span);
    }
    if (terminal == "floatlit" || terminal == "floatLit") {
        ast::SourceSpan span{tok.line, tok.col, tok.line, tok.col + static_cast<int>(tok.lexeme.size())};
        return make_shared<ASTNode>("FloatLiteral", tok.lexeme, span);
    }
    if (terminal == "plus") {
        return makeOperatorNode("Operator", "+", tok);
    }
    if (terminal == "minus") {
        return makeOperatorNode("Operator", "-", tok);
    }
    if (terminal == "mult" || terminal == "*") {
        return makeOperatorNode("Operator", "*", tok);
    }
    if (terminal == "div" || terminal == "/") {
        return makeOperatorNode("Operator", "/", tok);
    }
    if (terminal == "and") {
        return makeOperatorNode("Operator", "and", tok);
    }
    if (terminal == "or") {
        return makeOperatorNode("Operator", "or", tok);
    }
    if (terminal == "not") {
        return makeOperatorNode("Operator", "not", tok);
    }
    if (terminal == "eq") {
        return makeOperatorNode("Operator", "==", tok);
    }
    if (terminal == "neq") {
        return makeOperatorNode("Operator", "!=", tok);
    }
    if (terminal == "lt") {
        return makeOperatorNode("Operator", "<", tok);
    }
    if (terminal == "gt") {
        return makeOperatorNode("Operator", ">", tok);
    }
    if (terminal == "leq") {
        return makeOperatorNode("Operator", "<=", tok);
    }
    if (terminal == "geq") {
        return makeOperatorNode("Operator", ">=", tok);
    }
    if (terminal == "equal") {
        return makeOperatorNode("Operator", "=", tok);
    }
    if (terminal == "class" || terminal == "function" || terminal == "return" || terminal == "while" ||
        terminal == "if" || terminal == "then" || terminal == "else" || terminal == "read" ||
        terminal == "write" || terminal == "public" || terminal == "private" || terminal == "attribute" ||
        terminal == "constructor" || terminal == "localvar") {
        ast::SourceSpan span{tok.line, tok.col, tok.line, tok.col + static_cast<int>(tok.lexeme.size())};
        return make_shared<ASTNode>("Keyword", tok.lexeme, span);
    }
    if (terminal == "void" || terminal == "integer" || terminal == "float") {
        ast::SourceSpan span{tok.line, tok.col, tok.line, tok.col + static_cast<int>(tok.lexeme.size())};
        return make_shared<ASTNode>("TypeName", tok.lexeme, span);
    }
    return nullptr;
}

ASTNode::Ptr foldBinaryChain(ASTNode::Ptr left,
                             const ASTNode::Ptr& chainNode,
                             const function<ASTNode::Ptr(const string&, ASTNode::Ptr, ASTNode::Ptr)>& makeBinaryFn) {
    if (!left) {
        return nullptr;
    }
    if (!chainNode) {
        return left;
    }
    const auto& chainChildren = chainNode->getChildren();
    if (chainChildren.empty()) {
        return left;
    }
    auto current = left;
    for (size_t i = 0; i + 1 < chainChildren.size(); i += 2) {
        const auto& opNode = chainChildren[i];
        const auto& rhsNode = chainChildren[i + 1];
        if (!opNode || !rhsNode) {
            continue;
        }
        const auto& optValue = opNode->getValue();
        const string opLexeme = optValue ? *optValue : opNode->getType();
        current = makeBinaryFn(opLexeme, current, rhsNode);
    }
    return current;
}

ASTNode::Ptr buildSemanticNode(const ActionFrame& frame, const vector<ChildValue>& children) {
    const string& nt = frame.nonterminal;
    const auto& rhs = frame.rhs;

    // START ::= PROG eof — wrap translation units in Program root.
    if (nt == "START") {
        auto program = make_shared<ASTNode>("Program");
        auto rootList = childAt(children, 0);
        if (rootList) {
            for (const auto& item : rootList->getChildren()) {
                program->addChild(item);
            }
        }
        return program;
    }

    // REPTSTART0 — flatten list of translation units.
    if (nt == "REPTSTART0") {
        auto list = make_shared<ASTNode>("TranslationUnitList");
        for (const auto& child : children) {
            if (child.node) {
                if (child.node->getType() == "TranslationUnitList") {
                    for (const auto& grand : child.node->getChildren()) {
                        list->addChild(grand);
                    }
                } else {
                    list->addChild(child.node);
                }
            }
        }
        return list;
    }

    // CLASSDECLORFUNCDEF ::= CLASSDECL | FUNCDEF — pass through selected child.
    if (nt == "CLASSDECLORFUNCDEF") {
        return childAt(children, 0);
    }

    // REPTCLASSDECL4 — accumulate class members with optional visibility wrapper.
    if (nt == "REPTCLASSDECL4") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("MemberList");
        }
        auto list = make_shared<ASTNode>("MemberList");
        auto vis = findChildBySymbol(children, "VISIBILITY");
        auto member = findChildBySymbol(children, "MEMBERDECL");
        if (member) {
            if (vis) {
                auto wrapper = make_shared<ASTNode>("MemberDecl");
                wrapper->addChild(vis);
                wrapper->addChild(member);
                list->addChild(wrapper);
            } else {
                list->addChild(member);
            }
        }
        if (auto rest = findChildBySymbol(children, "REPTCLASSDECL4")) {
            for (const auto& more : rest->getChildren()) {
                list->addChild(more);
            }
        }
        return list;
    }

    // VISIBILITY ::= public | private — forward keyword node.
    if (nt == "VISIBILITY") {
        return childAt(children, 0);
    }

    // MEMBERDECL ::= MEMBERFUNCDECL | MEMBERVARDECL — forward built member.
    if (nt == "MEMBERDECL") {
        return childAt(children, 0);
    }

    // MEMBERVARDECL ::= attribute id : TYPE ... — build VarDecl with dimensions.
    if (nt == "MEMBERVARDECL") {
        auto idNode = findChildBySymbol(children, "id");
        auto typeNode = findChildBySymbol(children, "TYPE");
        auto var = ast::makeVarDecl(idNode, typeNode);
        if (auto dims = findChildBySymbol(children, "REPTMEMBERVARDECL4")) {
            for (const auto& dim : dims->getChildren()) {
                var->addChild(dim);
            }
        }
        return var;
    }

    // CLASSDECL ::= class id OPTCLASSDECL2 { REPTCLASSDECL4 } ; — assemble ClassDecl.
    if (nt == "CLASSDECL") {
        auto classNode = make_shared<ASTNode>("ClassDecl");
        auto name = findChildBySymbol(children, "id");
        if (name) {
            classNode->addChild(name);
        }
        if (auto inherits = findChildBySymbol(children, "OPTCLASSDECL2")) {
            classNode->addChild(inherits);
        }
        if (auto members = findChildBySymbol(children, "REPTCLASSDECL4")) {
            for (const auto& member : members->getChildren()) {
                classNode->addChild(member);
            }
        }
        return classNode;
    }

    // OPTCLASSDECL2 ::= isa id ... | EPSILON — optional inheritance list.
    if (nt == "OPTCLASSDECL2") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return nullptr;
        }
        auto list = make_shared<ASTNode>("InheritList");
        auto idNode = findChildBySymbol(children, "id");
        if (idNode) {
            list->addChild(idNode);
        }
        if (auto tail = findChildBySymbol(children, "REPTOPTCLASSDECL22")) {
            for (const auto& base : tail->getChildren()) {
                list->addChild(base);
            }
        }
        return list;
    }

    // REPTOPTCLASSDECL22 — gather trailing base classes.
    if (nt == "REPTOPTCLASSDECL22") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("InheritTail");
        }
        auto list = make_shared<ASTNode>("InheritTail");
        if (auto idNode = findChildBySymbol(children, "id")) {
            list->addChild(idNode);
        }
        if (auto rest = findChildBySymbol(children, "REPTOPTCLASSDECL22")) {
            for (const auto& node : rest->getChildren()) {
                list->addChild(node);
            }
        }
        return list;
    }

    // ASSIGNSTAT — synthesize AssignStmt from VARIABLE and EXPR.
    if (nt == "ASSIGNSTAT") {
        auto lhs = findChildBySymbol(children, "VARIABLE");
        auto rhsExpr = findChildBySymbol(children, "EXPR");
        return ast::makeAssign(lhs, rhsExpr);
    }

    // STATEMENT — discriminate among assignment, calls, control flow, and IO.
    if (nt == "STATEMENT") {
        if (!rhs.empty()) {
            const string& first = rhs.front();
            if (first == "ASSIGNSTAT") {
                return childAt(children, 0);
            }
            if (first == "FUNCTIONCALL") {
                auto callStmt = make_shared<ASTNode>("CallStmt");
                if (auto call = childAt(children, 0)) {
                    callStmt->addChild(call);
                }
                return callStmt;
            }
            if (first == "return") {
                auto expr = findChildBySymbol(children, "EXPR");
                auto node = make_shared<ASTNode>("ReturnStmt");
                if (expr) {
                    node->addChild(expr);
                }
                return node;
            }
            if (first == "write") {
                auto expr = findChildBySymbol(children, "EXPR");
                auto node = make_shared<ASTNode>("WriteStmt");
                if (expr) {
                    node->addChild(expr);
                }
                return node;
            }
            if (first == "read") {
                auto varNode = findChildBySymbol(children, "VARIABLE");
                auto node = make_shared<ASTNode>("ReadStmt");
                if (varNode) {
                    node->addChild(varNode);
                }
                return node;
            }
            if (first == "while") {
                auto cond = findChildBySymbol(children, "RELEXPR");
                auto body = findChildBySymbol(children, "STATBLOCK");
                return ast::makeWhile(cond, body);
            }
            if (first == "if") {
                auto cond = findChildBySymbol(children, "RELEXPR");
                auto thenBlock = findChildBySymbol(children, "STATBLOCK", 0);
                auto elseBlock = findChildBySymbol(children, "STATBLOCK", 1);
                return ast::makeIf(cond, thenBlock, elseBlock);
            }
        }
        // Fallback: return first child if no specific case matched
        return childAt(children, 0);
    }

    // STATBLOCK — normalize block forms into Block node.
    if (nt == "STATBLOCK") {
        if (rhs.size() == 1 && rhs.front() == "STATEMENT") {
            return childAt(children, 0);
        }
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("Block");
        }
        auto block = make_shared<ASTNode>("Block");
        auto stmts = findChildBySymbol(children, "REPTSTATBLOCK1");
        if (stmts) {
            for (const auto& stmt : stmts->getChildren()) {
                block->addChild(stmt);
            }
        }
        return block;
    }

    // FUNCBODY ::= { REPTFUNCBODY1 } — materialize Block from local decls/stmts.
    if (nt == "FUNCBODY") {
        auto block = make_shared<ASTNode>("Block");
        auto seq = findChildBySymbol(children, "REPTFUNCBODY1");
        if (seq) {
            for (const auto& stmt : seq->getChildren()) {
                block->addChild(stmt);
            }
        }
        return block;
    }

    // FUNCDEF ::= FUNCHEAD FUNCBODY — pair declaration with body.
    if (nt == "FUNCDEF") {
        auto decl = findChildBySymbol(children, "FUNCHEAD");
        auto body = findChildBySymbol(children, "FUNCBODY");
        return ast::makeFuncDef(decl, body);
    }

    // FUNCHEAD — build FunctionDecl with params and return type.
    if (nt == "FUNCHEAD") {
        auto funcDecl = make_shared<ASTNode>("FunctionDecl");
        
        // Check for scope resolution (Class::method)
        if (auto scope = findChildBySymbol(children, "OPTFUNCHEAD1")) {
            funcDecl->addChild(scope);
        }
        
        // Add function name
        auto nameNode = findChildBySymbol(children, "id", 0);
        if (nameNode) {
            funcDecl->addChild(nameNode);
        }
        
        // Add parameters
        auto paramsNode = findChildBySymbol(children, "FPARAMS");
        if (paramsNode) {
            for (const auto& param : paramsNode->getChildren()) {
                funcDecl->addChild(param);
            }
        }
        
        // Add return type
        auto returnType = findChildBySymbol(children, "RETURNTYPE");
        if (returnType) {
            funcDecl->addChild(returnType);
        }
        
        return funcDecl;
    }

    // Forward simple terminals/nonterminals that already yield AST terminals.
    if (nt == "RETURNTYPE" || nt == "TYPE" || nt == "ASSIGNOP" || nt == "ADDOP" ||
        nt == "MULTOP" || nt == "SIGN" || nt == "RELOP") {
        return childAt(children, 0);
    }

    // EXPR — expression nodes already built by child.
    if (nt == "EXPR") {
        return childAt(children, 0);
    }

    // RELEXPR ::= ARITHEXPR RELOP ARITHEXPR — binary comparison.
    if (nt == "RELEXPR") {
        auto left = findChildBySymbol(children, "ARITHEXPR", 0);
        auto opNode = findChildBySymbol(children, "RELOP");
        auto right = findChildBySymbol(children, "ARITHEXPR", 1);
        const string op = (opNode && opNode->getValue()) ? *opNode->getValue() : (opNode ? opNode->getType() : "");
        return ast::makeBinary(op, left, right);
    }

    // ARITHEXPR ::= TERM RIGHTRECARITHEXPR — fold additive chain.
    if (nt == "ARITHEXPR") {
        auto left = findChildBySymbol(children, "TERM");
        auto tail = findChildBySymbol(children, "RIGHTRECARITHEXPR");
        return foldBinaryChain(left, tail, [](const string& op, ASTNode::Ptr l, ASTNode::Ptr r) {
            return ast::makeBinary(op, l, r);
        });
    }

    // TERM ::= FACTOR RIGHTRECTERM — fold multiplicative chain.
    if (nt == "TERM") {
        auto left = findChildBySymbol(children, "FACTOR");
        auto tail = findChildBySymbol(children, "RIGHTRECTERM");
        return foldBinaryChain(left, tail, [](const string& op, ASTNode::Ptr l, ASTNode::Ptr r) {
            return ast::makeBinary(op, l, r);
        });
    }

    // RIGHTREC* — accumulate operator RHS pairs for folding.
    if (nt == "RIGHTRECARITHEXPR" || nt == "RIGHTRECTERM") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>(nt);
        }
        auto list = make_shared<ASTNode>(nt);
        if (!children.empty() && children[0].node) {
            list->addChild(children[0].node);
        }
        if (children.size() > 1 && children[1].node) {
            list->addChild(children[1].node);
        }
        if (children.size() > 2 && children[2].node) {
            if (children[2].node->getType() == nt) {
                for (const auto& ch : children[2].node->getChildren()) {
                    list->addChild(ch);
                }
            } else {
                list->addChild(children[2].node);
            }
        }
        return list;
    }

    // FACTEXPR — collapse helper nonterminal to the meaningful child node.
    if (nt == "FACTEXPR") {
        if (auto call = findChildBySymbol(children, "FUNCTIONCALL")) {
            return call;
        }
        if (auto variable = findChildBySymbol(children, "VARIABLE")) {
            return variable;
        }
        if (!rhs.empty() && rhs.front() == "lpar") {
            if (auto expr = findChildBySymbol(children, "EXPR")) {
                return expr;
            }
        }
        for (const auto& child : children) {
            if (!child.node) {
                continue;
            }
            if (child.symbol == "lpar" || child.symbol == "rpar") {
                continue;
            }
            return child.node;
        }
        return nullptr;
    }

    // FACTOR — resolve literals, parenthesized, unary, calls, and variables.
    if (nt == "FACTOR") {
        if (rhs.size() == 1) {
            return childAt(children, 0);
        }
        if (rhs.size() == 3 && rhs.front() == "lpar") {
            return findChildBySymbol(children, "ARITHEXPR");
        }
        if (rhs.size() == 2 && rhs.front() == "SIGN") {
            auto signNode = childAt(children, 0);
            auto operand = childAt(children, 1);
            const string sign = (signNode && signNode->getValue()) ? *signNode->getValue() : (signNode ? signNode->getType() : "+");
            return ast::makeUnary(sign, operand);
        }
        if (rhs.size() == 2 && rhs.front() == "not") {
            auto operand = childAt(children, 1);
            return ast::makeUnary("not", operand);
        }
        if (rhs.front() == "FUNCTIONCALL") {
            return childAt(children, 0);
        }
        if (rhs.front() == "VARIABLE") {
            return childAt(children, 0);
        }
        return childAt(children, 0);
    }

    // REPTFUNCBODY1 — sequence of LOCALVARDECLORSTMT nodes.
    if (nt == "REPTFUNCBODY1") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("StmtList");
        }
        auto list = make_shared<ASTNode>("StmtList");
        if (auto item = findChildBySymbol(children, "LOCALVARDECLORSTMT")) {
            list->addChild(item);
        }
        if (auto rest = findChildBySymbol(children, "REPTFUNCBODY1")) {
            for (const auto& stmt : rest->getChildren()) {
                list->addChild(stmt);
            }
        }
        return list;
    }

    // REPTSTATBLOCK1 — sequence of STATEMENT nodes.
    if (nt == "REPTSTATBLOCK1") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("StmtList");
        }
        auto list = make_shared<ASTNode>("StmtList");
        if (auto stmt = findChildBySymbol(children, "STATEMENT")) {
            list->addChild(stmt);
        }
        if (auto rest = findChildBySymbol(children, "REPTSTATBLOCK1")) {
            for (const auto& ch : rest->getChildren()) {
                list->addChild(ch);
            }
        }
        return list;
    }

    // LOCALVARDECLORSTMT — pass through alt (local var or statement).
    if (nt == "LOCALVARDECLORSTMT") {
        return childAt(children, 0);
    }

    // LOCALVARDECL ::= localvar id : TYPE ARRAYOROBJECT — build VarDecl with init/dims.
    if (nt == "LOCALVARDECL") {
        auto idNode = findChildBySymbol(children, "id");
        auto typeNode = findChildBySymbol(children, "TYPE");
        ASTNode::Ptr init;
        if (auto params = findChildBySymbol(children, "APARAMS")) {
            init = make_shared<ASTNode>("InitializerCall");
            for (const auto& arg : params->getChildren()) {
                init->addChild(arg);
            }
        }
        auto varDecl = ast::makeVarDecl(idNode, typeNode, init);
        if (auto dims = findChildBySymbol(children, "REPTLOCALVARDECL4")) {
            for (const auto& dim : dims->getChildren()) {
                varDecl->addChild(dim);
            }
        }
        return varDecl;
    }

    // VARIABLE — compose nested id/indices chain.
    if (nt == "VARIABLE") {
        auto node = make_shared<ASTNode>("Variable");
        auto prefix = findChildBySymbol(children, "REPTVARIABLE0");
        if (prefix) {
            for (const auto& ch : prefix->getChildren()) {
                node->addChild(ch);
            }
        }
        auto idNode = findChildBySymbol(children, "id");
        if (idNode) {
            node->addChild(idNode);
        }
        auto suffix = findChildBySymbol(children, "REPTVARIABLE2");
        if (suffix) {
            for (const auto& ch : suffix->getChildren()) {
                node->addChild(ch);
            }
        }
        return node;
    }

    // FUNCTIONCALL — build CallExpr with optional prefix chain and arguments.
    if (nt == "FUNCTIONCALL") {
        auto call = make_shared<ASTNode>("FunctionCall");
        
        // Add prefix member access chain if present
        if (auto prefix = findChildBySymbol(children, "REPTFUNCTIONCALL0")) {
            for (const auto& nest : prefix->getChildren()) {
                call->addChild(nest);
            }
        }
        
        // Add function name
        auto funcName = findChildBySymbol(children, "id");
        if (funcName) {
            call->addChild(funcName);
        }
        
        // Add arguments
        auto argsNode = findChildBySymbol(children, "APARAMS");
        if (argsNode) {
            for (const auto& arg : argsNode->getChildren()) {
                call->addChild(arg);
            }
        }
        
        return call;
    }

    // FPARAMS — assemble formal parameter list head and tail.
    if (nt == "FPARAMS") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("ParamList");
        }
        auto paramList = make_shared<ASTNode>("ParamList");
        auto idNode = findChildBySymbol(children, "id");
        auto typeNode = findChildBySymbol(children, "TYPE");
        auto param = make_shared<ASTNode>("Param");
        if (idNode) param->addChild(idNode);
        if (typeNode) param->addChild(typeNode);
        if (auto dims = findChildBySymbol(children, "REPTFPARAMS3")) {
            for (const auto& dim : dims->getChildren()) {
                param->addChild(dim);
            }
        }
        paramList->addChild(param);
        if (auto tail = findChildBySymbol(children, "REPTFPARAMS4")) {
            for (const auto& extra : tail->getChildren()) {
                paramList->addChild(extra);
            }
        }
        return paramList;
    }

    // REPTFPARAMS4 — collect additional parameters.
    if (nt == "REPTFPARAMS4") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("ParamTail");
        }
        auto list = make_shared<ASTNode>("ParamTail");
        if (auto param = findChildBySymbol(children, "FPARAMSTAIL")) {
            list->addChild(param);
        }
        if (auto rest = findChildBySymbol(children, "REPTFPARAMS4")) {
            for (const auto& item : rest->getChildren()) {
                list->addChild(item);
            }
        }
        return list;
    }

    // FPARAMSTAIL — comma parameter continuation.
    if (nt == "FPARAMSTAIL") {
        auto param = make_shared<ASTNode>("Param");
        auto idNode = findChildBySymbol(children, "id");
        auto typeNode = findChildBySymbol(children, "TYPE");
        if (idNode) param->addChild(idNode);
        if (typeNode) param->addChild(typeNode);
        if (auto dims = findChildBySymbol(children, "REPTFPARAMSTAIL4")) {
            for (const auto& dim : dims->getChildren()) {
                param->addChild(dim);
            }
        }
        return param;
    }

    // APARAMS — actual argument list (possibly empty).
    if (nt == "APARAMS") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("ArgumentList");
        }
        auto list = make_shared<ASTNode>("ArgumentList");
        if (auto firstArg = findChildBySymbol(children, "EXPR")) {
            list->addChild(firstArg);
        }
        if (auto tail = findChildBySymbol(children, "REPTAPARAMS1")) {
            for (const auto& arg : tail->getChildren()) {
                if (arg && arg->getType() == "ArgumentExpr" && !arg->getChildren().empty()) {
                    list->addChild(arg->getChildren().front());
                } else if (arg) {
                    list->addChild(arg);
                }
            }
        }
        return list;
    }

    // REPTAPARAMS1 — collect additional actual arguments.
    if (nt == "REPTAPARAMS1") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("ArgumentTail");
        }
        auto list = make_shared<ASTNode>("ArgumentTail");
        if (auto tail = findChildBySymbol(children, "APARAMSTAIL")) {
            for (const auto& arg : tail->getChildren()) {
                list->addChild(arg);
            }
        }
        if (auto rest = findChildBySymbol(children, "REPTAPARAMS1")) {
            for (const auto& arg : rest->getChildren()) {
                list->addChild(arg);
            }
        }
        return list;
    }

    // APARAMSTAIL — comma-prefixed argument expression.
    if (nt == "APARAMSTAIL") {
        auto node = make_shared<ASTNode>("ArgumentExpr");
        if (auto expr = findChildBySymbol(children, "EXPR")) {
            node->addChild(expr);
        }
        return node;
    }

    // ARRAYSIZE ::= [ intlit ] | [ ] — dimension specification for arrays.
    if (nt == "ARRAYSIZE") {
        auto dim = make_shared<ASTNode>("ArrayDim");
        if (auto intNode = findChildBySymbol(children, "intlit")) {
            dim->addChild(intNode);
        }
        return dim;
    }

    // MEMBERFUNCDECL — member function declaration in class.
    if (nt == "MEMBERFUNCDECL") {
        auto decl = make_shared<ASTNode>("MemberFuncDecl");
        if (!rhs.empty() && rhs.front() == "constructor") {
            auto ctor = make_shared<ASTNode>("Constructor");
            auto params = findChildBySymbol(children, "FPARAMS");
            if (params) {
                for (const auto& param : params->getChildren()) {
                    ctor->addChild(param);
                }
            }
            return ctor;
        }
        auto nameNode = findChildBySymbol(children, "id");
        auto params = findChildBySymbol(children, "FPARAMS");
        auto returnType = findChildBySymbol(children, "RETURNTYPE");
        if (nameNode) decl->addChild(nameNode);
        if (params) {
            for (const auto& param : params->getChildren()) {
                decl->addChild(param);
            }
        }
        if (returnType) decl->addChild(returnType);
        return decl;
    }

    // INDICE ::= [ ARITHEXPR ] — array index expression.
    if (nt == "INDICE") {
        auto idx = make_shared<ASTNode>("Index");
        if (auto expr = findChildBySymbol(children, "ARITHEXPR")) {
            idx->addChild(expr);
        }
        return idx;
    }

    // IDNEST ::= id REPTIDNEST1 . | id ( APARAMS ) . — member access chain link.
    if (nt == "IDNEST") {
        auto nest = make_shared<ASTNode>("MemberAccess");
        if (auto idNode = findChildBySymbol(children, "id")) {
            nest->addChild(idNode);
        }
        if (auto indices = findChildBySymbol(children, "REPTIDNEST1")) {
            for (const auto& idx : indices->getChildren()) {
                nest->addChild(idx);
            }
        }
        if (auto params = findChildBySymbol(children, "APARAMS")) {
            auto call = make_shared<ASTNode>("MethodCall");
            for (const auto& arg : params->getChildren()) {
                call->addChild(arg);
            }
            nest->addChild(call);
        }
        return nest;
    }

    // REPTMEMBERVARDECL4, REPTLOCALVARDECL4, REPTFPARAMS3, REPTFPARAMSTAIL4 — array dimension lists.
    if (nt == "REPTMEMBERVARDECL4" || nt == "REPTLOCALVARDECL4" || 
        nt == "REPTFPARAMS3" || nt == "REPTFPARAMSTAIL4") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("DimList");
        }
        auto list = make_shared<ASTNode>("DimList");
        if (auto dim = findChildBySymbol(children, "ARRAYSIZE")) {
            list->addChild(dim);
        }
        if (auto rest = findChildBySymbol(children, nt)) {
            for (const auto& d : rest->getChildren()) {
                list->addChild(d);
            }
        }
        return list;
    }

    // REPTIDNEST1 ::= INDICE REPTIDNEST1 | EPSILON — indices on nested member.
    if (nt == "REPTIDNEST1") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("IndiceList");
        }
        auto list = make_shared<ASTNode>("IndiceList");
        if (auto idx = findChildBySymbol(children, "INDICE")) {
            list->addChild(idx);
        }
        if (auto rest = findChildBySymbol(children, "REPTIDNEST1")) {
            for (const auto& i : rest->getChildren()) {
                list->addChild(i);
            }
        }
        return list;
    }

    // REPTVARIABLE0 ::= IDNEST REPTVARIABLE0 | EPSILON — prefix member access chain.
    if (nt == "REPTVARIABLE0") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("PrefixChain");
        }
        auto list = make_shared<ASTNode>("PrefixChain");
        if (auto nest = findChildBySymbol(children, "IDNEST")) {
            list->addChild(nest);
        }
        if (auto rest = findChildBySymbol(children, "REPTVARIABLE0")) {
            for (const auto& ch : rest->getChildren()) {
                list->addChild(ch);
            }
        }
        return list;
    }

    // REPTVARIABLE2 ::= INDICE REPTVARIABLE2 | EPSILON — suffix indices on variable.
    if (nt == "REPTVARIABLE2") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("SuffixIndices");
        }
        auto list = make_shared<ASTNode>("SuffixIndices");
        if (auto idx = findChildBySymbol(children, "INDICE")) {
            list->addChild(idx);
        }
        if (auto rest = findChildBySymbol(children, "REPTVARIABLE2")) {
            for (const auto& i : rest->getChildren()) {
                list->addChild(i);
            }
        }
        return list;
    }

    // REPTFUNCTIONCALL0 ::= IDNEST REPTFUNCTIONCALL0 | EPSILON — prefix member access for calls.
    if (nt == "REPTFUNCTIONCALL0") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>("CallPrefix");
        }
        auto list = make_shared<ASTNode>("CallPrefix");
        if (auto nest = findChildBySymbol(children, "IDNEST")) {
            list->addChild(nest);
        }
        if (auto rest = findChildBySymbol(children, "REPTFUNCTIONCALL0")) {
            for (const auto& ch : rest->getChildren()) {
                list->addChild(ch);
            }
        }
        return list;
    }

    // OPTFUNCHEAD1 ::= id sr | EPSILON — optional class scope qualifier.
    if (nt == "OPTFUNCHEAD1") {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return nullptr;
        }
        if (auto idNode = findChildBySymbol(children, "id")) {
            auto scope = make_shared<ASTNode>("ScopeResolution");
            scope->addChild(idNode);
            return scope;
        }
        return nullptr;
    }

    // Generic REPT* fallback — build homogeneous list node.
    if (nt.rfind("REPT", 0) == 0) {
        if (rhs.size() == 1 && isEpsilonSymbol(rhs.front())) {
            return make_shared<ASTNode>(nt);
        }
        return makeListNode(nt, children, rhs);
    }

    // Generic OPT* fallback — return optional child or null.
    if (nt.rfind("OPT", 0) == 0) {
        if (children.empty()) {
            return nullptr;
        }
        return childAt(children, 0);
    }

    cerr << "[ERROR] Unknown nonterminal " << nt << "\n";
    return makePlaceholder(nt, children);
}

}  // namespace

static vector<string> splitWS(const string& s) {
    vector<string> out;
    string cur; istringstream is(s);
    while (is >> cur) out.push_back(cur);
    return out;
}

static string trimAscii(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

static string sanitizeTableCell(string val) {
    const string arrow = u8"→";
    const string nbsp = "\xC2\xA0";

    size_t pos = 0;
    while ((pos = val.find(nbsp, pos)) != string::npos) {
        val.replace(pos, nbsp.size(), " ");
    }
    for (char& ch : val) {
        if (ch == '\r' || ch == '\t') {
            ch = ' ';
        }
    }

    size_t arrowPos = val.find(arrow);
    if (arrowPos != string::npos) {
        val = val.substr(arrowPos + arrow.size());
    }

    val = trimAscii(val);
    return val;
}

static vector<string> splitCsvLine(const string& line) {
    vector<string> cells;
    string cell;
    stringstream ss(line);
    while (getline(ss, cell, ',')) {
        if (!cell.empty() && cell.front() == '"' && cell.back() != '"') {
            string tail;
            bool closed = false;
            while (getline(ss, tail, ',')) {
                cell += ',' + tail;
                if (!tail.empty() && tail.back() == '"') {
                    closed = true;
                    break;
                }
            }
            if (!closed) {
                throw runtime_error("Malformed CSV line: " + line);
            }
        }
        cells.push_back(cell);
    }
    return cells;
}

static Table loadTable(const string& path) {
    Table T;
    ifstream in(path);
    if (!in) { throw runtime_error("Cannot open parsing_table.csv"); }
    string line;

    if (!getline(in, line)) throw runtime_error("Empty parsing_table.csv");
    vector<string> headerCells = splitCsvLine(line);
    if (!headerCells.empty() && !headerCells.front().empty()) {
        headerCells.insert(headerCells.begin(), "");
    }
    for (size_t j = 1; j < headerCells.size(); ++j) {
        string term = sanitizeTableCell(headerCells[j]);
        T.terms.push_back(term);
    }
    while (!T.terms.empty() && T.terms.back().empty()) {
        T.terms.pop_back();
    }

    const size_t expectedColumns = T.terms.size() + 1; // include nonterminal column

    while (getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        vector<string> cells = splitCsvLine(line);
        if (cells.empty()) {
            continue;
        }
        if (cells.size() < expectedColumns) {
            cells.resize(expectedColumns);
        }

        string A = sanitizeTableCell(cells[0]);
        if (A.empty()) {
            continue;
        }
        T.nonterms.push_back(A); // Store nonterminal

        vector<vector<string>> row(T.terms.size());
        for (size_t j = 1; j < cells.size() && (j - 1) < T.terms.size(); ++j) {
            string val = sanitizeTableCell(cells[j]);
            if (val == "#" || val.empty()) {
                row[j - 1] = {};
            } else if (val == "EPSILON" || val == "&epsilon") {
                row[j - 1] = {"EPSILON"};
            } else {
                row[j - 1] = splitWS(val);
            }
        }
        if (A == "START") {
            cerr << "[DEBUG] START row entries:\n";
            for (size_t idx = 0; idx < row.size(); ++idx) {
                if (!row[idx].empty()) {
                    cerr << "  term " << T.terms[idx] << " -> ";
                    for (const auto& sym : row[idx]) {
                        cerr << sym << ' ';
                    }
                    cerr << "\n";
                }
            }
        }
        row.resize(T.terms.size());
        T.map.push_back(std::move(row));
    }
    return T;
}

struct ParseOutcome {
    bool hadErrors = false;
    filesystem::path sourcePath;
    filesystem::path derivationPath;
    filesystem::path errorPath;
    filesystem::path astTextPath;
    filesystem::path astDotPath;
    filesystem::path tokensPath;
};

static bool hasWildcard(const string& spec) {
    return spec.find('*') != string::npos || spec.find('?') != string::npos;
}

static bool matchPattern(const string& pattern, const string& value) {
    size_t p = 0, v = 0;
    size_t star = string::npos, match = 0;
    while (v < value.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == value[v])) {
            ++p;
            ++v;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star = p++;
            match = v;
        } else if (star != string::npos) {
            p = star + 1;
            v = ++match;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == '*') {
        ++p;
    }
    return p == pattern.size();
}

static bool isSourceFile(const filesystem::path& p) {
    const string ext = p.extension().string();
    return ext == ".src" || ext == ".source";
}

static vector<filesystem::path> expandInputSpec(const string& spec) {
    namespace fs = filesystem;
    vector<fs::path> results;
    auto addIfSource = [&](const fs::path& candidate) {
        if (fs::is_regular_file(candidate) && isSourceFile(candidate)) {
            results.push_back(fs::weakly_canonical(candidate));
        }
    };

    if (hasWildcard(spec)) {
        fs::path patternPath(spec);
        fs::path dir = patternPath.parent_path();
        if (dir.empty()) {
            dir = fs::path(".");
        }
        const string namePattern = patternPath.filename().string();
        error_code ec;
        fs::directory_iterator it(dir, ec);
        if (ec) {
            return results;
        }
        for (const auto& entry : it) {
            const auto filename = entry.path().filename().string();
            if (entry.is_regular_file() && matchPattern(namePattern, filename) && isSourceFile(entry.path())) {
                results.push_back(fs::weakly_canonical(entry.path()));
            }
        }
        return results;
    }

    fs::path pathSpec(spec);
    error_code ec;
    const fs::file_status status = fs::status(pathSpec, ec);
    if (ec) {
        return results;
    }
    if (fs::is_directory(status)) {
        for (const auto& entry : fs::directory_iterator(pathSpec)) {
            addIfSource(entry.path());
        }
    } else if (fs::is_regular_file(status)) {
        addIfSource(pathSpec);
    }
    return results;
}

static int termIndex(const Table& T, const string& a) {
    for (int j=0;j<(int)T.terms.size();++j) if (T.terms[j]==a) return j;
    return -1;
}
static int nontermIndex(const Table& T, const string& A) {
    for (int i=0;i<(int)T.nonterms.size();++i) if (T.nonterms[i]==A) return i;
    return -1;
}

// ---------- Derivation pretty-printer ----------
static string join(const vector<string>& v, const string& sep=" ") {
    string out;
    for (size_t i=0;i<v.size();++i) {
        if (i) out += sep;
        out += v[i];
    }
    return out;
}

// Replace leftmost nonterminal in α A β with α γ β
static bool looksLikeNonterminal(const string& sym) {
    if (sym.empty()) {
        return false;
    }
    if (sym.front() == '<' && sym.back() == '>') {
        return true;
    }
    return static_cast<bool>(isupper(static_cast<unsigned char>(sym.front())));
}

static void applyLeftmost(vector<string>& sentential, const vector<string>& rhs) {
    for (size_t i=0;i<sentential.size();++i) {
        const string& sym = sentential[i];
        if (looksLikeNonterminal(sym)) {
            vector<string> newSentential;
            newSentential.insert(newSentential.end(), sentential.begin(), sentential.begin()+i);
            if (!(rhs.size()==1 && rhs[0]=="EPSILON")) {
                newSentential.insert(newSentential.end(), rhs.begin(), rhs.end());
            }
            newSentential.insert(newSentential.end(), sentential.begin()+i+1, sentential.end());
            sentential.swap(newSentential);
            return;
        }
    }
}

// ---------- Parser driver ----------
static ParseOutcome parseOneFile(const string& pathSrc,
                                 const Table& PT,
                                 const string& startSym = "START") {
    ParseOutcome outcome;
    outcome.sourcePath = filesystem::path(pathSrc);

    ifstream in(pathSrc);
    if (!in) {
        cerr << "Cannot open: " << pathSrc << "\n";
        outcome.hadErrors = true;
        return outcome;
    }
    string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

    auto baseName = [](const string& p){
        size_t s = p.find_last_of("/\\");
        string n = (s==string::npos)? p : p.substr(s+1);
        size_t d = n.find_last_of('.');
        if (d!=string::npos) n = n.substr(0,d);
        return n;
    };
    string base = baseName(pathSrc);
    filesystem::path srcPath(pathSrc);
    filesystem::path outputDir = srcPath.has_parent_path() ? srcPath.parent_path()
                                                           : filesystem::path(".");

    filesystem::create_directories("outputs");
    outcome.derivationPath = filesystem::path("outputs") / (base + ".outderivation");
    outcome.errorPath = filesystem::path("outputs") / (base + ".outsyntaxerrors");

    ofstream deriv(outcome.derivationPath);
    ofstream errs(outcome.errorPath);
    if (!deriv || !errs) {
        cerr << "Cannot write outputs for " << pathSrc << "\n";
        outcome.hadErrors = true;
        return outcome;
    }

    auto emitError = [&](const auto&... parts) -> std::ostream& {
        outcome.hadErrors = true;
        ((errs << parts), ...);
        return errs;
    };

    Lexer lx; lx.src = std::move(content);

    struct PTok { string term; Token tok; };
    vector<PTok> input;
    vector<Token> allTokens;

    while (true) {
        Token t = lx.nextToken();
        allTokens.push_back(t);
        string a = tokToTerm(t);
        if (a == "__SKIP__") {
            continue;
        }
        if (a == "__LEXERR__") {
            emitError("Syntax error due to lexical error at line ", t.line,
                      " near \"", t.lexeme, "\".\n");
            continue;
        }
        if (t.type == "eof") {
            input.push_back({"$", t});
            break;
        }
        if (a == "__UNKNOWN__") {
            emitError("Unknown token \"", t.type, "\" at line ", t.line, ".\n");
            continue;
        }
        input.push_back({a, t});
    }

    outcome.tokensPath = outputDir / (base + ".outtokens");
    {
        ofstream tokOut(outcome.tokensPath);
        if (!tokOut) {
            emitError("Failed to write token stream output to ", outcome.tokensPath, "\n");
        } else {
            for (const auto& tok : allTokens) {
                tokOut << tok.line << ':' << tok.col << ' ' << tok.type;
                if (!tok.lexeme.empty()) {
                    tokOut << " \"" << tok.lexeme << "\"";
                }
                tokOut << " [" << tokToTerm(tok) << "]\n";
            }
        }
    }

    vector<string> stack; stack.push_back("$"); stack.push_back(startSym);

    SemanticStack semStack;
    vector<ActionFrame> actionFrames;

    vector<string> sentential = { startSym };
    deriv << join(sentential) << "\n";

    size_t ip = 0;
    auto isNonTerm = [&](const string& s){ return nontermIndex(PT, s) >= 0; };

    auto performSemanticFrame = [&](const ActionFrame& frame) {
        vector<ChildValue> childValues;
        bool underflow = false;
        for (auto it = frame.rhs.rbegin(); it != frame.rhs.rend(); ++it) {
            if (isEpsilonSymbol(*it)) {
                continue;
            }
            if (semStack.empty()) {
                underflow = true;
                break;
            }
            childValues.push_back({*it, semStack.pop()});
        }
        if (underflow) {
            emitError("Semantic stack underflow while processing ", frame.nonterminal, ".\n");
            semStack.push(nullptr);
            return;
        }
        reverse(childValues.begin(), childValues.end());
        auto result = buildSemanticNode(frame, childValues);
        semStack.push(result);
    };

    while (!stack.empty()) {
        string X = stack.back(); stack.pop_back();

        if (isActionMarker(X)) {
            const int idx = actionIndexFromMarker(X);
            if (idx >= 0 && idx < static_cast<int>(actionFrames.size())) {
                performSemanticFrame(actionFrames[idx]);
            }
            continue;
        }

        const string a = (ip < input.size() ? input[ip].term : "$");

        if (X == "$") {
            if (a != "$") {
                if (ip < input.size()) {
                    emitError("Extra tokens at end near line ", input[ip].tok.line, ".\n");
                }
            }
            break;
        }

        if (!isNonTerm(X)) {
            if (X == a) {
                if (ip < input.size()) {
                    semStack.push(createNodeForTerminal(X, input[ip].tok));
                    ip++;
                }
            } else {
                if (ip < input.size()) {
                    emitError("Syntax error: expected \"", X, "\" but found \"", a,
                               "\" at line ", input[ip].tok.line, ".\n");
                    semStack.push(nullptr);
                    ip++;
                } else {
                    semStack.push(nullptr);
                }
            }
            continue;
        }

        int i = nontermIndex(PT, X);
        int j = termIndex(PT, a);
        
        // Apply heuristics BEFORE checking table
        vector<string> rhs;
        bool hasHeuristic = false;
        
        if (X == "FACTOR" && a == "id") {
            // CRITICAL FIX: Grammar says FACTOR → id FACTOR2 REPTVARIABLEORFUNCTIONCALL
            // but those nonterminals don't exist in the table!
            // Table has VARIABLE and FUNCTIONCALL instead.
            // Decide between them based on lookahead:
            // - FUNCTIONCALL if we see: id ( or id.id( or id[...].id(
            // - VARIABLE otherwise
            
            // Scan ahead to check the pattern
            size_t look = ip + 1; // after the id
            
            // Skip array indices
            while (look < input.size() && input[look].term == "lsqbr") {
                int depth = 1;
                look++;
                while (look < input.size() && depth > 0) {
                    if (input[look].term == "lsqbr") depth++;
                    else if (input[look].term == "rsqbr") depth--;
                    look++;
                }
            }
            
            bool isFunctionCall = false;
            if (look < input.size()) {
                if (input[look].term == "lpar") {
                    // Simple function call: id(...)
                    isFunctionCall = true;
                } else if (input[look].term == "dot") {
                    // Member access: id.something
                    // Check if it leads to a function call
                    look++; // skip dot
                    if (look < input.size() && input[look].term == "id") {
                        look++; // skip id
                        // Skip more array indices after the member
                        while (look < input.size() && input[look].term == "lsqbr") {
                            int depth = 1;
                            look++;
                            while (look < input.size() && depth > 0) {
                                if (input[look].term == "lsqbr") depth++;
                                else if (input[look].term == "rsqbr") depth--;
                                look++;
                            }
                        }
                        if (look < input.size() && input[look].term == "lpar") {
                            // Member function call: id.member(...)
                            isFunctionCall = true;
                        }
                    }
                }
            }
            
            if (isFunctionCall) {
                rhs = {"FUNCTIONCALL"};
            } else {
                rhs = {"VARIABLE"};
            }
            hasHeuristic = true;
        } else if (X == "ARRAYSIZE" && a == "lsqbr") {
            // ARRAYSIZE → [ ARRAYSIZE2, where ARRAYSIZE2 → intlit ] | ]
            // Check if there's an intlit after the lsqbr
            const string afterLsqbr = (ip + 1 < input.size() ? input[ip + 1].term : "$");
            if (afterLsqbr == "intlit") {
                rhs = {"lsqbr", "intlit", "rsqbr"};
            } else {
                // Empty array: []
                rhs = {"lsqbr", "rsqbr"};
            }
            hasHeuristic = true;
        } else if (X == "STATEMENT" && a == "id") {
            // Table says STATEMENT → ASSIGNSTAT semi (for assignments)
            // But need to handle function call statements too
            // Check lookahead pattern to decide
            const string next = (ip + 1 < input.size() ? input[ip + 1].term : "$");
            
            if (next == "lpar") {
                // Simple function call: id(...)
                rhs = {"FUNCTIONCALL", "semi"};
                hasHeuristic = true;
            } else if (next == "dot") {
                // Member access - could be assignment (self.a = ...) or function call (obj.method(...))
                // Scan ahead to determine
                size_t look = ip + 2; // after id and dot
                if (look < input.size() && input[look].term == "id") {
                    look++; // after member id
                    // Skip array indices
                    while (look < input.size() && input[look].term == "lsqbr") {
                        int depth = 1;
                        look++;
                        while (look < input.size() && depth > 0) {
                            if (input[look].term == "lsqbr") depth++;
                            else if (input[look].term == "rsqbr") depth--;
                            look++;
                        }
                    }
                    const string afterMember = (look < input.size() ? input[look].term : "$");
                    if (afterMember == "lpar") {
                        // Member function call: id.member(...)
                        rhs = {"FUNCTIONCALL", "semi"};
                        hasHeuristic = true;
                    } else if (afterMember == "equal") {
                        // Member assignment: id.member = ...
                        // Can't use VARIABLE (doesn't handle member access properly in table)
                        // Inline the pattern: id . id = EXPR semi
                        // (simplified - doesn't handle id.member[idx] or id.member.member)
                        rhs = {"id", "dot", "id", "equal", "EXPR", "semi"};
                        hasHeuristic = true;
                    } else if (afterMember == "dot") {
                        // Chained member access - complex case
                        // For now, use VARIABLE and let it handle
                        rhs = {"VARIABLE", "ASSIGNOP", "EXPR", "semi"};
                        hasHeuristic = true;
                    }
                }
            } else {
                // Simple assignment or array assignment - use table's ASSIGNSTAT
                hasHeuristic = false;
            }
        } else if (X == "REPTVARIABLE0" && a == "id") {
            // REPTVARIABLE0 appears after id in VARIABLE/FUNCTIONCALL
            // Table says: REPTVARIABLE0 → IDNEST REPTVARIABLE0 (with lookahead id)
            // But IDNEST starts with dot, not id!
            // So this table entry is wrong - REPTVARIABLE0 should only match on dot.
            // Force epsilon for any lookahead except dot.
            rhs = {"EPSILON"};
            hasHeuristic = true;
        } else if (X == "REPTVARIABLE0") {
            // For any lookahead that's not dot, take epsilon
            if (a != "dot") {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
            // If lookahead is dot, let table handle it (IDNEST REPTVARIABLE0)
        } else if (X == "REPTVARIABLE2") {
            // REPTVARIABLE2 → INDICE REPTVARIABLE2 | EPSILON
            // Only continue if we see lsqbr
            if (a == "lsqbr") {
                // Let table handle it
                hasHeuristic = false;
            } else {
                // Anything else: epsilon
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "RIGHTRECTERM") {
            // RIGHTRECTERM → MULTOP FACTOR RIGHTRECTERM | EPSILON
            // Only continue if we see a multiplicative operator
            if (a == "mult" || a == "div" || a == "and") {
                hasHeuristic = false; // Let table handle it
            } else {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "RIGHTRECARITHEXPR") {
            // RIGHTRECARITHEXPR → ADDOP TERM RIGHTRECARITHEXPR | EPSILON
            // Only continue if we see an additive operator
            if (a == "plus" || a == "minus" || a == "or") {
                hasHeuristic = false; // Let table handle it
            } else {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "REPTSTATBLOCK1") {
            // REPTSTATBLOCK1 → STATEMENT REPTSTATBLOCK1 | EPSILON
            // Check if lookahead can start a statement
            if (a == "id" || a == "if" || a == "while" || a == "read" || 
                a == "write" || a == "return") {
                hasHeuristic = false; // Let table handle it
            } else {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "REPTFUNCBODY1") {
            // REPTFUNCBODY1 → LOCALVARDECLORSTMT REPTFUNCBODY1 | EPSILON
            // Check if lookahead can start a local var or statement
            if (a == "localvar" || a == "id" || a == "if" || a == "while" || 
                a == "read" || a == "write" || a == "return") {
                hasHeuristic = false; // Let table handle it
            } else {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "REPTSTART0" && a == "$") {
            rhs = {"EPSILON"};
            hasHeuristic = true;
        } else if (X == "IDNEST") {
            // CRITICAL FIX: The parsing table has wrong entry for IDNEST.
            // Table says: IDNEST → id REPTIDNEST1 dot (with lookahead id)
            // Grammar says: IDNEST → dot id IDNEST2
            // We must override the table and use the correct grammar production.
            if (a == "dot") {
                // Check what follows: dot id ...
                const string afterDot = (ip + 1 < input.size() ? input[ip + 1].term : "$");
                if (afterDot == "id") {
                    const string afterId = (ip + 2 < input.size() ? input[ip + 2].term : "$");
                    if (afterId == "lpar") {
                        // dot id ( ... ) - method call
                        rhs = {"dot", "id", "lpar", "APARAMS", "rpar"};
                        hasHeuristic = true;
                    } else {
                        // dot id [array] or dot id - field access
                        rhs = {"dot", "id", "REPTIDNEST1"};
                        hasHeuristic = true;
                    }
                }
            }
            // If lookahead is not dot, or pattern doesn't match, reject (no heuristic)
        } else if (X == "REPTVARIABLEORFUNCTIONCALL") {
            // REPTVARIABLEORFUNCTIONCALL → IDNEST REPTVARIABLEORFUNCTIONCALL | EPSILON
            // IDNEST starts with dot, so only continue if we see dot
            if (a == "dot") {
                hasHeuristic = false; // Let table handle it
            } else {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "REPTFUNCTIONCALL0" && a == "id") {
            bool isIdnest = false;
            size_t look = ip + 1;
            while (look < input.size() && input[look].term == "lsqbr") {
                int depth = 1;
                look++;
                while (look < input.size() && depth > 0) {
                    if (input[look].term == "lsqbr") depth++;
                    else if (input[look].term == "rsqbr") depth--;
                    look++;
                }
            }
            if (look < input.size()) {
                if (input[look].term == "dot") {
                    isIdnest = true;
                } else if (input[look].term == "lpar") {
                    int depth = 1;
                    look++;
                    while (look < input.size() && depth > 0) {
                        if (input[look].term == "lpar") depth++;
                        else if (input[look].term == "rpar") depth--;
                        look++;
                    }
                    if (look < input.size() && input[look].term == "dot") {
                        isIdnest = true;
                    }
                }
            }
            if (isIdnest) {
                rhs = {"IDNEST", "REPTFUNCTIONCALL0"};
                hasHeuristic = true;
            } else {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "LOCALVARDECL" && a == "localvar") {
            const string afterType = (ip + 4 < input.size() ? input[ip + 4].term : "$");
            if (afterType == "lpar") {
                rhs = {"localvar", "id", "colon", "TYPE", "lpar", "APARAMS", "rpar", "semi"};
                hasHeuristic = true;
            }
        } else if (X == "OPTFUNCHEAD1" && a == "id") {
            const string next = (ip + 1 < input.size() ? input[ip + 1].term : "$");
            if (next == "sr") {
                rhs = {"id", "sr"};
                hasHeuristic = true;
            } else {
                rhs = {"EPSILON"};
                hasHeuristic = true;
            }
        } else if (X == "FUNCHEAD" && a == "function") {
            const string next1 = (ip + 1 < input.size() ? input[ip + 1].term : "$");
            const string next2 = (ip + 2 < input.size() ? input[ip + 2].term : "$");
            const string next3 = (ip + 3 < input.size() ? input[ip + 3].term : "$");
            if (next1 == "id" && next2 == "sr" && next3 == "constructor") {
                rhs = {"function", "id", "sr", "constructor", "lpar", "FPARAMS", "rpar"};
                hasHeuristic = true;
            }
        }
        
        if (!hasHeuristic) {
            if (i < 0 || j < 0 || PT.map[i][j].empty()) {
                if (ip < input.size()) {
                    emitError("Syntax error: no rule for ", X, " with lookahead \"", a,
                               "\" at line ", input[ip].tok.line, ". Recovering by popping.\n");
                } else {
                    emitError("Syntax error: no rule for ", X, " with lookahead \"", a,
                               "\" at line EOF. Recovering by popping.\n");
                }
                applyLeftmost(sentential, {"EPSILON"});
                deriv << join(sentential) << "\n";
                semStack.push(nullptr);
                continue;
            }
            rhs = PT.map[i][j];
        }
        ActionFrame frame{X, rhs};
        int actionIdx = static_cast<int>(actionFrames.size());
        actionFrames.push_back(frame);
        stack.push_back(makeActionMarker(actionIdx));

        if (!(rhs.size() == 1 && rhs[0] == "EPSILON")) {
            for (int k = static_cast<int>(rhs.size()) - 1; k >= 0; --k) {
                stack.push_back(rhs[k]);
            }
        }

        applyLeftmost(sentential, rhs);
        deriv << join(sentential) << "\n";
    }

    ASTNode::Ptr root;
    if (!semStack.empty()) {
        root = semStack.pop();
    }
    if (!semStack.empty()) {
        emitError("Semantic stack contains ", semStack.size(),
                   " leftover entries after parsing.\n");
    }

    if (root) {
        try {
            const filesystem::path astBasePath = outputDir / base;
            const auto astBaseStr = astBasePath.string();
            ast::writeText(*root, astBaseStr);
            ast::writeDot(*root, astBaseStr);
            outcome.astTextPath = astBasePath;
            outcome.astTextPath += ".outast";
            outcome.astDotPath = astBasePath;
            outcome.astDotPath += ".dot";
        } catch (const exception& ex) {
            emitError("Failed to write AST outputs: ", ex.what(), "\n");
        }
    } else {
        emitError("No AST root generated for this translation unit.\n");
    }

    if (ip + 1 < input.size()) {
        if (ip < input.size()) {
            emitError("Syntax error: unparsed input remains near line ", input[ip].tok.line, ".\n");
        } else {
            emitError("Syntax error: unparsed input remains near line EOF.\n");
        }
    }

    return outcome;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 3) {
        cerr << "Usage: ./parserdriver <parsing_table.csv> <path-or-glob>...\n";
        return 1;
    }

    Table PT;
    try {
        PT = loadTable(argv[1]);
    } catch (const exception& ex) {
        cerr << "Failed to load parsing table: " << ex.what() << "\n";
        return 1;
    }

    if (nontermIndex(PT, "START") < 0) {
        cerr << "[WARN] Parsing table does not define START; defaulting to START with REPTSTART0.\n";
    }
    cerr << "[DEBUG] term index of class = " << termIndex(PT, "class") << "\n";
    cerr << "[DEBUG] nonterm index of START = " << nontermIndex(PT, "START") << "\n";

    vector<filesystem::path> files;
    bool hadMissing = false;
    for (int i = 2; i < argc; ++i) {
        const string spec = argv[i];
        auto expanded = expandInputSpec(spec);
        if (expanded.empty()) {
            cerr << "No source files matched \"" << spec << "\".\n";
            hadMissing = true;
        } else {
            files.insert(files.end(), expanded.begin(), expanded.end());
        }
    }

    if (files.empty()) {
        cerr << "No input source files found.\n";
        return hadMissing ? 2 : 1;
    }

    sort(files.begin(), files.end());
    files.erase(unique(files.begin(), files.end()), files.end());

    bool anyErrors = false;
    for (const auto& file : files) {
        const ParseOutcome outcome = parseOneFile(file.string(), PT);
        if (outcome.hadErrors) {
            anyErrors = true;
            cout << "[FAIL] " << file;
            if (!outcome.errorPath.empty()) {
                cout << " (see " << outcome.errorPath << ')';
            }
            cout << '\n';
        } else {
            cout << "[OK]   " << file;
            if (!outcome.astTextPath.empty() && !outcome.astDotPath.empty()) {
                cout << " -> " << outcome.astTextPath << ", " << outcome.astDotPath;
            }
            if (!outcome.tokensPath.empty()) {
                cout << " (tokens: " << outcome.tokensPath << ')';
            }
            cout << '\n';
        }
    }

    if (anyErrors) {
        return 2;
    }
    if (hadMissing) {
        return 1;
    }
    return 0;
}
