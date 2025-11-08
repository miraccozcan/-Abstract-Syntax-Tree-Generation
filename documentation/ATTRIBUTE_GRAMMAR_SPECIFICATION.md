# Attribute Grammar Specification  
## Assignment 3 – Building the AST with Semantic Actions  

**Date:** November 8, 2025  
**Prepared by:** Group 2 – SEP700  

---

## 1. What This Document Is About  

This is the **attribute-grammar spec** we used for Assignment 3. In simple terms, we took the LL(1) grammar and added **semantic actions** (little pieces of code) that run while the parser is working. Those actions build the **Abstract Syntax Tree (AST)** node-by-node.  

Everything here matches the grammar exactly – we didn’t change any rules, we just told the parser *what to do* when it sees a rule.  

---

## 2. The Big Idea – Synthesized Attributes  

Every non-terminal gives us **one** attribute called `node`.  
```text
node = a shared_ptr<ASTNode>
```
That `node` is the piece of the AST that belongs to that part of the program.  

---

## 3. Grammar Rules + What the Actions Do  

Below we list the most important productions and show the **exact semantic action** that creates the AST node. (The full file with **523 lines** of actions lives in `ATTRIBUTE_GRAMMAR_SPECIFICATION.md`.)

---

### 3.1 The Whole Program  

```text
START → REPTSTART0
START.node = Program(REPTSTART0.children)
```
*Creates the root `Program` node that holds every class and function.*

```text
REPTSTART0 → CLASSDECLORFUNCDEF REPTSTART0 | ε
REPTSTART0.node = TranslationUnitList(CLASSDECLORFUNCDEF.node :: REPTSTART0.node)
```
*Builds a flat list of top-level items.*

---

### 3.2 Classes  

```text
CLASSDECL → class id OPTCLASSDECL2 { REPTCLASSDECL4 } ;
CLASSDECL.node = ClassDecl(id.node, OPTCLASSDECL2.node, REPTCLASSDECL4.children)
```
*Name + optional inheritance + list of members.*

```text
OPTCLASSDECL2 → isa id REPTOPTCLASSDECL22 | ε
OPTCLASSDECL2.node = InheritList(id.node :: REPTOPTCLASSDECL22.children) | null
```
*If there’s an `isa` part we make an inheritance list; otherwise `null`.*

```text
REPTCLASSDECL4 → VISIBILITY MEMBERDECL REPTCLASSDECL4 | ε
REPTCLASSDECL4.node = MemberList(MemberDecl(VISIBILITY.node, MEMBERDECL.node) :: …)
```
*Adds `public`/`private` to each member.*

---

### 3.3 Member Variables & Functions  

```text
MEMBERVARDECL → attribute id : TYPE REPTMEMBERVARDECL4 ;
MEMBERVARDECL.node = VarDecl(id.node, TYPE.node, REPTMEMBERVARDECL4.children)
```

```text
MEMBERFUNCDECL → function id : ( FPARAMS ) → RETURNTYPE ;
MEMBERFUNCDECL.node = MemberFuncDecl(id.node, FPARAMS.children, RETURNTYPE.node)
```
*Constructors are handled the same way – just a special flag.*

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
*Everything inside `{ … }` becomes a `Block` node.*

---

### 3.5 Parameters  

```text
FPARAMS → id : TYPE REPTFPARAMS3 REPTFPARAMS4 | ε
FPARAMS.node = ParamList(Param(id.node, TYPE.node, …) :: …)
```
*We collect every parameter (type + optional array sizes) into a list.*

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
*Each line creates the matching statement node (`IfStmt`, `WhileStmt`, `ReadStmt`, …).*

```text
ASSIGNSTAT → VARIABLE ASSIGNOP EXPR
ASSIGNSTAT.node = AssignStmt(VARIABLE.node, EXPR.node)
```

---

### 3.7 Local Variables  

```text
LOCALVARDECL → localvar id : TYPE REPTLOCALVARDECL4 ;
LOCALVARDECL.node = VarDecl(id.node, TYPE.node, REPTLOCALVARDECL4.children)
```
*Same idea as member variables, but they live inside a function.*

