#pragma once // Ensures this header file is processed only once during compilation

#include "tokenization.hpp" // Include tokenization header file

// Structure representing an integer literal expression node
struct node_expr {
    token int_lit; // Stores an integer literal token
};

// Structure representing an exit node in the AST
struct node_exit {
    node_expr expr; // Stores an expression related to the exit command
};

// Parser class responsible for parsing tokens into an AST (Abstract Syntax Tree)
class parser {
  public:
    // Constructor: Initializes the parser with a vector of tokens
    inline explicit parser(std::vector<token> tokens) : m_tokens(std::move(tokens)) {}

    // Function to parse an integer literal expression
    std::optional<node_expr> parse_expr() {
        // Check if the current token is an integer literal
        if (peak().has_value() && peak().value().type == tokentype::int_lit) {
            return node_expr{.int_lit = consume()}; // Consume the token and return as node_expr
        } else {
            return {}; // Return empty optional if no valid expression found
        }
    }

    // Function to parse an exit statement
    std::optional<node_exit> parse() {
        std::optional<node_exit> exit_node;

        // Loop while there are tokens to process
        while (peak().has_value()) {
            // Check if the token is an exit keyword
            if (peak().value().type == tokentype::exit) {
                consume(); // Consume the exit token

                // Parse the expression following the exit keyword
                if (auto node_expr = parse_expr()) {
                    exit_node = node_exit{.expr = node_expr.value()}; // Store parsed expression
                } else {
                    std::cerr << "Error: Invalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                // Ensure the expression is followed by a semicolon
                if (peak().has_value() && peak().value().type == tokentype::semi) {
                    consume();
                } else {
                    std::cerr << "Error: Missing semicolon" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            m_index = 0;      // Reset index for potential re-parsing
            return exit_node; // Return parsed exit node
        }
        return {}; // Return empty optional if no exit statement found
    }

  private:
    const std::vector<token> m_tokens; // Vector storing input tokens
    size_t m_index = 0;                // Current position in the token list

    // Function to peek at the next token without consuming it
    [[nodiscard]] inline std::optional<token> peak(int ahead = 0) const {
        if (m_index + ahead >= m_tokens.size()) {
            return {}; // Return empty optional if out of bounds
        } else {
            return m_tokens.at(m_index + ahead); // Return token at current position
        }
    }

    // Function to consume a token and move to the next one
    inline token consume() {
        return m_tokens.at(m_index++);
    }
};
