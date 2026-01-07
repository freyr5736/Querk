# Querk

**Querk** is a single-pass custom compiler built as a learning project to explore low-level concepts and compiler design. The primary goal of this project is to understand how compilers work internally from parsing and semantic analysis to code generation using modern C++.

This project combines **theoretical compiler concepts** with **practical C++ implementation**, focusing on clarity and experimentation rather than production readiness.

---

## Overview

Querk follows a **single-pass compilation model** and is structured around an **Abstract Syntax Tree (AST)**. The compiler parses input, builds semantic meaning through AST nodes, and generates **NASM assembly**, which is currently hardcoded to keep the focus on learning core compiler mechanics.

While the foundation of this project was inspired by a YouTube tutorial by Pixeled, it has been **extended and modified** with custom features and design choices to deepen understanding and experimentation.

> **Note:** This project is still under active development. Features are added incrementally as learning progresses.

---

## Key Concepts Explored

* Compiler architecture (single-pass design)
* Abstract Syntax Trees (AST)
* Node-based semantic modeling
* Operator precedence handling
* Variable scoping and shadowing
* Low-level assembly generation (NASM)
* Practical C++ project structure

---

## Features Implemented So Far

* CMake-based build system
* Abstract Syntax Tree (AST) and node architecture
* Operator precedence handling
* `exit` statement support
* `if` control flow
* Variable declarations and usage
* Shadow scoping
* Comment handling

---

## Project Structure

* **AST & Nodes**
  Used to represent syntax and semantics in a structured, extensible way.

* **Semantic Execution**
  NASM assembly generation is currently hardcoded to simplify experimentation and focus on compiler flow.

* **Readable Flow**
  The codebase includes self-explanatory comments to make the compilation pipeline easy to follow.

---

## Disclaimer

This is **not a production-ready compiler**. It is an educational project designed to explore compiler internals and low-level programming concepts.

An instruction manual is included explaining how to run the project. It is developed in a Linux environment and is intended to run on Linux only. Prototypes and intermediate versions created throughout the project’s duration are also included.

---
