## **Assignment 3 – Abstract Syntax Tree Generator**

**Group 2 – README**

**Group Members:**

* Mirac Ozcan (mozkan1@myseneca.ca)
* Sidhant Sharma (ssharma471@myseneca.ca)
* Arvin (aarmand1@myseneca.ca)
* Paschal (pibeh@myseneca.ca)

**Date:** November 8, 2025

**Declaration:**
This work was completed by our group. We didn’t share it and didn’t receive work from other groups or external services.

---

## **1. Overview**

In this assignment, we extended our LL(1) parser so it also builds an **Abstract Syntax Tree (AST)**. The parser reads a `.src` file, follows the parsing table, and whenever a grammar rule finishes, it creates the corresponding AST node. The AST is then written out in two formats:

* A readable tree (`.outast`)
* A DOT graph (`.dot`) that can be viewed with GraphViz

This implementation supports the full language: classes, inheritance, functions, statements, expressions, arrays, and nested indexing.

---

## **2. Requirements / Tools**

You’ll need:

* A C++17 compatible compiler (g++ or clang++)
* Terminal access to run the program
* (Optional) GraphViz to visualize `.dot` output

Works on:

* Linux
* macOS
* Windows (via WSL or MinGW)

---

## **3. How to Compile**

### Using Makefile (if available)

```bash
cd Abstract-Syntax-Tree-Generation
make clean
make
```

### Or compile manually

```bash
g++ -std=c++17 parserdriver.cpp ast.cpp ast_factory.cpp semantic_stack.cpp include/lexer.cpp -o parser
```

After compiling, you should see an executable named `parser`.

---

## **4. How to Run**

Basic command:

```bash
./parser grammar/parsing_table.csv <source_file.src>
```

Examples:

```bash
./parser grammar/parsing_table.csv test-checklist-comprehensive.src
./parser grammar/parsing_table.csv source-files/example-polynomial.src
```

For each `.src` input, the program creates:

| Output File        | Purpose                     |
| ------------------ | --------------------------- |
| `.outast`          | Text AST                    |
| `.dot`             | GraphViz format             |
| `.outtokens`       | Token listing               |
| `.outderivation`   | Parse steps                 |
| `.outsyntaxerrors` | Syntax error notes (if any) |

---

## **5. Project Structure (Simplified)**

```
Abstract-Syntax-Tree-Generation/
│ parserdriver.cpp        # Parsing + semantic actions
│ ast.cpp / ast.hpp       # AST node types
│ ast_factory.cpp/.hpp    # Helpers for creating nodes
│ semantic_stack.*        # Stack for building AST
│ include/lexer.*         # Lexer
│ grammar/parsing_table.csv
│ grammar/LL1grammar.grm
│ test-checklist-comprehensive.src
│ source-files/*.src
```

---

## **6. Test Files**

* `test-checklist-comprehensive.src` exercises all language features.
* Additional `test-6.*.src` files focus on specific features (classes, arrays, expressions, etc.).
* `example-bubblesort.src` and `example-polynomial.src` test real programs.

All these tests were run successfully and produced valid AST outputs.

---

## **7. Viewing the AST**

### Text format:

```bash
head example.outast
```

### Visual format (requires GraphViz):

```bash
dot -Tpng example.dot -o example.png
```

---

## **8. Troubleshooting**

| Issue                                        | Fix                                                                |
| -------------------------------------------- | ------------------------------------------------------------------ |
| Parser says it cannot find the parsing table | Make sure the path `grammar/parsing_table.csv` is correct          |
| No executable appears after compiling        | Check that compilation completed without errors                    |
| Output files not created                     | The input file likely had syntax errors → check `.outsyntaxerrors` |

---

## **9. Notes**

* The AST only contains meaningful structure, not grammar symbols or punctuation.
* Expression trees are properly nested, so operator precedence is preserved via grammar structure.
* The project uses `std::shared_ptr`, so no manual memory cleanup is needed.

---

**Last Updated:** November 8, 2025