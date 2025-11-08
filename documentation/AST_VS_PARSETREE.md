# AST vs Parse Tree - Detailed Comparison

## Requirement 5.2: The output represents an Abstract Syntax Tree (AST), not a parse tree

This document demonstrates that our parser generates a true **Abstract Syntax Tree (AST)**, not a parse tree.

---

## Example 1: Simple Assignment

### Source Code:
```
x = 5;
```

### Parse Tree (What we DON'T generate):
```
STATEMENT
├── ASSIGNSTAT
│   ├── id (token: x)
│   ├── equal (token: =)
│   ├── EXPR
│   │   ├── ARITHEXPR
│   │   │   ├── TERM
│   │   │   │   └── FACTOR
│   │   │   │       └── intlit (token: 5)
│   │   │   └── RIGHTRECARITHEXPR
│   │   │       └── ε (epsilon)
│   │   └── RELEXPR
│   │       └── ε (epsilon)
│   └── semi (token: ;)
```

**Parse Tree Characteristics**:
- Contains grammar symbols: STATEMENT, ASSIGNSTAT, EXPR, ARITHEXPR, TERM, FACTOR, RELEXPR, RIGHTRECARITHEXPR
- Contains tokens: id, equal, semi, intlit
- Shows epsilon productions (ε)
- 13 nodes total for simple assignment

### AST (What we DO generate):
```
AssignStmt
├── Variable
│   └── Identifier: x
└── IntLiteral: 5
```

**AST Characteristics**:
- Only semantic nodes: AssignStmt, Variable, Identifier, IntLiteral
- No grammar symbols (STATEMENT, ASSIGNSTAT, EXPR, ARITHEXPR, TERM, FACTOR)
- No punctuation tokens (equal, semi)
- No epsilon productions
- 4 nodes total

---

## Example 2: Binary Expression

### Source Code:
```
result = a + b * 2;
```

### Parse Tree (What we DON'T generate):
```
STATEMENT
├── ASSIGNSTAT
│   ├── id (token: result)
│   ├── equal (token: =)
│   ├── EXPR
│   │   ├── ARITHEXPR
│   │   │   ├── TERM
│   │   │   │   └── FACTOR
│   │   │   │       └── VARIABLE
│   │   │   │           ├── id (token: a)
│   │   │   │           └── REPTVARIABLE0
│   │   │   │               └── ε
│   │   │   └── RIGHTRECARITHEXPR
│   │   │       ├── addop (token: +)
│   │   │       ├── TERM
│   │   │       │   ├── FACTOR
│   │   │       │   │   └── VARIABLE
│   │   │       │   │       ├── id (token: b)
│   │   │       │   │       └── REPTVARIABLE0
│   │   │       │   │           └── ε
│   │   │       │   └── RIGHTRECTERM
│   │   │       │       ├── multop (token: *)
│   │   │       │       ├── FACTOR
│   │   │       │       │   └── intlit (token: 2)
│   │   │       │       └── RIGHTRECTERM
│   │   │       │           └── ε
│   │   │       └── RIGHTRECARITHEXPR
│   │   │           └── ε
│   │   └── RELEXPR
│   │       └── ε
│   └── semi (token: ;)
```

**Parse Tree Characteristics**:
- Shows derivation steps through grammar productions
- Includes intermediate nonterminals for operator precedence (TERM for *, ARITHEXPR for +)
- Right-recursive productions (RIGHTRECARITHEXPR, RIGHTRECTERM) shown explicitly
- 35+ nodes

### AST (What we DO generate):
```
AssignStmt
├── Variable
│   └── Identifier: result
└── BinaryExpr: +
    ├── Variable
    │   └── Identifier: a
    └── BinaryExpr: *
        ├── Variable
        │   └── Identifier: b
        └── IntLiteral: 2
```

**AST Characteristics**:
- Direct representation of semantics
- Binary operators embedded in expression nodes (BinaryExpr: +, BinaryExpr: *)
- Correct precedence: * is child of +, so * evaluates first
- No grammar symbols, no tokens
- 9 nodes total

