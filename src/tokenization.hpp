#pragma once // Ensures this header file is processed only once during
             // compilation

#include <cctype>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Enum representing different types of tokens
enum class tokentype {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    equals,
    plus,
    star,
    minus,
    div,
    modu,
    open_curly,
    close_curly,
    if_
};

// Token structure representing a token with its type and optional value
struct token {
    tokentype type;                   // Type of the token
    std::optional<std::string> value; // Optional string value, used only for
                                      // identifiers and integer literals
};

std::optional<int> binary_precedence(tokentype type) {
    switch (type) {

    case tokentype::minus:
    case tokentype::plus:
        return 0;
    case tokentype::div:
    case tokentype::modu:
    case tokentype::star:
        return 1;
    default:
        return {};
    }
}

class tokenizer {
  public:
    // Constructor: Initializes tokenizer with source code string
    inline tokenizer(std::string src)
        : m_src(std::move(
              src)) // Move the input string to avoid unnecessary copying
    {}

    // Function to tokenize the input source code
    inline std::vector<token> tokenize() {
        std::string tkn;           // Stores characters of the current token
        std::vector<token> tokens; // Stores all parsed tokens

        // Loop while there are characters to process
        while (peek().has_value()) {

            // for comments
            if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
                continue;
            }

            // For words
            char current = peek().value();
            if (std::isalpha(current)) { // Check if character is alphabetic
                tkn.push_back(
                    consume()); // Append character and advance position

                // Continue consuming alphanumeric characters (identifiers or
                // keywords)
                while (peek().has_value() && std::isalnum(peek().value())) {
                    tkn.push_back(consume());
                }

                // Check for keywords
                if (tkn == "exit" || tkn == "let") {
                    tokens.push_back({.type = (tkn == "exit")
                                                  ? tokentype::exit
                                                  : tokentype::let});
                } else if (tkn == "if") {
                    tokens.push_back({.type = tokentype::if_});
                    tkn.clear();
                }

                else {
                    tokens.push_back({.type = tokentype::ident, .value = tkn});
                }
                tkn.clear();
                continue;
            }

            // For symbols
            switch (current) {
            case ')':
                consume();
                tokens.push_back({.type = tokentype::close_paren});
                break;
            case '(':
                consume();
                tokens.push_back({.type = tokentype::open_paren});
                break;
            case ';':
                consume();
                tokens.push_back({.type = tokentype::semi});
                break;
            case '=':
                consume();
                tokens.push_back({.type = tokentype::equals});
                break;
            case '+':
                consume();
                tokens.push_back({.type = tokentype::plus});
                break;
            case '*':
                consume();
                tokens.push_back({.type = tokentype::star});
                break;
            case '-':
                consume();
                tokens.push_back({.type = tokentype::minus});
                break;
            case '/':
                consume();
                tokens.push_back({.type = tokentype::div});
                break;
            case '%':
                consume();
                tokens.push_back({.type = tokentype::modu});
                break;
            case '{':
                consume();
                tokens.push_back({.type = tokentype::open_curly});
                break;
            case '}':
                consume();
                tokens.push_back({.type = tokentype::close_curly});
                break;
            default:
                if (std::isdigit(current)) {
                    tkn.push_back(consume());
                    while (peek().has_value() && std::isdigit(peek().value())) {
                        tkn.push_back(consume());
                    }
                    tokens.push_back(
                        {.type = tokentype::int_lit, .value = tkn});
                    tkn.clear();
                } else if (std::isspace(current)) {
                    consume();
                } else {
                    std::cerr << "Error: Unrecognized character '" << current
                              << "'" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        return tokens; // Return the list of parsed tokens
    }

  private:
    // Function to peek ahead in the source string without consuming characters
    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {}; // Return empty optional if out of bounds
        } else {
            return m_src.at(m_index +
                            offset); // Return character at current position
        }
    }

    // Function to consume a character and move to the next one
    inline char consume() {
        return m_src.at(
            m_index++); // Return current character and increment index
    }

    const std::string m_src; // Source code string
    size_t m_index = 0;      // Current position in the source code
};
