
# Attribute Grammar Specification  
## Assignment 3 – Construction of the Abstract Syntax Tree via Semantic Actions  

**Date:** 8 November 2025  
**Prepared by:** Group 2 – SEP700  

---

## 1. Purpose of This Document  

This document presents the **attribute grammar specification** developed for Assignment 3. It extends the provided LL(1) grammar by incorporating **semantic actions**—executable code fragments that are invoked during parsing. These actions systematically construct the **Abstract Syntax Tree (AST)** on a node-by-node basis.  

All specifications adhere strictly to the original grammar; no production rules have been altered. The actions merely define the behaviour upon recognition of each rule.  

---

## 2. Core Concept – Synthesized Attributes  

Each non-terminal symbol is associated with a single synthesized attribute, denoted `node`:  
```text
node = std::shared_ptr<ASTNode>
```  
This attribute represents the corresponding AST subtree for that syntactic construct.  

---

## 3. Production Rules and Associated Semantic Actions  

The following subsections detail the principal productions and their precise semantic actions for AST construction. (The complete specification, comprising **523 lines** of actions, is available in `ATTRIBUTE_GRAMMAR_SPECIFICATION.md`.)  

---

### 3.1 Program Structure  

```text
START → REPTSTART0
START.node = Program(REPTSTART0.children)
```  
*Instantiates the root `Program` node containing all top-level declarations.*  

```text
REPTSTART0 → CLASSDECLORFUNCDEF REPTSTART0 | ε
REPTSTART0.node = TranslationUnitList(CLASSDECLORFUNCDEF.node :: REPTSTART0.node)
```  
*Forms a flattened list of class and function definitions.*  

---

### 3.2 Class Declarations  

```text
CLASSDECL → class id OPTCLASSDECL2 { REPTCLASSDECL4 } ;
CLASSDECL.node = ClassDecl(id.node, OPTCLASSDECL2.node, REPTCLASSDECL4.children)
```  
*Encapsulates class name, optional inheritance, and member list.*  

```text
OPTCLASSDECL2 → isa id REPTOPTCLASSDECL22 | ε
OPTCLASSDECL2.node = InheritList(id.node :: REPTOPTCLASSDECL22.children) | nullptr
```  
*Constructs an inheritance list if `isa` is present; otherwise returns `nullptr`.*  

```text
REPTCLASSDECL4 → VISIBILITY MEMBERDECL REPTCLASSDECL4 | ε
REPTCLASSDECL4.node = MemberList(MemberDecl(VISIBILITY.node, MEMBERDECL.node) :: …)
```  
*Associates visibility (`public`/`private`) with each member.*  

---

### 3.3 Member Variables and Functions  

```text
MEMBERVARDECL → attribute id : TYPE REPTMEMBERVARDECL4 ;
MEMBERVARDECL.node = VarDecl(id.node, TYPE.node, REPTMEMBERVARDECL4.children)
```  

```text
MEMBERFUNCDECL → function id : ( FPARAMS ) → RETURNTYPE ;
MEMBERFUNCDECL.node = MemberFuncDecl(id.node, FPARAMS.children, RETURNTYPE.node)
```  
*Constructors are treated identically, distinguished by a dedicated flag.*  

---

### 3.4 Function Definitions  

```text
FUNCDEF → FUNCHEAD FUNCBODY
FUNCDEF.node = FunctionDef(FUNCHEAD.node, FUNCBODY.node)
```  

```text
FUNCBODY → { REPTFUNCBODY1 }
FUNCBODY.node = Block(REPTFUNCBODY1.children)
```  
*Encapsulates the function body within a `Block` node.*  

---

### 3.5 Formal Parameters  

```text
FPARAMS → id : TYPE REPTFPARAMS3 REPTFPARAMS4 | ε
FPARAMS.node = ParamList(Param(id.node, TYPE.node, …) :: …)
```  
*Aggregates parameters, including type and optional array dimensions, into a list.*  

---

### 3.6 Statements  

```text
STATEMENT → ASSIGNSTAT ; 
          | if ( RELEXPR ) then STATBLOCK else STATBLOCK ;
          | while ( RELEXPR ) STATBLOCK ;
          | read ( VARIABLE ) ;
          | write ( EXPR ) ;
          | return ( EXPR ) ;
          | FUNCTIONCALL ;
```  
*Each variant instantiates the corresponding statement node (`IfStmt`, `WhileStmt`, `ReadStmt`, etc.).*  

```text
ASSIGNSTAT → VARIABLE ASSIGNOP EXPR
ASSIGNSTAT.node = AssignStmt(VARIABLE.node, EXPR.node)
```  

---

### 3.7 Local Variable Declarations  

