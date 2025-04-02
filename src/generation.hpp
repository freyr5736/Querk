#pragma once // Ensures this header file is only included once during compilation

#include "parser.hpp" // Includes the parser, which provides the AST (Abstract Syntax Tree)
#include <sstream>    // Used to construct the output assembly as a string
#include <vector>     // Used for storing variables and their stack locations
#include <assert.h>
#include <algorithm>

// ============================= CODE GENERATOR CLASS =============================

// The generator class converts the parsed AST into assembly code
class generator {
  public:
    // Constructor: Takes an AST (node_program) as input
    explicit generator(node_program prog) : m_prog(std::move(prog)) {}

    void generate_binary_expr(const node_binary_expr *bin_expr) {
        struct binary_expr_visitor {
            generator *gen;

            void operator()(const node_binary_expr_minus *minus) const {
                gen->generate_expr(minus->rhs);
                gen->generate_expr(minus->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    sub rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const node_binary_expr_add *add) const {
                gen->generate_expr(add->rhs);
                gen->generate_expr(add->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx\n";
                gen->push("rax");
            }

            void operator()(const node_binary_expr_multiply *multi) const {
                gen->generate_expr(multi->rhs);
                gen->generate_expr(multi->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    mul rbx\n";
                gen->push("rax");
            }

            void operator()(const node_binary_expr_divide *div) const {
                gen->generate_expr(div->rhs);
                gen->generate_expr(div->lhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    div rbx\n";
                gen->push("rax");
            }
            void operator()(const node_binary_expr_modulus *modu) const {
                gen->generate_expr(modu->rhs);     // Evaluate RHS first (divisor)
                gen->generate_expr(modu->lhs);     // Then LHS (dividend)
                gen->pop("rax");                   // Pop dividend (numerator) into RAX
                gen->pop("rbx");                   // Pop divisor into RBX
                gen->m_output << "    cqo\n";      // Sign-extend RAX into RDX:RAX for division
                gen->m_output << "    idiv rbx\n"; // Perform signed division
                gen->push("rdx");                  // Push remainder (modulus result) onto the stack
            }
        };
        binary_expr_visitor visitor{.gen = this};
        std::visit(visitor, bin_expr->var);
    }

    void generate_term(const node_term *term) {
        struct term_visitor {
            generator *gen;

            void operator()(const node_term_int_lit *term_int_lit) const {
                // Move the integer value into register RAX (used for syscall arguments and computation)
                gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
                // Push RAX onto the stack to store the computed value
                gen->push("rax");
            }
            void operator()(const node_term_identifier *term_ident) const {
                // Ensure the variable has been declared before using it
                auto it = std::find_if(gen->m_variables.cbegin(), gen->m_variables.cend(), [&](const variable &var) {
                    return var.name == term_ident->identifier.value.value();
                });
                if (it == gen->m_variables.cend()) {
                    std::cerr << "Error: Undeclared Identifier " << term_ident->identifier.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // Retrieve the variable's stack location
                const auto &var = (*it);

                // Corrected offset calculation
                std::stringstream offset;
                offset << "QWORD [rsp + " << ((gen->m_stack_size - (*it).stack_loc - 1) * 8)
                       << "]"; // No subtraction from stack size
                // Push the variable onto the stack
                gen->push(offset.str());
            }

            void operator()(const node_term_parentheses *term_paren) const {
                gen->generate_expr(term_paren->expr);
            }
        };
        term_visitor visitor{.gen = this};
        std::visit(visitor, term->var);
    }

    // ============================= EXPRESSION GENERATION =============================

    // Function to generate assembly code for an expression
    void generate_expr(const node_expr *expr) {
        // Define a visitor struct to handle different expression types
        struct expr_visitor {
            generator *gen; // Pointer to the generator instance

            void operator()(const node_term *term) const {
                gen->generate_term(term);
            }

            // Handles binary expressions (e.g., addition, multiplication)
            void operator()(const node_binary_expr *bin_expr) const {
                gen->generate_binary_expr(bin_expr);
            }
        };

        expr_visitor visitor{.gen = this}; // Create a visitor instance
        std::visit(visitor, expr->var);    // Apply visitor pattern to handle the expression
    }

    // ============================= STATEMENT GENERATION =============================

    void generate_scope(const node_scope *scope) {
        begin_scope();
        for (const node_statement *stmt : scope->stmts) {
            generate_statement(*stmt);
        }
        end_scope();
    }

    // Function to generate assembly code for a statement
    void generate_statement(const node_statement &stmt) {
        // Define a visitor struct to handle different statement types
        struct statement_visitor {
            generator *gen; // Pointer to the generator instance

            // Handles exit statements (e.g., exit(5);)
            void operator()(const node_statement_exit &stmt_exit) {
                // Generate code for the expression inside exit()
                gen->generate_expr(stmt_exit.expr);
                // Move the syscall number for exit (60) into RAX
                gen->m_output << "    mov rax, 60\n";
                // Pop the expression result from the stack into RDI (exit code argument for syscall)
                gen->pop("rdi");
                // Execute the syscall to terminate the program
                gen->m_output << "    syscall\n";
            }

            // Handles let statements (e.g., let x = 5;)
            void operator()(const node_statement_let &stmt_let) {
                // Ensure the variable is not already declared
                auto it = std::find_if(gen->m_variables.cbegin(), gen->m_variables.cend(),
                                       [&](const variable &var) { return var.name == stmt_let.ident.value.value(); });
                if (it != gen->m_variables.cend()) {
                    std::cerr << "Error: Identifier already exists: " << stmt_let.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }
                // Store the variable in the symbol table with its stack location
                gen->m_variables.push_back({.name = stmt_let.ident.value.value(), .stack_loc = gen->m_stack_size});

                // Generate code for the assigned expression
                gen->generate_expr(stmt_let.expr);
            }

            void operator()(const node_scope *scope) const {
                gen->generate_scope(scope);
            }

            void operator()(const node_statement_if *stmt_if) {
                gen->generate_expr(stmt_if->expr);
                gen->pop("rax");
                std::string label = gen->create_label();
                gen->m_output << "    test rax, rax\n";
                gen->m_output << "    jz " << label << "\n";
                gen->generate_scope(stmt_if->scope);
                gen->m_output << label << ":\n";
            }
        };

        statement_visitor visitor{.gen = this}; // Create a visitor instance
        std::visit(visitor, stmt.var);          // Apply visitor pattern to handle the statement
    }

    // ============================= PROGRAM GENERATION =============================

    // Function to generate assembly code for the entire program
    std::string generate_program() {
        std::stringstream output; // String stream to store generated assembly code

        // Start of the assembly program
        m_output << "global _start\n"; // Declares the _start entry point for the assembler
        m_output << "_start:\n";       // Defines the _start label

        // Generate assembly for each statement in the program
        for (const node_statement &stmt : m_prog.stmts) {
            generate_statement(stmt);
        }

        // Ensure the program exits cleanly in case there is no exit() statement
        m_output << "    mov rax, 60\n"; // Move syscall number for exit into RAX
        m_output << "    mov rdi, 0\n";  // Set exit status to 0 (successful termination)
        m_output << "    syscall\n";     // Call the exit syscall

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

    void begin_scope() {
        m_scopes.push_back(m_variables.size());
    }

    void end_scope() {
        size_t pop_count = m_variables.size() - m_scopes.back();
        m_output << "    add rsp, " << pop_count * 8 << "\n"; // add becasue stack grows from top
        m_stack_size -= pop_count;

        for (int i = 0; i < pop_count; ++i) {
            m_variables.pop_back();
        }
        m_scopes.pop_back();
    }

    std::string create_label() {
        return "label" + std::to_string(m_label_count);
    }

    // Structure representing a variable in the symbol table
    struct variable {
        std::string name;
        size_t stack_loc; // Stack position of the variable
    };

    const node_program m_prog;           // Stores the parsed program (AST)
    std::stringstream m_output;          // Used to construct the assembly output
    size_t m_stack_size = 0;             // Tracks the current stack size
    std::vector<variable> m_variables{}; // Symbol table for variable storage
    std::vector<size_t> m_scopes{};      // stores the scopes
    int m_label_count = 0;               // stores number of labels
};
