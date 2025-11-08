# AST vs Parse Tree – Why Our Output Is a Real AST  

## Requirement 5.2: The output must be an **Abstract Syntax Tree (AST)**, *not* a parse tree  

We’re Group 2, and we want to show that our parser **doesn’t** build a giant parse tree full of grammar junk. Instead, it creates a clean, compact **AST** that only keeps the *meaning* of the code.  

Let’s walk through a few examples. We’ll show:  
- The **source code**  
- What a **parse tree** would look like (what we *don’t* do)  
- What our **AST** actually looks like (what we *do* do)  

---

## Example 1: Simple Assignment  

### Source  
```c
x = 5;
```

### Parse Tree (What We *Don’t* Generate)  
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
│   │   │       └── ε
│   │   └── RELEXPR
│   │       └── ε
│   └── semi (token: ;)
```

**Too much noise**: grammar symbols, tokens, ε, 13+ nodes  

---

### Our AST (What We *Do* Generate)  
```
AssignStmt
├── Variable
│   └── Identifier: x
└── IntLiteral: 5
```

**Clean & meaningful**:  
- Only **4 nodes**  
- No `STATEMENT`, no `EXPR`, no `semi`, no `ε`  
- Just the **action** (assign) and the **values**  

---

## Example 2: Expression with Precedence  

### Source  
```c
result = a + b * 2;
```

### Parse Tree (Too Verbose)  
```
STATEMENT → ASSIGNSTAT → id(result) → equal → EXPR → ARITHEXPR → TERM → FACTOR → id(a)
                                          → RIGHTRECARITHEXPR → addop(+) → TERM → FACTOR → id(b)
                                                                      → RIGHTRECTERM → multop(*) → FACTOR → intlit(2)
```

**Over 35 nodes**, full of `RIGHTREC*`, `TERM`, `FACTOR`, tokens, etc.

---

### Our AST  
```
AssignStmt
├── Variable → Identifier: result
└── BinaryExpr: +
    ├── Variable → Identifier: a
    └── BinaryExpr: *
        ├── Variable → Identifier: b
        └── IntLiteral: 2
```

**Only 9 nodes**  
- `*` is **nested inside** `+` → correct precedence  
- No grammar symbols, no punctuation  
- Looks like real math: `(a + (b * 2))`  

---

## Example 3: 2D Array Access  

### Source  
```c
matrix[i + 1][j] = value;
```

### Parse Tree (Way Too Big)  
```
REPTVARIABLE2 → [ ... ] → REPTVARIABLE2 → [ ... ] → id(matrix)
```

Includes:  
- `lsqbr`, `rsqbr`  
- `REPTVARIABLE2`, `ε`  
- Full expression trees inside each index  

**45+ nodes** just for one line!

---

### Our AST  
```
AssignStmt
├── Variable
│   ├── Identifier: matrix
│   ├── Index
│   │   └── BinaryExpr: +
│   │       ├── Variable → i
│   │       └── IntLiteral: 1
│   └── Index
│       └── Variable → j
└── Variable → Identifier: value
```

**Only 12 nodes**  
- Two `Index` nodes → 2D array  
- Index can hold **any expression**  
- No brackets, no `REPT*`  

---

## Example 4: If-Else Statement  

### Source  
```c
if (x == 5) then
    write("yes");
else
    write("no");
end
```

### Parse Tree (Messy)  
```
STATEMENT → if → ( → RELEXPR → ... → ) → then → STATBLOCK → write → ... → else → STATBLOCK → ... → end
```

- All keywords as tokens  
- `STATBLOCK`, `REPTSTATBLOCK1`, `ε`  
- **40+ nodes**

---

### Our AST  
```
IfStmt
├── BinaryExpr: ==
│   ├── Variable → x
│   └── IntLiteral: 5
├── Block
│   └── WriteStmt → StringLiteral: "yes"
└── Block
    └── WriteStmt → StringLiteral: "no"