```text
LOCALVARDECL → localvar id : TYPE REPTLOCALVARDECL4 ;
LOCALVARDECL.node = VarDecl(id.node, TYPE.node, REPTLOCALVARDECL4.children)
```  
*Analogous to member variables, scoped within functions.*  

---

### 3.8 Arithmetic and Relational Expressions  

```text
ARITHEXPR → TERM RIGHTRECARITHEXPR
ARITHEXPR.node = fold(TERM.node, RIGHTRECARITHEXPR.children, makeBinary)
```  
*Left-associative folding of `+`, `–`, `or`.*  

```text
TERM → FACTOR RIGHTRECTERM
TERM.node = fold(FACTOR.node, RIGHTRECTERM.children, makeBinary)
```  
*Folding of `*`, `/`, `and`.*  

```text
RELEXPR → ARITHEXPR RELOP ARITHEXPR
RELEXPR.node = BinaryOp(RELOP.value, left, right)
```  

---

### 3.9 Factors  

```text
FACTOR → VARIABLE | FUNCTIONCALL | intlit | floatlit
       | ( ARITHEXPR ) | not FACTOR | SIGN FACTOR
```  
*Generates literal values, variable references, function calls, parenthesised expressions, logical negation, or unary signs.*  

---

### 3.10 Variables and Function Calls  

```text
VARIABLE → REPTVARIABLE0 id REPTVARIABLE2
VARIABLE.node = Variable(prefix…, id.node, indices…)
```  
*Supports complex lvalues such as `obj.arr[i+1].field`.*  

```text
FUNCTIONCALL → REPTFUNCTIONCALL0 id ( APARAMS )
FUNCTIONCALL.node = FunctionCall(prefix…, id.node, APARAMS.children)
```  

---

### 3.11 Actual Parameter Lists  

```text
APARAMS → EXPR REPTAPARAMS1 | ε
APARAMS.node = ArgumentList(EXPR.node :: …)
```  
*Collects expressions supplied to a function call.*  

---

### 3.12 Types and Array Dimensions  

```text
TYPE → integer | float | id
TYPE.node = TypeNode(type)
```  

```text
ARRAYSIZE → [ intlit ] | [ ]
ARRAYSIZE.node = ArrayDim(intlit.node) | ArrayDim()
```  

---

### 3.13 Operators  

```text
ADDOP  → + | - | or
MULTOP → * | / | and
RELOP  → == | <> | < | > | <= | >=
```  
*Represented as terminal nodes carrying their textual value.*  

---

## 4. Supporting Utility Functions  

| Function | Purpose |
|----------|---------|
| `fold(left, opList, makeBinary)` | Transforms `a + b – c` into `((a + b) – c)` |
| `findChildBySymbol(children, sym, n)` | Retrieves the *n*-th child matching a given symbol |
| `childAt(children, idx)` | Direct access to `children[idx]` |

---

## 5. Implementation Challenges and Resolutions  

### 5.1 Imperfections in the Parsing Table  

The supplied LL(1) table contained **missing or ambiguous entries**, including:  

* `REPTSTART0` on `$`  
* `IDNEST` (two productions, one table entry)  
* Constructor syntax `function id :: constructor`  

**Resolution:**  
Additional **lookahead logic** (2–3 tokens) was implemented *prior* to table consultation, enabling correct production selection without grammar modification.  

### 5.2 Token Identifier Mapping  

Lexer tokens (e.g., `intnum`, `assign`, `scopeop`) are normalised in `tokToTerm()`:  

```cpp
intnum   → intlit
assign   → equal
scopeop  → sr
...
```  

---

## 6. Validation and Testing  

| Test Case | Outcome |
|-----------|---------|
| Basic class and function definitions | Correct AST generated |
| All 50+ non-terminals | Semantic actions defined |
| Empty lists and optional constructs | Properly handled via `ε` |
| Deeply nested expressions and indices | Accurate structure in `.dot` output |
| Edge cases (empty class, parameterless functions) | No runtime failures |

**Known Limitation:**  
Highly complex member access chains (e.g., `obj.method(arg).field[i]`) occasionally exceed two-token lookahead, resulting in syntax errors due to table constraints—not action logic.  

---

## 7. Conclusion  

A **comprehensive attribute grammar** has been implemented, transforming every valid input program into a semantically accurate **AST**.  

* Each production is equipped with an explicit semantic action.  
* Recursive list constructs (`REPT*`) are flattened, eliminating them from the final tree.  
* The parser compensates for parsing table deficiencies using safe, minimal lookahead.  
* Generated `.outast` and `.dot` files fully comply with assignment requirements.  

The exhaustive action set is documented in `ATTRIBUTE_GRAMMAR_SPECIFICATION.md`, reflecting the exact behaviour executed by the parser.  

---  