---

## Example 3: Array Access with Complex Index

### Source Code:
```
matrix[i + 1][j] = value;
```

### Parse Tree (What we DON'T generate):
```
STATEMENT
├── ASSIGNSTAT
│   ├── id (token: matrix)
│   ├── REPTVARIABLE2
│   │   ├── lsqbr (token: [)
│   │   ├── ARITHEXPR
│   │   │   ├── TERM
│   │   │   │   └── FACTOR
│   │   │   │       └── VARIABLE
│   │   │   │           ├── id (token: i)
│   │   │   │           └── REPTVARIABLE0
│   │   │   │               └── ε
│   │   │   └── RIGHTRECARITHEXPR
│   │   │       ├── addop (token: +)
│   │   │       ├── TERM
│   │   │       │   └── FACTOR
│   │   │       │       └── intlit (token: 1)
│   │   │       └── RIGHTRECARITHEXPR
│   │   │           └── ε
│   │   ├── rsqbr (token: ])
│   │   └── REPTVARIABLE2
│   │       ├── lsqbr (token: [)
│   │       ├── ARITHEXPR
│   │       │   ├── TERM
│   │       │   │   └── FACTOR
│   │       │   │       └── VARIABLE
│   │       │   │           ├── id (token: j)
│   │       │   │           └── REPTVARIABLE0
│   │       │   │               └── ε
│   │       │   └── RIGHTRECARITHEXPR
│   │       │       └── ε
│   │       ├── rsqbr (token: ])
│   │       └── REPTVARIABLE2
│   │           └── ε
│   ├── equal (token: =)
│   ├── EXPR
│   │   ├── ARITHEXPR
│   │   │   ├── TERM
│   │   │   │   └── FACTOR
│   │   │   │       └── VARIABLE
│   │   │   │           ├── id (token: value)
│   │   │   │           └── REPTVARIABLE0
│   │   │   │               └── ε
│   │   │   └── RIGHTRECARITHEXPR
│   │   │       └── ε
│   │   └── RELEXPR
│   │       └── ε
│   └── semi (token: ;)
```

**Parse Tree Characteristics**:
- REPTVARIABLE2 handles repetition of indices
- Bracket tokens (lsqbr, rsqbr) shown explicitly
- Epsilon productions for empty repetitions
- 45+ nodes

### AST (What we DO generate):
```
AssignStmt
├── Variable
│   ├── Identifier: matrix
│   ├── Index
│   │   └── BinaryExpr: +
│   │       ├── Variable
│   │       │   └── Identifier: i
│   │       └── IntLiteral: 1
│   └── Index
│       └── Variable
│           └── Identifier: j
└── Variable
    └── Identifier: value
```

**AST Characteristics**:
- Index nodes represent array subscripts
- Multiple Index nodes for multi-dimensional arrays
- Index expressions can be complex (BinaryExpr: +)
- No bracket tokens, no REPTVARIABLE2
- 12 nodes total

---

## Example 4: If Statement

### Source Code:
```
if (x == 5) then
    write("yes");
else
    write("no");
end
```

### Parse Tree (What we DON'T generate):
```
STATEMENT
├── if (token: if)
├── lparen (token: ()
├── RELEXPR
│   ├── ARITHEXPR
│   │   ├── TERM
│   │   │   └── FACTOR
│   │   │       └── VARIABLE
│   │   │           ├── id (token: x)
│   │   │           └── REPTVARIABLE0
│   │   │               └── ε
│   │   └── RIGHTRECARITHEXPR
│   │       └── ε
│   ├── relop (token: ==)
│   └── ARITHEXPR
│       ├── TERM
│       │   └── FACTOR
│       │       └── intlit (token: 5)
│       └── RIGHTRECARITHEXPR
│           └── ε
├── rparen (token: ))
├── then (token: then)
├── STATBLOCK
│   ├── STATEMENT
│   │   ├── write (token: write)
│   │   ├── lparen (token: ()
│   │   ├── EXPR
│   │   │   └── stringlit (token: "yes")
│   │   ├── rparen (token: ))
│   │   └── semi (token: ;)
│   └── REPTSTATBLOCK1
│       └── ε
├── else (token: else)
├── STATBLOCK
│   ├── STATEMENT
│   │   ├── write (token: write)
│   │   ├── lparen (token: ()
│   │   ├── EXPR
│   │   │   └── stringlit (token: "no")
│   │   ├── rparen (token: ))
│   │   └── semi (token: ;)
│   └── REPTSTATBLOCK1
│       └── ε
└── end (token: end)
```

