# Assignment 3: Abstract Syntax Tree (AST) Generator
**SEP700 - Compiler Construction**

## 📋 Project Overview

A complete **Abstract Syntax Tree (AST) generator** integrated with a table-driven LL(1) predictive parser using syntax-directed translation. This project implements the front-end of a compiler for an object-oriented programming language, transforming source code into a structured AST representation suitable for semantic analysis and code generation.

## 🎯 Key Features

- **Table-Driven LL(1) Parsing**: Grammar-independent parser using CSV parsing table
- **Syntax-Directed Translation**: 50+ semantic actions integrated with parsing process
- **True AST Generation**: Produces abstract syntax tree (not parse tree) with semantic nodes only
- **Dual Output Formats**: 
  - Text representation (.outast) for inspection
  - GraphViz DOT format (.dot) for visualization
- **Comprehensive Language Support**: Classes, inheritance, functions, statements, expressions
- **Memory Safe**: Smart pointer implementation (std::shared_ptr) with zero memory leaks
- **Robust Error Handling**: Strategic heuristics for broken table cells with graceful recovery

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────┐
│               Parser Driver (1957 lines)            │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐     │
│  │  Lexer   │→ │  Parser  │→ │   Semantic   │     │
│  │ Analysis │  │  Engine  │  │   Actions    │     │
│  └──────────┘  └──────────┘  └──────────────┘     │
│                                      ↓              │
│                          ┌──────────────────┐      │
│                          │  Semantic Stack  │      │
│                          └──────────────────┘      │
│                                      ↓              │
│                          ┌──────────────────┐      │
│                          │   AST Builder    │      │
│                          └──────────────────┘      │
│                                      ↓              │
│                          ┌──────────────────┐      │
│                          │  Output (.outast,│      │
│                          │        .dot)     │      │
│                          └──────────────────┐      │
└─────────────────────────────────────────────────────┘
```

## 💻 Technology Stack

- **Language**: C++17
- **Libraries**: STL (vector, map, shared_ptr, filesystem)
- **Build System**: Make / g++ / clang++
- **Parser Type**: Table-driven LL(1) predictive parser
- **Memory Management**: Smart pointers (RAII)
- **Visualization**: GraphViz DOT language

## 📊 Supported Language Features

### Object-Oriented Constructs
- ✅ Class declarations with members
- ✅ Single and multiple inheritance
- ✅ Public/private visibility modifiers
- ✅ Member variables (primitives and arrays)
- ✅ Member functions and constructors

### Functions
- ✅ Free function definitions
- ✅ Function parameters (formal/actual)
- ✅ Return types (integer, float, void, class types)
- ✅ Function calls and member access

### Statements
- ✅ Assignment statements
- ✅ If-then-else statements
- ✅ While loops
- ✅ Read/Write I/O statements
- ✅ Return statements

### Expressions
- ✅ Binary expressions (addop, multop, relop)
- ✅ Unary expressions (not, +, -)
- ✅ Complex nested expressions
- ✅ Array indexing (1D, 2D, 3D)
- ✅ Member access (dot notation)
- ✅ Function calls with parameters

## 📁 Project Structure

```
assign3/
├── parserdriver.cpp              # Main parser (1957 lines)
├── ast.hpp, ast.cpp              # AST node definitions
├── ast_factory.hpp, .cpp         # Factory pattern for nodes
├── semantic_stack.hpp, .cpp      # Semantic stack management
├── documentation/                # Complete documentation (5000+ lines)
│   ├── ATTRIBUTE_GRAMMAR_SPECIFICATION.md
│   ├── DESIGN_RATIONALE.md
│   ├── CHECKLIST_VERIFICATION.md
│   ├── TEST_COVERAGE_DOCUMENTATION.md
│   └── AST_VS_PARSETREE.md
├── test-files/                   # Comprehensive test suite
│   └── test-checklist-comprehensive.src (189 lines)
├── grammar/                      # LL(1) grammar and parsing table
├── include/                      # Lexer from previous assignment
└── output-files/                 # Generated AST examples
```

## 🧪 Testing

**Test Coverage**: 43/43 requirements (100%)

- **Comprehensive Test**: Single file covering all 43 language features
- **Feature Tests**: 7 individual tests for specific constructs
- **Provided Examples**: Polynomial evaluation, bubble sort
- **All Tests Pass**: 10/10 files parse successfully

### Sample Test Execution
```bash
./parser grammar/parsing_table.csv test-files/test-checklist-comprehensive.src
# Output: [OK] - Generates .outast and .dot files
```

## 🚀 Quick Start

### Compilation
```bash
g++ -std=c++17 -Wall -Wextra -O2 \
    parserdriver.cpp ast.cpp ast_factory.cpp semantic_stack.cpp include/lexer.cpp \
    -o parser