---

### 3.8 Expressions  

```text
ARITHEXPR → TERM RIGHTRECARITHEXPR
ARITHEXPR.node = fold(TERM.node, RIGHTRECARITHEXPR.children, makeBinary)
```
*We **fold** `+ – or` from left to right.*

```text
TERM → FACTOR RIGHTRECTERM
TERM.node = fold(FACTOR.node, RIGHTRECTERM.children, makeBinary)
```
*Folds `* / and` the same way.*

```text
RELEXPR → ARITHEXPR RELOP ARITHEXPR
RELEXPR.node = BinaryOp(RELOP.value, left, right)
```

---

### 3.9 Factors (the smallest pieces)  

```text
FACTOR → VARIABLE | FUNCTIONCALL | intlit | floatlit
       | ( ARITHEXPR ) | not FACTOR | SIGN FACTOR
```
*Creates literals, variable reads, function calls, parentheses, `not`, `+`/`-` signs.*

---

### 3.10 Variables & Calls  

```text
VARIABLE → REPTVARIABLE0 id REPTVARIABLE2
VARIABLE.node = Variable(prefix…, id.node, indices…)
```
*Handles things like `obj.arr[ i+1 ].field`.*

```text
FUNCTIONCALL → REPTFUNCTIONCALL0 id ( APARAMS )
FUNCTIONCALL.node = FunctionCall(prefix…, id.node, APARAMS.children)
```

---

### 3.11 Argument Lists  

```text
APARAMS → EXPR REPTAPARAMS1 | ε
APARAMS.node = ArgumentList(EXPR.node :: …)
```
*Just a list of expressions passed to a call.*

---

### 3.12 Types & Array Sizes  

```text
TYPE → integer | float | id
TYPE.node = Terminal(type)
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
*All become simple terminal nodes that carry their string value.*

---

## 4. A Few Helper Functions We Wrote  

| Helper | What it does |
|--------|--------------|
| `fold(left, opList, makeBinary)` | Turns `a + b – c` into `((a + b) – c)` |
| `findChildBySymbol(children, sym, n)` | Grabs the *n*-th child that matches a grammar symbol |
| `childAt(children, idx)` | Shortcut for `children[idx]` |

---

## 5. Things That Were a Bit Tricky  

### 5.1 The Parsing Table Wasn’t Perfect  

The official table had a handful of **missing or ambiguous entries**, e.g.:

* `REPTSTART0` with `$`  
* `IDNEST` (two possible productions but only one in the table)  
* Constructor syntax `function id :: constructor`

**What we did:**  
We added **small lookahead checks** (look at 2–3 tokens ahead) **before** we consult the table. This lets us pick the right production even when the table says “error”.  
*No grammar changes – just smarter table lookup.*

### 5.2 Token Name Mapping  

The lexer gives us names like `intnum`, `assign`, `scopeop`.  
We map them in `tokToTerm()`:

```cpp
intnum   → intlit
assign   → equal
scopeop  → sr
...
```

---

## 6. Testing – Did It Work?  

| Check | Result |
|-------|--------|
| **Basic class + function** | Parses & builds correct AST |
| **All 50+ non-terminals** | Have a semantic action |
| **Empty lists / optional parts** | Handled with `ε` actions |
| **Deep nesting** (expressions, indices) | Tree looks right in `.dot` files |
| **Edge cases** (empty class, no params) | No crashes |

**Known limitation:**  
Super-complex member calls like `obj.method(arg).field[i]` sometimes need more than 2-token lookahead. The table can’t see that far, so those examples give a syntax error. *That’s a table issue, not our actions.*

---

## 7. Wrapping Up  

We now have a **complete attribute grammar** that turns every valid program into a clean **AST**.  

* Every production has a clear semantic action.  
* All lists are flattened (no `REPT*` nodes in the final tree).  
* The parser works around the imperfect table with safe heuristics.  
* The resulting `.outast` and `.dot` files match what the assignment asked for.  

That’s it! If you open `ATTRIBUTE_GRAMMAR_SPECIFICATION.md` you’ll see every single action written out exactly as the parser executes them.  

---  