**Parse Tree Characteristics**:
- All keywords shown as tokens (if, then, else, end)
- STATBLOCK wraps statement lists
- REPTSTATBLOCK1 for multiple statements
- 40+ nodes

### AST (What we DO generate):
```
IfStmt
├── BinaryExpr: ==
│   ├── Variable
│   │   └── Identifier: x
│   └── IntLiteral: 5
├── Block
│   └── WriteStmt
│       └── StringLiteral: "yes"
└── Block
    └── WriteStmt
        └── StringLiteral: "no"
```

**AST Characteristics**:
- IfStmt node with three children: condition, then-block, else-block
- Block nodes contain statement lists directly (no REPTSTATBLOCK1)
- No keyword tokens (if, then, else, end)
- 11 nodes total

---

## Example 5: Class with Member Function Call

### Source Code:
```
class Point {
    public attribute integer: x;
    public integer compute() => {
        return x;
    }
};

function void test() => {
    local Point: p;
    write(p.compute());
}
```

### Parse Tree (What we DON'T generate):
Extremely verbose with:
- CLASSDECL, MEMBERDECL, REPTMEMBERDECL, VISIBILITY, VARDECL, MEMBERFUNCDECL
- FUNCDEF, FUNCHEAD, FUNCBODY, REPTFUNCBODY1
- STATEMENT, FACTOR, FUNCTIONCALL, APARAMS, REPTAPARAMS1
- VARIABLE, IDNEST, REPTVARIABLE0, REPTIDNEST1
- All punctuation tokens: class, lcurbr, rcurbr, public, attribute, colon, semi, lparen, rparen, dot, etc.
- 100+ nodes

### AST (What we DO generate):
```
Program
├── ClassDecl
│   ├── Identifier: Point
│   ├── MemberDecl
│   │   ├── Keyword: public
│   │   └── VarDecl
│   │       ├── Identifier: x
│   │       └── TypeName: integer
│   └── MemberDecl
│       ├── Keyword: public
│       └── MemberFuncDecl
│           ├── Identifier: compute
│           └── TypeName: integer
└── FunctionDef
    ├── FunctionDecl
    │   ├── Identifier: test
    │   └── TypeName: void
    └── Block
        ├── VarDecl
        │   ├── Identifier: p
        │   └── Identifier: Point
        └── WriteStmt
            └── FunctionCall
                ├── MemberAccess
                │   └── Identifier: p
                ├── Identifier: compute
                └── (no arguments)
```

**AST Characteristics**:
- Semantic structure: Program → ClassDecl/FunctionDef → members/body
- MemberAccess node for member function calls (p.compute)
- No grammar symbols (CLASSDECL, REPTMEMBERDECL, FUNCDEF, etc.)
- No punctuation (class, lcurbr, rcurbr, dot, lparen, rparen, semi)
- ~25 nodes vs 100+ in parse tree

---

## Key Transformations: Parse Tree → AST

Our parser performs these transformations through semantic actions:

### 1. Grammar Symbol Elimination
**Parse Tree**: STATEMENT, ASSIGNSTAT, EXPR, ARITHEXPR, TERM, FACTOR, VARIABLE, etc.  
**AST**: AssignStmt, BinaryExpr, Variable, Identifier

