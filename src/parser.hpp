#pragma once // Ensures this header file is only included once during compilation

#include <variant>  // Allows storing multiple types in a single variable
#include <vector>   // Used for storing lists of statements
#include <iostream> // Used for error logging
#include <optional> // Used to represent optional values that may or may not be present

#include "tokenization.hpp" // Includes the tokenization module for handling tokens

// ============================= NODE STRUCTURES =============================

// Structure representing an integer literal expression node
// Example: `5` in `exit(5);`
struct node_expr_int_lit {
    token int_lit; // Stores an integer literal token
};

// Structure representing an identifier expression node
// Example: `x` in `let x = 5;`
struct node_expr_identifier {
    token identifier; // Stores an identifier token
};

// Structure representing a general expression node, which can be:
// 1. An integer literal (`node_expr_int_lit`)
// 2. An identifier (`node_expr_identifier`)
struct node_expr {
    std::variant<node_expr_int_lit, node_expr_identifier> var; // Variant stores either type
};

// Structure representing an `exit` statement node
// Example: `exit(5);`
struct node_statement_exit {
    node_expr expr; // Stores the expression inside `exit()`
};

// Structure representing a `let` statement node
// Example: `let x = 5;`
struct node_statement_let {
    token ident;    // Stores the identifier (`x` in `let x = 5;`)
    node_expr expr; // Stores the assigned expression (`5` in `let x = 5;`)
};

// A `node_statement` can be one of the following:
// 1. `node_statement_exit` (for `exit()` statements)
// 2. `node_statement_let` (for `let` statements)
using node_statement = std::variant<node_statement_exit, node_statement_let>;

// Structure representing a complete program containing multiple statements
struct node_program {
    std::vector<node_statement> stmts; // Stores a list of statements in the program
};

// ============================= PARSER CLASS =============================

// The `parser` class is responsible for converting a list of tokens into an Abstract Syntax Tree (AST)
class parser {
  public:
    // Constructor: Initializes the parser with a vector of tokens
    explicit parser(std::vector<token> tokens) : m_tokens(std::move(tokens)) {}

    // Function to parse an expression (either an integer literal or an identifier)
    std::optional<node_expr> parse_expr() {
        // If the next token is an integer literal, parse it as `node_expr_int_lit`
        if (peek().has_value() && peek().value().type == tokentype::int_lit) {
            return node_expr{.var = node_expr_int_lit{.int_lit = consume()}};
        }
        // If the next token is an identifier, parse it as `node_expr_identifier`
        else if (peek().has_value() && peek().value().type == tokentype::ident) {
            return node_expr{.var = node_expr_identifier{.identifier = consume()}};
        }
        return {}; // Return empty optional if no valid expression is found
    }

    // Function to parse a statement (either an `exit` statement or a `let` statement)
    std::optional<node_statement> parse_statement() {
        // =================== Handling `exit()` Statements ===================
        // Example: `exit(5);`
        if (peek().has_value() && peek().value().type == tokentype::exit && peek(1).has_value() &&
            peek(1).value().type == tokentype::open_paren) {

            consume(); // Consume 'exit' keyword
            consume(); // Consume '(' token

            node_statement_exit stmt_exit;

            // Parse the expression inside `exit()`
            if (auto expr = parse_expr()) {
                stmt_exit.expr = expr.value();
            } else {
                std::cerr << "Error: Invalid expression inside 'exit()'" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Ensure closing parenthesis `)` exists
            if (peek().has_value() && peek().value().type == tokentype::close_paren) {
                consume(); // Consume `)`
            } else {
                std::cerr << "Error: Expected ')' after expression in 'exit()'" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Ensure semicolon `;` exists at the end
            if (peek().has_value() && peek().value().type == tokentype::semi) {
                consume(); // Consume `;`
            } else {
                std::cerr << "Error: Missing semicolon after 'exit()'" << std::endl;
                exit(EXIT_FAILURE);
            }

            return stmt_exit; // Return the parsed exit statement
        }

        // =================== Handling `let` Statements ===================
        // Example: `let x = 5;`
        else if (peek().has_value() && peek().value().type == tokentype::let && peek(1).has_value() &&
                 peek(1).value().type == tokentype::ident && peek(2).has_value() &&
                 peek(2).value().type == tokentype::equals) {

            consume(); // Consume 'let' keyword

            node_statement_let stmt_let;
            stmt_let.ident = consume(); // Consume identifier (`x` in `let x = 5;`)
            consume();                  // Consume '=' token

            // Parse the right-hand expression (`5` in `let x = 5;`)
            if (auto expr = parse_expr()) {
                stmt_let.expr = expr.value();
            } else {
                std::cerr << "Error: Invalid expression in 'let' statement" << std::endl;
                exit(EXIT_FAILURE);
            }

            // Ensure semicolon `;` exists at the end
            if (peek().has_value() && peek().value().type == tokentype::semi) {
                consume(); // Consume `;`
            } else {
                std::cerr << "Error: Expected ';' after 'let' statement" << std::endl;
                exit(EXIT_FAILURE);
            }

            return stmt_let; // Return the parsed let statement
        }

        return {}; // Return empty optional if no valid statement is found
    }

    // Function to parse an entire program containing multiple statements
    std::optional<node_program> parse_prog() {
        node_program prog; // Create an empty program structure

        // Keep parsing statements until all tokens are consumed
        while (peek().has_value()) {
            if (auto stmt = parse_statement()) {
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "Error: Invalid statement in program" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog; // Return the parsed program
    }

  private:
    const std::vector<token> m_tokens; // Stores the input tokens
    size_t m_index = 0;                // Index to track the current position in the token list

    // Function to look ahead at tokens without consuming them
    [[nodiscard]] std::optional<token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size())
            return {}; // Return empty optional if out of bounds
        return m_tokens.at(m_index + offset);
    }

    // Function to consume the current token and move to the next one
    token consume() {
        return m_tokens.at(m_index++);
    }
};