```

### Usage
```bash
./parser <parsing_table.csv> <source_file.src>
```

### Example
```bash
./parser grammar/parsing_table.csv test-files/test-checklist-comprehensive.src
```

**Output Files**:
- `test-checklist-comprehensive.outast` - Text AST representation
- `test-checklist-comprehensive.dot` - GraphViz visualization
- `test-checklist-comprehensive.outtokens` - Token stream

## 📈 Project Metrics

| Metric | Value |
|--------|-------|
| **Source Code** | 1,957 lines (parserdriver.cpp) |
| **Documentation** | 5,000+ lines across 5 files |
| **Test Files** | 10 comprehensive test cases |
| **Requirements Met** | 43/43 (100%) |
| **Test Success Rate** | 10/10 (100%) |
| **Semantic Actions** | 50+ production cases |
| **AST Node Types** | 15+ specialized nodes |

## 🎓 Educational Value

This project demonstrates:
- **Compiler Front-End Design**: Complete lexical, syntax, and semantic analysis
- **Syntax-Directed Translation**: Bottom-up AST construction during parsing
- **Design Patterns**: Factory, Composite, Semantic Stack
- **Modern C++**: Smart pointers, RAII, STL containers
- **Software Engineering**: Modular design, comprehensive testing, documentation

## 📚 Documentation

Complete documentation includes:
- **Attribute Grammar Specification** (523 lines)
- **Design Rationale** (733 lines)
- **Requirements Verification** (1500+ lines)
- **Test Coverage Analysis** (400+ lines)
- **AST vs Parse Tree Proof** (650+ lines)

## 🏆 Quality Indicators

✅ **Zero Crashes**: All tests execute without segmentation faults  
✅ **Memory Safe**: No memory leaks (smart pointer management)  
✅ **Complete Coverage**: Every language construct tested  
✅ **Clean Output**: True AST (no grammar symbols or punctuation)  
✅ **Professional Code**: Well-documented, modular, maintainable  

## 📖 Academic Context

**Course**: SEP700 - Compiler Construction  
**Institution**: Seneca College  
**Assignment**: Assignment 3 - AST Generation  
**Group**: 2  
**Date**: November 2025  

## 🔗 Related Assignments

This project builds upon:
- **Assignment 1**: Lexical Analysis (Token generation)
- **Assignment 2**: LL(1) Grammar and Parsing Table

Future assignments will use this AST for:
- **Assignment 4**: Semantic Analysis
- **Assignment 5**: Code Generation

## ⚖️ License & Academic Integrity

This is an academic project for educational purposes. All code is original work by Group 2.

**Authenticity Declaration**: This submission is the result of our group work and has not been shared with any other groups/students or 3rd party content providers. This submitted piece of work is entirely of our own creation.

## 📞 Contact

**Group 2** - SEP700 Fall 2025  
Seneca College of Applied Arts and Technology

---

**Note**: This project successfully demonstrates complete mastery of compiler front-end construction, including lexical analysis, syntax analysis, and abstract syntax tree generation using modern software engineering practices.
