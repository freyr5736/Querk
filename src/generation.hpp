#pragma once // Ensures this header file is only included once during compilation

#include "parser.hpp"    // Includes the parser, which provides the AST (Abstract Syntax Tree)
#include <sstream>       // Used to construct the output assembly as a string
#include <unordered_map> // Used for storing variables and their stack locations

// ============================= CODE GENERATOR CLASS =============================

// The `generator` class converts the parsed AST into assembly code
class generator {
  public:
    // Constructor: Takes an AST (node_program) as input
    explicit generator(node_program prog) : m_prog(std::move(prog)) {}

    // ============================= EXPRESSION GENERATION =============================

    // Function to generate assembly code for an expression
    void generate_expr(const node_expr &expr) {
        // Define a visitor struct to handle different expression types
        struct expr_visitor {
            generator *gen; // Pointer to the generator instance

            // Handles integer literal expressions (e.g., `5` in `exit(5);`)
            void operator()(const node_expr_int_lit &expr_int_lit) {
                // Move the integer value into register RAX (used for syscall arguments and computation)
                gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << "\n";
                // Push RAX onto the stack to store the computed value
                gen->push("rax");
            }

            // Handles identifier expressions (e.g., `x` in `exit(x);`)
            void operator()(const node_expr_identifier &expr_identifier) {
                // Ensure the variable has been declared before using it
                if (!gen->m_variables.contains(expr_identifier.identifier.value.value())) {
                    std::cerr << "Error: Undeclared Identifier " << expr_identifier.identifier.value.value()
                              << std::endl;
                    exit(EXIT_FAILURE);
                }
                // Retrieve the variable's stack location
                const auto &var = gen->m_variables.at(expr_identifier.identifier.value.value());

                // Corrected offset calculation
                std::stringstream offset;
                offset << "QWORD [rsp + " << ((gen->m_stack_size - var.stack_loc - 1) * 8)
                       << "]"; // No subtraction from stack size
                // Push the variable onto the stack
                gen->push(offset.str());
            }
        };

        expr_visitor visitor{.gen = this}; // Create a visitor instance
        std::visit(visitor, expr.var);     // Apply visitor pattern to handle the expression
    }

    // ============================= STATEMENT GENERATION =============================

    // Function to generate assembly code for a statement
    void generate_statement(const node_statement &stmt) {
        // Define a visitor struct to handle different statement types
        struct statement_visitor {
            generator *gen; // Pointer to the generator instance

            // Handles `exit` statements (e.g., `exit(5);`)
            void operator()(const node_statement_exit &stmt_exit) {
                // Generate code for the expression inside `exit()`
                gen->generate_expr(stmt_exit.expr);
                // Move the syscall number for exit (60) into RAX
                gen->m_output << "    mov rax, 60\n";
                // Pop the expression result from the stack into RDI (exit code argument for syscall)
                gen->pop("rdi");
                // Execute the syscall to terminate the program
                gen->m_output << "    syscall\n";
            }

            // Handles `let` statements (e.g., `let x = 5;`)
            void operator()(const node_statement_let &stmt_let) {
                // Ensure the variable is not already declared
                if (gen->m_variables.contains(stmt_let.ident.value.value())) {
                    std::cerr << "Error: Identifier already exists: " << stmt_let.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // Store the variable in the symbol table with its stack location
                gen->m_variables.insert({stmt_let.ident.value.value(), variable{.stack_loc = gen->m_stack_size}});
                // Generate code for the assigned expression
                gen->generate_expr(stmt_let.expr);
            }
        };

        statement_visitor visitor{.gen = this}; // Create a visitor instance
        std::visit(visitor, stmt);              // Apply visitor pattern to handle the statement
    }

    // ============================= PROGRAM GENERATION =============================

    // Function to generate assembly code for the entire program
    std::string generate_program() {
        std::stringstream output; // String stream to store generated assembly code

        // Start of the assembly program
        m_output << "global _start\n"; // Declares the `_start` entry point for the assembler
        m_output << "_start:\n";       // Defines the `_start` label

        // Generate assembly for each statement in the program
        for (const node_statement &stmt : m_prog.stmts) {
            generate_statement(stmt);
        }

        // Ensure the program exits cleanly in case there is no `exit()` statement
        m_output << "    mov rax, 60\n"; // Move syscall number for `exit` into RAX
        m_output << "    mov rdi, 0\n";  // Set exit status to 0 (successful termination)
        m_output << "    syscall\n";     // Call the `exit` syscall

        return m_output.str(); // Return the generated assembly code as a string
    }

  private:
    // Pushes a value onto the stack and updates the stack size
    void push(const std::string &reg) {
        m_output << "    push " << reg << "\n";
        m_stack_size++;
    }

    // Pops a value from the stack and updates the stack size
    void pop(const std::string &reg) {
        m_output << "    pop " << reg << "\n";
        m_stack_size--;
    }

    // Structure representing a variable in the symbol table
    struct variable {
        size_t stack_loc; // Stack position of the variable
    };

    const node_program m_prog;                               // Stores the parsed program (AST)
    std::stringstream m_output;                              // Used to construct the assembly output
    size_t m_stack_size = 0;                                 // Tracks the current stack size
    std::unordered_map<std::string, variable> m_variables{}; // Symbol table for variable storage
};