```markdown
# Test Cases – **ALL 17 REQUIREMENTS: 100% COVERED**  
## **Group 2 – SEP700**  
**Date:** November 8, 2025  

---

## **ONE FILE. ALL TESTS.**

```
test-checklist-comprehensive.src → 189 lines → ALL 17 REQUIREMENTS
```

**Plus 7 focused test files** for surgical precision.

---

## **Coverage at a Glance**

| Req | Feature | Test File | Status |
|-----|--------|-----------|--------|
| 6.1 | Class declarations | `test-checklist-comprehensive.src` | PASS |
| 6.2 | Data members | `test-checklist-comprehensive.src` | PASS |
| 6.3 | Member functions | `test-checklist-comprehensive.src` | PASS |
| 6.4 | Inheritance | `test-checklist-comprehensive.src` | PASS |
| 6.5 | Public/private | `test-checklist-comprehensive.src` | PASS |
| 6.6 | Free functions | `test-checklist-comprehensive.src` | PASS |
| 6.7 | Constructors | `test-checklist-comprehensive.src` | PASS |
| 6.8 | Local vars | `test-checklist-comprehensive.src` | PASS |
| 6.9 | int/float | `test-checklist-comprehensive.src` | PASS |
| 6.10 | Arrays | `test-checklist-comprehensive.src` | PASS |
| 6.11 | `if` statements | `test-checklist-comprehensive.src` | PASS |
| 6.12 | `while` loops | `test-checklist-comprehensive.src` | PASS |
| 6.13 | `read`/`write` | `test-checklist-comprehensive.src` | PASS |
| 6.14 | `return` | `test-checklist-comprehensive.src` | PASS |
| 6.15 | Assignments | `test-checklist-comprehensive.src` | PASS |
| 6.16 | Complex indices | `test-checklist-comprehensive.src` | PASS |
| 6.17 | Complex expressions | `test-checklist-comprehensive.src` | PASS |

**TOTAL: 17/17**

---

## **Proof: Real Code, Real AST**

### 6.1 – Class Declarations
```c
class Empty {};
class Full isa Base { public attribute x : integer; };
```
**AST**: `ClassDecl`, `InheritList`, `MemberDecl`

---

### 6.2 – Data Members
```c
public attribute x : integer;
private attribute arr : integer[10];
private attribute matrix : float[5][5];
```
**AST**: `VarDecl`, `ArrayDim`, `TypeName`

---

### 6.3 – Member Functions
```c
public function compute: (a: integer) => float;
private function helper: () => void;
```
**AST**: `MemberFuncDecl`, `Param`, `TypeName`

---

### 6.4 – Inheritance
```c
class Derived isa Base { ... }
```
**AST**: `InheritList → Identifier: Base`

---

### 6.5 – Visibility
```c
public attribute x : integer;
private function secret: () => void;
```
**AST**: `Keyword: public`, `Keyword: private`

---

### 6.6 – Free Functions
```c
function add(a: integer, b: integer) => integer { return(a + b); }
```
**AST**: `FunctionDef` at top level

---

### 6.7 – Constructors
```c
public constructor: (val: integer);
```
**AST**: `Constructor` node

---

### 6.8 – Local Variables
```c
localvar a : integer;
localvar arr : integer[5];
```
**AST**: `VarDecl` inside `Block`

---

### 6.9 – int/float
```c
localvar x : integer;
localvar y : float;
```
**AST**: `TypeName: integer`, `TypeName: float`

---

### 6.10 – Arrays
```c
integer[10], float[5][5], integer[]
```
**AST**: `ArrayDim` with `IntLiteral` or empty

---

### 6.11 – `if` Statements
```c
if (x == 5) then { ... } else { ... };
```
**AST**: `IfStmt` with 3 children

---

### 6.12 – `while` Loops
```c
while (i < n) { i = i + 1; }
```
**AST**: `WhileStmt` with condition + body

---

### 6.13 – `read`/`write`
```c
read(x); write("Hello"); write(a + b);
```
**AST**: `ReadStmt`, `WriteStmt` with `StringLiteral`

---

### 6.14 – `return`
```c
return(a + b * 2);
```
**AST**: `ReturnStmt` with `BinaryExpr`

---

### 6.15 – Assignments
```c
x = 42; arr[i] = 10; p.x = 5;
```
**AST**: `AssignStmt` with `Variable`, `Index`

---

### 6.16 – Complex Indices
```c
matrix[i + 1][j - 1] = 3;
matrix[arr[i]][arr[j]] = 4;
```
**AST**: Nested `Index` + `BinaryExpr`

---

### 6.17 – Complex Expressions
```c
a + b * 2 - 3 / 1
not (x == 5 and y > 0)
p.distance(q)
```
**AST**:
- `BinaryExpr: +` → `*` → `/`
- `UnaryExpr: not`
- `FunctionCall` with `MemberAccess`

---

## **Verification Commands**

```bash
# Run the master test
./parser grammar/parsing_table.csv test-checklist-comprehensive.src
[OK] → .outast, .dot

# Count key nodes
cat test-checklist-comprehensive.outast | grep -c "ClassDecl"      # 8+
cat test-checklist-comprehensive.outast | grep -c "FunctionDef"   # 10+
cat test-checklist-comprehensive.outast | grep -c "BinaryExpr"    # 20+
```

---

## **Bonus: 7 Focused Test Files**

| File | Purpose |
|------|--------|
| `test-6.1-class-declarations.src` | Edge-case classes |
| `test-6.2-data-members.src` | All array + visibility combos |
| `test-6.4-inheritance.src` | Multi-level inheritance |
| `test-6.5-visibility.src` | Mixed public/private |
| `test-6.7-member-function-defs.src` | Constructors galore |
| `test-6.9-int-float.src` | Type system stress test |
| `test-6.16-complex-indices.src` | Index expression madness |

**All PASS** → `[OK]`

---

## **Final Verdict**

- **One file covers all 17**  
- **7 more for depth**  
- **All ASTs correct**  
- **All edge cases hit**  
- **All operators, nesting, precedence verified**

---

**Group 2 – SEP700**  
**November 8, 2025**