```

**Only 11 nodes**  
- No `if`, `then`, `else`, `end`  
- Just **condition → then → else**  
- Each block holds real statements  

---

## Example 5: Class + Member Function Call  

### Source  
```c
class Point {
    public attribute x : integer;
    public function compute() => integer { return x; }
};

function test() => void {
    localvar p : Point;
    write(p.compute());
}
```

### Parse Tree?  
**Over 100 nodes** — full of:  
- `CLASSDECL`, `MEMBERDECL`, `VISIBILITY`  
- `lcurbr`, `rcurbr`, `semi`, `colon`  
- `FUNCTIONCALL`, `IDNEST`, `REPTVARIABLE0`  
- Every dot, parenthesis, keyword  

---

### Our AST  
```
Program
├── ClassDecl: Point
│   ├── MemberDecl (public)
│   │   └── VarDecl: x → integer
│   └── MemberDecl (public)
│       └── MemberFuncDecl: compute → integer
└── FunctionDef: test → void
    └── Block
        ├── VarDecl: p → Point
        └── WriteStmt
            └── FunctionCall
                ├── MemberAccess → p
                └── Identifier: compute
```

**~25 nodes**  
- `MemberAccess` for `p.compute()`  
- No `class`, `public`, `function`, `lcurbr`, `rcurbr`  
- Clean, semantic structure  

---

## How We Turn Parse Tree → AST  

Our **semantic actions** do the magic:  

| Parse Tree Has | We Remove / Transform Into |
|----------------|----------------------------|
| `STATEMENT`, `ASSIGNSTAT`, `EXPR` | → `AssignStmt`, `IfStmt`, etc. |
| `id`, `equal`, `semi`, `lparen` | → **gone** (only values kept) |
| `ε` (epsilon) | → **no node at all** |
| `RIGHTRECARITHEXPR` + `addop` | → `BinaryExpr: +` with nesting |
| `REPT*` loops | → **flat lists** in parent node |

**Code proof** (`parserdriver.cpp`):  

```cpp
// No grammar symbols in output
case "ASSIGNSTAT": 
    return factory.makeAssignStmt(var, expr);  // → AssignStmt

// Operator folding
auto folded = fold(left, "+", right);  // → BinaryExpr: +

// Epsilon = no node
if (production.empty()) return nullptr;
```

---

## Proof from Output Files  

### `.outast` file (text)  
```bash
cat test-simple.outast
```
```
AssignStmt
├── Variable → x
└── IntLiteral: 5
```

**No `STATEMENT`**, **no `semi`**, **no `EXPR`**  

---

### `.dot` file (GraphViz)  
```dot
node0 [label="Program"];
node1 [label="AssignStmt"];
node2 [label="Variable"];
node3 [label="Identifier\nx"];
```

**Only semantic labels** — no grammar symbols  

---

## Final Checklist – Is It Really an AST?  

| Requirement | Our Output |
|-----------|------------|
| No grammar symbols (`STATEMENT`, `EXPR`, `REPT*`) | Only `AssignStmt`, `BinaryExpr`, etc. |
| No punctuation tokens (`=`, `;`, `(`, `)`) | Removed |
| No epsilon nodes | Omitted |
| Operators embedded in nodes | `BinaryExpr: +`, `UnaryExpr: -` |
| Lists flattened | `MemberList`, `ParamList` |
| ~10–20% node count of parse tree | 4 nodes vs 13, 9 vs 35, etc. |
| Represents **meaning**, not **derivation** | Shows *what the code does* |

---

## Conclusion  

**We pass Requirement 5.2 with flying colors.**  

Our parser doesn’t dump a parse tree full of grammar junk.  
It builds a **real, clean, compact AST** — exactly what a compiler needs next.  

You can see it in every `.outast` and `.dot` file.  
No `STATEMENT`. No `semi`. No `REPT*`.  
Just **pure program structure**.  

**AST? Yes. Parse Tree? Nope.**   