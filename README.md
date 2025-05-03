# Kaleidoscope_Frontend_Compiler
This repository designs a frontend compiler that compiles the language into a LLVM Intermediate Representation(IR). The llvm IR can then be compiled with llvm to get the desired output of the inputted code in Kaleidoscope language .

## Overview

The compiler processes Kaleidoscope source code, generating LLVM Intermediate Representation (IR). Key components include:

-   **Lexer:** Tokenizes the Kaleidoscope source code into recognizable units like keywords, identifiers, and numbers. This forms the foundation for syntax analysis by breaking raw input into manageable pieces.
-   **Parser:** Converts the sequence of tokens into a structured Abstract Syntax Tree (AST) that reflects the program's grammar. It handles expressions, definitions, and control flow constructs for further semantic processing.
-   **AST:** Represents the hierarchical structure of the program using nodes such as `FunctionAST` and `BinaryExprAST`. This intermediate structure allows easier transformation into backend representations.
-   **LLVM IR Generation:** Walks through the AST and translates it into LLVM Intermediate Representation (IR), a low-level platform-independent code format. The IR enables optimization and execution using the LLVM infrastructure.

## Kaleidoscope Language

-   Procedural language with function definitions and calls. 
-   Supports conditionals (`if`), loops (`for`), and basic math. 
-   Single data type: `double`. 
-   Allows external function declarations (`extern`). 

## LLVM

-   LLVM is used for its modularity, portability, and JIT compilation capabilities. 
-   The compiler frontend generates LLVM IR, which LLVM can then optimize and compile. 

## Example
**Source Code** 
```
def sq(b) b*b;
```
**LLVM IR**
```
define double @sq(double %b) {
entry:
  %multmp = fmul double %b, %b
  ret double %multmp
}
```

## Run it locally ...!
### 📥 Clone the Repository

```bash
git clone https://github.com/your-username/llvm-toy-compiler.git
cd llvm-toy-compiler
```

### ⚙️ Prerequisites

Ensure the following are installed on your system:

- **macOS** (recommended)
- **Homebrew** (for macOS)
- **LLVM** (version ≥ 13)
- **Clang++** or **g++** (with C++17 support)

- Install LLVM
```bash
brew install llvm
```

- Add LLVM to Your Environment

- Install Dependencies

### 🛠️ Build the Compiler

#### Using `clang++`
```bash
clang++ -std=c++17 main.cpp lexer.cc codegen.cc -o toy_compiler 
```

#### Or using `g++`
```bash
g++ -std=c++17 main.cpp lexer.cc codegen.cc -o toy_compiler 
```

### 🚀 Run the Compiler
After building, run the executable:
```bash
./toy_compiler
```
