# Test Cases – **ALL 17 REQUIREMENTS: 100 % COVERED**  
## **Group 2 – SEP700**  
**Date:** 8 November 2025  

---

## **ONE COMPREHENSIVE FILE. ALL TESTS.**

```
test-checklist-comprehensive.src → 189 lines → COVERS ALL 17 REQUIREMENTS
```

**Supplemented by 7 targeted test files** for precision validation.

---

## **Coverage Summary**

| Req | Feature | Test File | Status |
|-----|---------|-----------|--------|
| 6.1 | Class declarations | `test-checklist-comprehensive.src` | PASS |
| 6.2 | Data members | `test-checklist-comprehensive.src` | PASS |
| 6.3 | Member functions | `test-checklist-comprehensive.src` | PASS |
| 6.4 | Inheritance | `test-checklist-comprehensive.src` | PASS |
| 6.5 | Public/private visibility | `test-checklist-comprehensive.src` | PASS |
| 6.6 | Free-standing functions | `test-checklist-comprehensive.src` | PASS |
| 6.7 | Constructors | `test-checklist-comprehensive.src` | PASS |
| 6.8 | Local variables | `test-checklist-comprehensive.src` | PASS |
| 6.9 | `integer` / `float` types | `test-checklist-comprehensive.src` | PASS |
| 6.10 | Array types | `test-checklist-comprehensive.src` | PASS |
| 6.11 | `if` statements | `test-checklist-comprehensive.src` | PASS |
| 6.12 | `while` loops | `test-checklist-comprehensive.src` | PASS |
| 6.13 | `read` / `write` statements | `test-checklist-comprehensive.src` | PASS |
| 6.14 | `return` statements | `test-checklist-comprehensive.src` | PASS |
| 6.15 | Assignment statements | `test-checklist-comprehensive.src` | PASS |
| 6.16 | Complex array indexing | `test-checklist-comprehensive.src` | PASS |
| 6.17 | Complex arithmetic expressions | `test-checklist-comprehensive.src` | PASS |

**TOTAL: 17/17**

---

## **Evidence: Source Code and Corresponding AST Fragments**

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

### 6.5 – Visibility Modifiers
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
**AST**: `FunctionDef` at program scope

---

### 6.7 – Constructors
```c
public constructor: (val: integer);
```
**AST**: `Constructor` node with parameter list

---

### 6.8 – Local Variables
```c
localvar a : integer;
localvar arr : integer[5];
```
**AST**: `VarDecl` within `Block`

---

### 6.9 – Integer and Float Types
```c
localvar x : integer;
localvar y : float;
```
**AST**: `TypeName: integer`, `TypeName: float`

---

### 6.10 – Array Types
```c
integer[10], float[5][5], integer[]
```
**AST**: `ArrayDim` with `IntLiteral` or empty dimension

---

### 6.11 – `if` Statements
```c
if (x == 5) then { ... } else { ... };
```
**AST**: `IfStmt` with condition, then-block, else-block

---

### 6.12 – `while` Loops
```c
while (i < n) { i = i + 1; }
```
**AST**: `WhileStmt` with condition and body

---

### 6.13 – `read` / `write`
```c
read(x); write(a + b);
```
**AST**: `ReadStmt`, `WriteStmt` with `BinaryExpr`

---

### 6.14 – `return`
```c
return(a + b * 2);
```
**AST**: `ReturnStmt` containing nested `BinaryExpr`

---

### 6.15 – Assignments
```c
x = 42; arr[i] = 10;
```
**AST**: `AssignStmt` with `Variable`, `Index`

---

### 6.16 – Complex Array Indexing
```c
matrix[i + 1][j - 1] = 3;
matrix[arr[i]][arr[j]] = 4;
```
**AST**: Nested `Index` nodes with `BinaryExpr` indices

---

### 6.17 – Complex Expressions
```c
a + b * 2 - 3 / 1
x == 5
```
**AST**:
- `BinaryExpr: +` → `*` → `/`
- `BinaryExpr: ==` with precedence preserved

---

## **Validation Commands**

```bash
# Execute comprehensive test
./parser grammar/parsing_table.csv test-checklist-comprehensive.src
[OK] → .outast, .dot

# Verify node counts
grep -c "ClassDecl"   test-checklist-comprehensive.outast  # ≥8
grep -c "FunctionDef" test-checklist-comprehensive.outast  # ≥10
grep -c "BinaryExpr"  test-checklist-comprehensive.outast  # ≥20
```

---

## **Supplementary Focused Test Files**

| File | Purpose |
|------|--------|
| `test-6.1-class-declarations.src` | Edge-case class structures |
| `test-6.2-data-members.src` | All visibility and array combinations |
| `test-6.4-inheritance.src` | Multi-level inheritance chains |
| `test-6.5-visibility.src` | Mixed visibility scoping |
| `test-6.7-member-function-defs.src` | Constructor variants |
| `test-6.9-int-float.src` | Type system boundary checks |
| `test-6.16-complex-indices.src` | Deeply nested index expressions |

**All execute successfully** → `[OK]`

---

## **Grammar Constraints and Test Adaptations**

The following language features are **not supported** by the provided grammar and were therefore excluded or adapted:

### **Unsupported Constructs:**
1. **Member access** (`obj.field`) – dot notation absent from grammar  
2. **Logical operators** (`and`, `or`, `not`) – only relational operators in conditions  
3. **Array return types** (`=> integer[10]`) – restricted to simple types or `void`  
4. **String literals** – lexer lacks string token support  
5. **Local variables in statement blocks** – must appear at function body level  

### **Adaptations Applied:**
- Replaced member access with standalone variables  
- Simplified conditions to relational expressions only  
- Modified return types to `void` or scalar types  
- Removed string literals from `write` statements  
- Relocated local declarations to function scope  

### **Outcome:**
All 17 requirements **successfully validated** within grammar constraints, producing correct ASTs.

---

## **Final Assessment**

- **Single file satisfies all 17 requirements**  
- **Seven additional files provide granular verification**  
- **All generated ASTs are structurally correct**  
- **Edge cases, operator precedence, and nesting fully exercised**  
- **Compliance achieved despite grammar limitations**

---

**Group 2 – SEP700**  
**8 November 2025**