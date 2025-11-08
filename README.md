# Assignment 3 - Abstract Syntax Tree Generation
## Group 2 - README

**Group Number:** 2  
**Group Members:**
- Mirac Ozcan (mozkan1@myseneca.ca)
- Sidhant Sharma (ssharma471@myseneca.ca)
- Arvin (aarmand1@myseneca.ca)
- Paschal (Pibeh@myseneca.ca)

**Date Completed:** November 8, 2025

**Authenticity Declaration:**
We declare this submission is the result of our group work and has not been shared with any other groups/students or 3rd party content provider. This submitted piece of work is entirely of our own creation.

---

## Table of Contents
1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Compilation Instructions](#compilation-instructions)
4. [Execution Instructions](#execution-instructions)
5. [File Structure](#file-structure)
6. [Test Files](#test-files)
7. [Output Files](#output-files)
8. [Troubleshooting](#troubleshooting)

---

## Overview

This assignment implements an Abstract Syntax Tree (AST) generator integrated into a table-driven LL(1) parser with syntax-directed translation. The parser reads source files written in the course project language and generates:

- **AST text representation** (.outast files)
- **AST DOT/GraphViz representation** (.dot files)
- **Token stream** (.outtokens files)
- **Derivation trace** (.outderivation files)

The implementation handles all language features including classes, inheritance, functions, statements, expressions, and complex nested structures.

---

## Prerequisites

### Required Tools
- **C++17 Compiler**: g++ 7.0+ or clang++ 5.0+
- **Make** (optional, for makefile-based build)
- **Git** (for cloning/managing repository)

### Environment Compatibility
- ✅ **GitHub Codespaces** (primary testing environment)
- ✅ **Linux** (Ubuntu 20.04+, Fedora, etc.)
- ✅ **macOS** (10.14+)
- ✅ **Windows** (WSL2, MinGW, or MSVC)

### Verification
Check that you have a compatible C++ compiler:
```bash
g++ --version    # Should show version 7.0 or higher
# OR
clang++ --version
```

---

## Compilation Instructions

### Option 1: Using the Makefile (Recommended)

If a `Makefile` is provided:

```bash
# Navigate to the assignment directory
cd Abstract-Syntax-Tree-Generation

# Clean previous builds
make clean

# Compile the parser
make

# OR compile with specific compiler
make CXX=g++
make CXX=clang++
```

### Option 2: Manual Compilation

If compiling manually:

```bash
# Navigate to the assignment directory
cd Abstract-Syntax-Tree-Generation

# Compile all source files
g++ -std=c++17 -Wall -Wextra -O2 \
    parserdriver.cpp \
    ast.cpp \
    ast_factory.cpp \
    semantic_stack.cpp \
    include/lexer.cpp \
    -o parser

# OR with all features
g++ -std=c++17 -Wall -Wextra -O2 -g \
    -I./include \
    parserdriver.cpp ast.cpp ast_factory.cpp semantic_stack.cpp include/lexer.cpp \
    -o parser
```

### Option 3: GitHub Codespaces

```bash
# In Codespaces terminal
cd Abstract-Syntax-Tree-Generation

# Use provided build script
chmod +x build.sh
./build.sh

# OR compile manually
g++ -std=c++17 -Wall parserdriver.cpp ast.cpp ast_factory.cpp semantic_stack.cpp include/lexer.cpp -o parser
```

### Successful Compilation

You should see the executable `parser` (or `parser.exe` on Windows) created in the Abstract-Syntax-Tree-Generation directory with no error messages.

---

## Execution Instructions

### Basic Usage

```bash
./parser <parsing_table.csv> <source_file.src>
```

### Running Individual Test Files

```bash
# Parse a specific test file
./parser grammar/parsing_table.csv test-checklist-comprehensive.src

# Parse provided examples
./parser grammar/parsing_table.csv source-files/example-polynomial.src
./parser grammar/parsing_table.csv source-files/example-bubblesort.src
```

### Running All Test Files (Batch Processing)

```bash
# Run all test files in current directory
for file in test-*.src; do
    echo "Processing $file..."
    ./parser grammar/parsing_table.csv "$file"
done

# Run all provided examples
for file in source-files/*.src; do
    echo "Processing $file..."
    ./parser grammar/parsing_table.csv "$file"
done
```

### Using the Driver Script

If an `ASTdriver` executable or script is provided:

```bash
# Make executable (if needed)
chmod +x ASTdriver

# Run all tests
./ASTdriver
```

### Expected Output

For each successfully parsed source file `example.src`, the following files are generated:

1. **example.outast** - Text representation of the AST
2. **example.dot** - GraphViz DOT format for visualization
3. **example.outtokens** - Token stream
4. **example.outderivation** - Derivation trace
5. **example.outsyntaxerrors** - Syntax error log (if any errors)

Success message:
```
[OK]   "path/to/file.src" -> "path/to/file.outast", "path/to/file.dot"
```

---

## File Structure

```
Abstract-Syntax-Tree-Generation/
├── parser                          # Executable (generated after compilation)
├── parserdriver.cpp                # Main parser implementation (1957 lines)
├── ast.hpp                         # AST node class definitions
├── ast.cpp                         # AST node implementations
├── ast_factory.hpp                 # AST node factory
├── ast_factory.cpp                 # Factory implementations
├── semantic_stack.hpp              # Semantic stack for AST building
├── semantic_stack.cpp              # Stack implementations
├── include/
│   ├── lexer.hpp                   # Lexer header
│   └── lexer.cpp                   # Lexer implementation
├── grammar/
│   ├── parsing_table.csv           # LL(1) parsing table
│   └── LL1grammar.grm              # Grammar specification
├── source-files/
│   ├── example-polynomial.src      # Provided test file
│   └── example-bubblesort.src      # Provided test file
├── test-checklist-comprehensive.src # Main comprehensive test
├── test-6.*.src                    # Individual feature tests
├── ATTRIBUTE_GRAMMAR_SPECIFICATION.md  # Section 1 of report
├── DESIGN_RATIONALE.md             # Section 2 of report
├── CHECKLIST_VERIFICATION.md       # Complete evidence for all requirements
├── TEST_COVERAGE_DOCUMENTATION.md  # Test coverage proof
├── AST_VS_PARSETREE.md             # AST correctness proof
└── group_XX_assign_3_README.md     # This file
```

---

## Test Files

### Comprehensive Test
- **test-checklist-comprehensive.src** - Covers ALL 43 requirements in a single file

### Feature-Specific Tests
- test-6.1-class-declarations.src - Class declarations
- test-6.2-data-members.src - Data member declarations
- test-6.4-inheritance.src - Inheritance structures
- test-6.5-visibility.src - Public/private visibility
- test-6.7-member-function-defs.src - Function definitions
- test-6.9-int-float.src - Integer and float types
- test-6.16-complex-indices.src - Complex array indexing
- (Additional test files for specific features)

### Provided Examples
- source-files/example-polynomial.src - Polynomial evaluation program
- source-files/example-bubblesort.src - Bubble sort implementation

All test files successfully parse and generate correct AST output.

---

## Output Files

### AST Text Format (.outast)

Plain text hierarchical representation of the AST:
```
Program
├─ClassDecl: MyClass
│ ├─VarDecl: x (type: integer)
│ └─FunctionDecl: myMethod
│   └─ParamList
└─FunctionDef: main
  └─StatementList
    └─AssignmentStatement
```

### DOT Format (.dot)

GraphViz-compatible graph description:
```dot
digraph AST {
    node1 [label="Program"];
    node2 [label="ClassDecl: MyClass"];
    node1 -> node2;
    ...
}
```

**Visualizing DOT files:**
```bash
# Using Graphviz (if installed)
dot -Tpng example.dot -o example.png

# Online viewers
# - https://dreampuf.github.io/GraphvizOnline/
# - http://www.webgraphviz.com/
```

### Token Stream (.outtokens)

```
1:1 class "class" [class]
1:7 id "Point" [id]
1:13 lbrace "{" [lcurbr]
...
```

---

## Troubleshooting

### Compilation Errors

**Error: "No such file or directory"**
```bash
# Make sure you're in the Abstract-Syntax-Tree-Generation directory
pwd  # Should show: .../Abstract-Syntax-Tree-Generation

# Check that all source files exist
ls -la *.cpp *.hpp include/
```

**Error: "C++17 required"**
```bash
# Use a newer compiler or specify -std=c++17
g++ -std=c++17 ...
```

### Runtime Errors

**Error: "Cannot open parsing table"**
```bash
# Check that parsing_table.csv exists
ls grammar/parsing_table.csv

# Use correct path
./parser grammar/parsing_table.csv test.src
```

**Error: "Cannot write outputs"**
```bash
# Check write permissions
chmod +w .

# Ensure sufficient disk space
df -h .
```

### Execution Issues in GitHub Codespaces

```bash
# If permissions issue
chmod +x parser

# If path issues
export PATH=$PATH:$(pwd)

# If library issues
ldd parser  # Check dependencies
```

### Parser Output Shows [FAIL]

This indicates syntax errors in the source file. Check:
```bash
# View syntax error log
cat filename.outsyntaxerrors

# Common issues:
# - Missing semicolons
# - Unmatched braces
# - Invalid token sequences
```

---

## Verifying Successful Compilation and Execution

### Quick Test

```bash
# 1. Compile
make clean && make

# 2. Run comprehensive test
./parser grammar/parsing_table.csv test-checklist-comprehensive.src

# 3. Check for success
echo $?  # Should output 0
ls test-checklist-comprehensive.outast  # Should exist

# 4. View AST
head -20 test-checklist-comprehensive.outast
```

### Expected Success Indicators

✅ Compilation completes without errors  
✅ `parser` executable is created  
✅ Running parser shows `[OK]` status  
✅ Output files (.outast, .dot) are generated  
✅ AST output contains semantic nodes (not grammar symbols)  
✅ No crashes or segmentation faults

---

## Additional Documentation

For detailed information about the implementation, refer to:

- **ATTRIBUTE_GRAMMAR_SPECIFICATION.md** - Complete attribute grammar with semantic actions
- **DESIGN_RATIONALE.md** - System architecture and design decisions
- **CHECKLIST_VERIFICATION.md** - Evidence for all 43 requirements
- **TEST_COVERAGE_DOCUMENTATION.md** - Comprehensive test coverage proof
- **AST_VS_PARSETREE.md** - Proof that output is AST, not parse tree

---

**Last Updated:** November 8, 2025
