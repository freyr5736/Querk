#pragma once // Ensures this header file is only included once during compilation

#include <variant>  // Allows storing multiple types in a single variable
#include <vector>   // Used for storing lists of statements
#include <iostream> // Used for error logging
#include <optional> // Used to represent optional values that may or may not be present

#include "tokenization.hpp" // Includes the tokenization module for handling tokens
#include "storage.hpp"

// ============================= NODE STRUCTURES =============================

// Structure representing an integer literal expression node
// Example: 5 in exit(5);
struct node_term_int_lit {
    token int_lit; // Stores an integer literal token
};

// Structure representing an identifier expression node
// Example: x in let x = 5;
struct node_term_identifier {
    token identifier; // Stores an identifier token
};
struct node_expr;

struct node_binary_expr_add {
    node_expr *lhs;
    node_expr *rhs;
};

struct node_binary_expr {
    node_binary_expr_add *add;
};

struct node_term {
    std::variant<node_term_int_lit *, node_term_identifier *> var;
};

// Structure representing a general expression node, which can be:
// 1. An integer literal (node_expr_int_lit)
// 2. An identifier (node_expr_identifier)
// 3. A binary expression (binary_expr)
struct node_expr {
    std::variant<node_term *, node_binary_expr *> var; // Variant stores either type
};

// Structure representing an exit statement node
// Example: exit(5);
struct node_statement_exit {
    node_expr *expr; // Stores the expression inside exit()
};

// Structure representing a let statement node
// Example: let x = 5;
struct node_statement_let {
    token ident;     // Stores the identifier (x in let x = 5;)
    node_expr *expr; // Stores the assigned expression (5 in let x = 5;)
};

// A node_statement can be one of the following:
// 1. node_statement_exit (for exit() statements)
// 2. node_statement_let (for let statements)
struct node_statement {
    std::variant<node_statement_exit, node_statement_let> var;
};

// Structure representing a complete program containing multiple statements
struct node_program {
    std::vector<node_statement> stmts; // Stores a list of statements in the program
};

// ============================= PARSER CLASS =============================

// The parser class is responsible for converting a list of tokens into an Abstract Syntax Tree (AST)
class parser {
  public:
    // Constructor: Initializes the parser with a vector of tokens
    inline explicit parser(std::vector<token> tokens) : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4) {}

    std::optional<node_binary_expr *> parse_bin_expr() {
        if (auto lhs = parse_expr()) {
            // Implementation missing
        } else {
            std::cerr << "Error: Operator not supported yet" << std::endl;
            exit(EXIT_FAILURE);
        }
        return {};
    }

    std::optional<node_term *> parse_term() {
        // If the next token is an integer literal, parse it as node_term_int_lit
        if (auto int_lit = try_consume(tokentype::int_lit)) {
            auto v_term_int_lit = m_allocator.alloc<node_term_int_lit>();
            v_term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<node_term>();
            term->var = v_term_int_lit;
            return term;
        }
        // If the next token is an identifier, parse it as node_expr_identifier
        else if (auto ident = try_consume(tokentype::ident)) {
            auto v_term_ident = m_allocator.alloc<node_term_identifier>();
            v_term_ident->identifier = ident.value();
            auto term = m_allocator.alloc<node_term>();
            term->var = v_term_ident;
            return term;
        }
        return {};
    }

    std::optional<node_expr *> parse_expr() {
        if (auto term = parse_term()) {
            if (try_consume(tokentype::plus).has_value()) {
                auto bin_expr = m_allocator.alloc<node_binary_expr>();
                auto v_bin_expr_add = m_allocator.alloc<node_binary_expr_add>();
                auto lhs_expr = m_allocator.alloc<node_expr>();
                lhs_expr->var = term.value();
                v_bin_expr_add->lhs = lhs_expr;

                if (auto rhs = parse_expr()) {
                    v_bin_expr_add->rhs = rhs.value();
                    bin_expr->add = v_bin_expr_add;
                    auto expr = m_allocator.alloc<node_expr>();
                    expr->var = bin_expr;
                    return expr;
                } else {
                    std::cerr << "Error: Unexpected Expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            auto expr = m_allocator.alloc<node_expr>();
            expr->var = term.value();
            return expr;
        } else if (auto bin_expr = parse_bin_expr()) {
            auto expr = m_allocator.alloc<node_expr>();
            expr->var = bin_expr.value();
            return expr;
        }
        return {};
    }

    std::optional<node_statement> parse_statement() {
        if (peek().has_value() && peek().value().type == tokentype::exit) {
            try_consume(tokentype::exit, "Error: Expected 'exit' keyword");
            try_consume(tokentype::open_paren, "Error: Expected '(' after 'exit'");

            auto stmt_exit = m_allocator.alloc<node_statement_exit>();
            if (auto expr = parse_expr()) {
                stmt_exit->expr = expr.value();
            } else {
                std::cerr << "Error: Invalid expression inside 'exit()'" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(tokentype::close_paren, "Error: Expected ')' after expression in 'exit()'");
            try_consume(tokentype::semi, "Error: Missing semicolon after 'exit()'");
            return node_statement{*stmt_exit};
        }

        // Handle let statements
        if (peek().has_value() && peek().value().type == tokentype::let) {
            try_consume(tokentype::let, "Error: Expected 'let' keyword");
            auto identifier = try_consume(tokentype::ident, "Error: Expected variable name");
            try_consume(tokentype::equals, "Error: Expected '=' after variable name");

            auto stmt_let = m_allocator.alloc<node_statement_let>();
            stmt_let->ident = identifier;

            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                std::cerr << "Error: Invalid expression in 'let' statement" << std::endl;
                exit(EXIT_FAILURE);
            }

            try_consume(tokentype::semi, "Error: Missing semicolon after 'let' statement");
            return node_statement{*stmt_let};
        }

        return {};
    }

    std::optional<node_program *> parse_prog() {
        auto prog = m_allocator.alloc<node_program>();
        while (peek().has_value()) {
            if (auto stmt = parse_statement()) {
                prog->stmts.push_back(stmt.value());
            } else {
                std::cerr << "Error: Invalid statement in program" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

  private:
    const std::vector<token> m_tokens;
    size_t m_index = 0;

    [[nodiscard]] std::optional<token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size())
            return {};
        return m_tokens.at(m_index + offset);
    }

    token consume() {
        return m_tokens.at(m_index++);
    }

    inline token try_consume(tokentype type, const std::string &err_msg) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<token> try_consume(tokentype type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        return {};
    }

    storage_allocator m_allocator;
};