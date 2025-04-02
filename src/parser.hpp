#pragma once // Ensures this header file is only included once during compilation

#include <variant>  // Allows storing multiple types in a single variable
#include <vector>   // Used for storing lists of statements
#include <iostream> // Used for error logging
#include <optional> // Used to represent optional values that may or may not be present
#include <cassert>

#include "tokenization.hpp" // Includes the tokenization module for handling tokens
#include "storage.hpp"

// ============================= NODE STRUCTURES =============================

struct node_expr;

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

struct node_term_parentheses {
    node_expr *expr;
};

// Structure representing addition in binary expression node
// Example: 5+5 in  let x = 5 + 5;
struct node_binary_expr_add {
    node_expr *lhs;
    node_expr *rhs;
};

// Structure representing an multiplication expression node
// Example: 5*5 in let x = 5*5;
struct node_binary_expr_multiply {
    node_expr *lhs;
    node_expr *rhs;
};

// Structure representing an subtraction expression node
// Example: 5-5 in let x = 5-5;
struct node_binary_expr_minus {
    node_expr *lhs;
    node_expr *rhs;
};

// Structure representing an division expression node
// Example: 5/5 in let x = 5/5;
struct node_binary_expr_divide {
    node_expr *lhs;
    node_expr *rhs;
};

// Structure representing an division expression node
// Example: 5%5 in let x = 5%5;
struct node_binary_expr_modulus {
    node_expr *lhs;
    node_expr *rhs;
};

// Structure representing an binary expression node
// Example let x = 5 + 5;
struct node_binary_expr {
    std::variant<node_binary_expr_add *, node_binary_expr_multiply *, node_binary_expr_minus *,
                 node_binary_expr_divide *, node_binary_expr_modulus *>
        var;
};

// Structure representing a term
// Example: let x = 5;
struct node_term {
    std::variant<node_term_int_lit *, node_term_identifier *, node_term_parentheses *> var;
};

// Structure representing a general expression node, which can be:
// 1. A term
// 2. A binary expression
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

struct node_statement;

struct node_scope {
    std::vector<node_statement *> stmts;
};

struct node_statement_if {
    node_expr *expr;
    node_scope *scope;
};

// A node_statement can be one of the following:
// 1. node_statement_exit (for exit() statements)
// 2. node_statement_let (for let statements)
struct node_statement {
    std::variant<node_statement_exit, node_statement_let, node_scope *, node_statement_if *> var;
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
        } else if (auto open_paren = try_consume(tokentype::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                std::cerr << "Error: Expected expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(tokentype::close_paren, "Error: Expected ')'");
            auto term_paren = m_allocator.alloc<node_term_parentheses>();
            term_paren->expr = expr.value();
            auto term = m_allocator.alloc<node_term>();
            term->var = term_paren;
            return term;
        } else {
            return {};
        }
    }

    std::optional<node_expr *> parse_expr(int min_prec = 0) {
        std::optional<node_term *> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }

        auto expr_lhs = m_allocator.alloc<node_expr>();
        expr_lhs->var = term_lhs.value();

        // precedence calculator
        while (true) {
            std::optional<token> curr_tkn = peek();
            std::optional<int> prec;
            if (curr_tkn.has_value()) {
                prec = binary_precedence(curr_tkn->type);
                if (!prec.has_value() || prec.value() < min_prec) {
                    break;
                }
            } else {
                break;
            }
            token op = consume();
            int v_next_min_prec = prec.value() + 1;
            auto expr_rhs = parse_expr(v_next_min_prec);
            if (!expr_rhs.has_value()) {
                std::cerr << "Error: Unable to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            auto expr = m_allocator.alloc<node_binary_expr>();
            auto expr_lhs2 = m_allocator.alloc<node_expr>();
            if (op.type == tokentype::plus) {
                auto add = m_allocator.alloc<node_binary_expr_add>();
                expr_lhs2->var = expr_lhs->var;
                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                expr->var = add;
            } else if (op.type == tokentype::star) {
                auto multi = m_allocator.alloc<node_binary_expr_multiply>();
                expr_lhs2->var = expr_lhs->var;
                multi->lhs = expr_lhs2;
                multi->rhs = expr_rhs.value();
                expr->var = multi;
            } else if (op.type == tokentype::minus) {
                auto minus = m_allocator.alloc<node_binary_expr_minus>();
                expr_lhs2->var = expr_lhs->var;
                minus->lhs = expr_lhs2;
                minus->rhs = expr_rhs.value();
                expr->var = minus;
            } else if (op.type == tokentype::div) {
                auto div = m_allocator.alloc<node_binary_expr_divide>();
                expr_lhs2->var = expr_lhs->var;
                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                expr->var = div;
            } else if (op.type == tokentype::modu) {
                auto modu = m_allocator.alloc<node_binary_expr_modulus>();
                expr_lhs2->var = expr_lhs->var;
                modu->lhs = expr_lhs2;
                modu->rhs = expr_rhs.value();
                expr->var = modu;
            } else {
                assert(false);
            }
            expr_lhs->var = expr;
        }

        return expr_lhs;
    }

    std::optional<node_scope *> parse_scope() {
        if (!try_consume(tokentype::open_curly).has_value()) {
            return {};
        }
        auto scope = m_allocator.alloc<node_scope>();
        while (auto stmt = parse_statement()) {
            scope->stmts.push_back(stmt.value());
        }
        try_consume(tokentype::close_curly, "Error: Expected '}'");
        return scope;
    }

    std::optional<node_statement *> parse_statement() {
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
            auto stmt = m_allocator.alloc<node_statement>();
            stmt->var = *stmt_exit;
            return stmt;
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
            auto stmt = m_allocator.alloc<node_statement>();
            stmt->var = *stmt_let;
            return stmt;
        } else if (peek().has_value() && peek().value().type == tokentype::open_curly) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.alloc<node_statement>();
                stmt->var = scope.value();
                return stmt;
            } else {
                std::cerr << "Error: Invalid scope" << std::endl;
                exit(EXIT_FAILURE);
            }

        } else if (auto if_ = try_consume(tokentype::if_)) {
            try_consume(tokentype::open_paren, "Error: Expected'('");
            auto stmt_if = m_allocator.alloc<node_statement_if>();
            if (auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            } else {
                std::cerr << "Error: Invalid expression in 'let' statement" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(tokentype::close_paren, "Error: Expected')'");
            if (auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            } else {
                std::cerr << "Error: Invalid scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto stmt = m_allocator.alloc<node_statement>();
            stmt->var = stmt_if;
            return stmt;
        }

        return {};
    }

    std::optional<node_program *> parse_prog() {
        auto prog = m_allocator.alloc<node_program>();
        while (peek().has_value()) {
            if (auto stmt = parse_statement()) {
                prog->stmts.push_back(*stmt.value());
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