**Code**: `buildSemanticNode()` function creates semantic node names (line 317-1017 in parserdriver.cpp)

### 2. Token Elimination
**Parse Tree**: id, equal, semi, lparen, rparen, lsqbr, rsqbr, addop, multop, relop  
**AST**: Tokens removed, only semantic values retained (identifier names, literal values)

**Code**: Tokens stored temporarily in `ChildValue`, then discarded or embedded as metadata

### 3. Epsilon Production Removal
**Parse Tree**: Shows all ε (epsilon) productions like REPTVARIABLE0 → ε, RIGHTRECARITHEXPR → ε  
**AST**: Epsilon productions simply omitted (no node created)

**Code**: `buildSemanticNode()` returns `nullptr` for epsilon cases (line 1010-1015)

### 4. Operator Folding
**Parse Tree**: Flat right-recursive structure (RIGHTRECARITHEXPR, RIGHTRECTERM)  
**AST**: Nested binary tree with operators embedded (BinaryExpr: +, BinaryExpr: *)

**Code**: `fold()` lambda function (line 652-697) combines operators with operands into tree

### 5. List Flattening
**Parse Tree**: REPT* nodes wrap each repetition element  
**AST**: List elements become direct children of parent

**Code**: `makeListNode()` function (line 181-203) collects repetitions into flat list

---

## Verification

### Text Output Format (.outast)
```bash
cat test-checklist-comprehensive.outast
```

Shows only semantic nodes:
- Program, ClassDecl, FunctionDef
- AssignStmt, IfStmt, WhileStmt
- BinaryExpr, Variable, Identifier
- NO grammar symbols: STATEMENT, ASSIGNSTAT, EXPR, ARITHEXPR, TERM, FACTOR, REPT*
- NO tokens: equal, semi, lparen, rparen

### DOT Output Format (.dot)
```bash
cat test-member-call.dot
```

GraphViz format with semantic nodes:
```
node0 [label="Program"];
node1 [label="FunctionDef"];
node2 [label="FunctionDecl"];
node3 [label="Identifier\ntest\n[1:10-1:14]"];
```

Node labels are semantic (Program, FunctionDef, Identifier), not grammar symbols.

### Code Evidence
File: `parserdriver.cpp`

**Semantic node creation** (line 332-347):
```cpp
case "ASSIGNSTAT": {
    // Creates AssignStmt (semantic), not ASSIGNSTAT (grammar)
    auto var = findChildBySymbol(children, "id");
    auto expr = findChildBySymbol(children, "EXPR");
    return factory.makeAssignStmt(var->node, expr->node);
}
```

**Operator folding** (line 652-697):
```cpp
auto fold = [&](ASTNode::Ptr left, const string& op, ASTNode::Ptr right) {
    // Creates BinaryExpr with embedded operator ("+", "*", "==")
    // NOT RIGHTRECARITHEXPR with separate addop token
    return factory.makeBinaryExpr(op, left, right);
};
```

**Epsilon handling** (line 1010-1015):
```cpp
if (production.empty()) {
    // Epsilon production → return nullptr (no node created)
    return {nullptr, "EPSILON"};
}
```

---

## Conclusion

Our parser generates a **true Abstract Syntax Tree (AST)**, verified by:

1. ✅ **No grammar symbols**: Output contains AssignStmt, BinaryExpr, IfStmt (semantic names), not STATEMENT, ASSIGNSTAT, EXPR (grammar symbols)
2. ✅ **No tokens**: Output omits punctuation (equal, semi, lparen, rparen) and keywords (if, then, else, class)
3. ✅ **No epsilon productions**: Epsilon cases omitted entirely
4. ✅ **Operator embedding**: BinaryExpr: +, UnaryExpr: - (operators embedded in nodes)
5. ✅ **Compact structure**: AST has ~10-20% of parse tree node count
6. ✅ **Semantic meaning**: Direct representation of program structure, not derivation steps

**Requirement 5.2 SATISFIED**: The output is an AST, not a parse tree.
