## **Assignment 3 – Abstract Syntax Tree Generator**

**Group 2 – README**

**Group Members:**

* Mirac Ozcan (mozkan1@myseneca.ca)  
* Sidhant Sharma (ssharma471@myseneca.ca)  
* Arvin (aarmand1@myseneca.ca)  
* Paschal (pibeh@myseneca.ca)  

**Date:** 8 November 2025

**Declaration:**  
This submission represents original work completed entirely by our group. No code or materials were shared with or received from other groups or external sources.

---

## **1. Overview**

This assignment extends the existing LL(1) parser to generate a **genuine Abstract Syntax Tree (AST)** during syntax analysis. The parser consumes a source file (`.src`), applies the provided parsing table, and executes **semantic actions** upon each successful reduction to construct AST nodes incrementally.

The resulting AST is emitted in two formats:  
- **`.outast`** – a human-readable, indented textual representation  
- **`.dot`** – a GraphViz-compatible directed graph for visualisation  

The implementation fully supports the target language, including:  
- Classes with inheritance and visibility  
- Member and free functions, including constructors  
- Local variable declarations  
- Array types (multi-dimensional, fixed and dynamic)  
- Control structures (`if`, `while`, `read`, `write`, `return`)  
- Complex expressions with correct operator precedence  
- Nested indexing and function calls  

---

## **2. System Requirements**

- **Compiler**: C++17-compliant (`g++`, `clang++`)  
- **Runtime Environment**: Terminal access  
- **Optional**: GraphViz (`dot`) for rendering `.dot` files  

**Tested on:**  
- Linux (Ubuntu, Fedora)  
- macOS  
- Windows (via WSL or MinGW)  

---

## **3. Compilation Instructions**

### Using Provided Makefile (Recommended)

```bash
cd Abstract-Syntax-Tree-Generation
make clean
make
```

### Manual Compilation

```bash
g++ -std=c++17 \
    parserdriver.cpp \
    ast.cpp \
    ast_factory.cpp \
    semantic_stack.cpp \
    include/lexer.cpp \
    -o parser
```

Successful compilation produces an executable: **`parser`**

---

## **4. Execution**

### Basic Syntax

```bash
./parser grammar/parsing_table.csv <input.src>
```

### Example Usage

```bash
./parser grammar/parsing_table.csv test-checklist-comprehensive.src
./parser grammar/parsing_table.csv source-files/example-polynomial.src
```

### Generated Output Files

| File | Description |
|------|-------------|
| `<input>.outast` | Indented, readable AST |
| `<input>.dot` | GraphViz graph (viewable via `dot -Tpng`) |
| `<input>.outtokens` | Complete token stream |
| `<input>.outderivation` | Step-by-step parse derivation |
| `<input>.outsyntaxerrors` | Syntax error diagnostics (if any) |

---

## **5. Project Structure**

```
Abstract-Syntax-Tree-Generation/
│
├── parserdriver.cpp          # Main parser loop + semantic dispatch
├── ast.cpp / ast.hpp         # AST node hierarchy
├── ast_factory.cpp/.hpp      # Factory methods for node creation
├── semantic_stack.*          # Parallel stack for bottom-up AST construction
├── include/lexer.*           # Tokenisation layer
│
├── grammar/
│   ├── parsing_table.csv     # LL(1) parsing table
│   └── LL1grammar.grm        # Reference grammar
│
├── test-checklist-comprehensive.src
├── test-6.*.src              # Focused requirement tests
└── source-files/*.src        # Example programs
```

---

## **6. Test Suite**

- **`test-checklist-comprehensive.src`** – 189 lines covering **all 17 language requirements**  
- **7 targeted test files** (`test-6.1-*.src`) for granular validation  
- Real-world examples:  
  - `example-bubblesort.src` – arrays, loops, function calls  
  - `example-polynomial.src` – OOP, inheritance, constructors  

**All tests execute successfully** and generate correct, verifiable ASTs.

---

## **7. Visualising the AST**

### Textual View

```bash
cat example.outast | head -20
```

### Graphical View (GraphViz)

```bash
dot -Tpng example.dot -o example.png
open example.png    # macOS
xdg-open example.png  # Linux
```

---

## **8. Troubleshooting**

| Symptom | Resolution |
|-------|------------|
| `parsing_table.csv` not found | Verify relative path from execution directory |
| No `parser` executable | Ensure compilation completed without errors |
| No output files generated | Check `.outsyntaxerrors` for syntax issues |
| Segfault or crash | Input likely exceeds grammar limits — review constraints |

---

## **9. Design Notes**

- **Semantic Purity**: The AST contains **only program meaning** — no grammar symbols, punctuation, or `ε`/`REPT*` nodes  
- **Operator Precedence**: Preserved via **nested `BinaryExpr` nodes** (e.g., `*` inside `+`)  
- **Memory Safety**: Full use of `std::shared_ptr<ASTNode>` — **zero manual deletion**  
- **Error Resilience**: Graceful recovery via stack pop-and-skip  

---

**Last Updated:** 8 November 2025
