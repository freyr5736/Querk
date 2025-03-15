#pragma once // Ensures this header file is processed only once during compilation

#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <cctype>

// Enum representing different types of tokens
enum class tokentype { exit, int_lit, semi, open_paren, close_paren, ident, let, equals, plus, star, minus, div, modu };

// Token structure representing a token with its type and optional value
struct token {
    tokentype type;                   // Type of the token
    std::optional<std::string> value; // Optional string value, used only for identifiers and integer literals
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
        : m_src(std::move(src)) // Move the input string to avoid unnecessary copying
    {}

    // Function to tokenize the input source code
    // with if-else statements
    // inline std::vector<token> tokenize() {
    //     std::string tkn;           // Stores characters of the current token
    //     std::vector<token> tokens; // Stores all parsed tokens

    //     // Loop while there are characters to process
    //     while (peek().has_value()) {
    //         if (std::isalpha(peek().value())) { // Check if character is alphabetic
    //             tkn.push_back(consume());       // Append character and advance position

    //             // Continue consuming alphanumeric characters (identifiers or keywords)
    //             while (peek().has_value() && std::isalnum(peek().value())) {
    //                 tkn.push_back(consume());
    //             }

    //             // Check for keywords
    //             if (tkn == "exit") {
    //                 tokens.push_back({.type = tokentype::exit}); // Store 'exit' keyword token
    //                 tkn.clear();
    //             } else if (tkn == "let") {
    //                 tokens.push_back({.type = tokentype::let}); // Store 'let' keyword token
    //                 tkn.clear();
    //             } else {
    //                 // Store identifier token
    //                 tokens.push_back({.type = tokentype::ident, .value = tkn});
    //                 tkn.clear();
    //             }
    //         } else if (std::isdigit(peek().value())) { // Check if character is a digit (integer literal)
    //             tkn.push_back(consume());

    //             // Continue consuming numeric characters
    //             while (peek().has_value() && std::isdigit(peek().value())) {
    //                 tkn.push_back(consume());
    //             }

    //             tokens.push_back({.type = tokentype::int_lit, .value = tkn}); // Store integer literal token
    //             tkn.clear();
    //         }

    //         // Check for parentheses ( ) and store them as tokens
    //         else if (peek().value() == ')') {
    //             consume();
    //             tokens.push_back({.type = tokentype::close_paren});
    //         } else if (peek().value() == '(') {
    //             consume();
    //             tokens.push_back({.type = tokentype::open_paren});
    //         }

    //         // Check for semicolon (;)
    //         else if (peek().value() == ';') {
    //             consume();                                   // Consume semicolon character
    //             tokens.push_back({.type = tokentype::semi}); // Store semicolon token
    //         }

    //         // Check for equals (=)
    //         else if (peek().value() == '=') {
    //             consume();                                     // Consume equals character
    //             tokens.push_back({.type = tokentype::equals}); // Store equals token
    //         }

    //         // Check for plus (+)
    //         else if (peek().value() == '+') {
    //             consume();                                   // Consume plus character
    //             tokens.push_back({.type = tokentype::plus}); // Store plus token
    //         }

    //         // Check for astrisk (*)
    //         else if (peek().value() == '*') {
    //             consume();                                   // Consume astrisk character
    //             tokens.push_back({.type = tokentype::star}); // Store astrisk token
    //         }

    //         // Check for astrisk (-)
    //         else if (peek().value() == '-') {
    //             consume();                                    // Consume minus character
    //             tokens.push_back({.type = tokentype::minus}); // Store minus token
    //         }

    //         // Check for astrisk (/)
    //         else if (peek().value() == '/') {
    //             consume();                                  // Consume slash character
    //             tokens.push_back({.type = tokentype::div}); // Store slash token
    //         }

    //          //Check for modu (%)
    //         else if (peek().value() == '%') {
    //             consume();                                    // Consume modulo character
    //             tokens.push_back({.type = tokentype::modu}); // Store modulo token
    //         }

    //         // Ignore whitespace (spaces, newlines, tabs)
    //         else if (std::isspace(peek().value())) {
    //             consume();
    //         }

    //         // Unrecognized character error
    //         else {
    //             std::cerr << "Error: Unrecognized character '" << peek().value() << "'" << std::endl;
    //             exit(EXIT_FAILURE);
    //         }
    //     }

    //     return tokens; // Return the list of parsed tokens
    // }

    // Function to tokenize the input source code
    inline std::vector<token> tokenize() {
        std::string tkn;           // Stores characters of the current token
        std::vector<token> tokens; // Stores all parsed tokens

        // Loop while there are characters to process
        while (peek().has_value()) {
            char current = peek().value();

            if (std::isalpha(current)) {  // Check if character is alphabetic
                tkn.push_back(consume()); // Append character and advance position

                // Continue consuming alphanumeric characters (identifiers or keywords)
                while (peek().has_value() && std::isalnum(peek().value())) {
                    tkn.push_back(consume());
                }

                // Check for keywords
                if (tkn == "exit" || tkn == "let") {
                    tokens.push_back({.type = (tkn == "exit") ? tokentype::exit : tokentype::let});
                } else {
                    tokens.push_back({.type = tokentype::ident, .value = tkn});
                }
                tkn.clear();
                continue;
            }

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
            default:
                if (std::isdigit(current)) {
                    tkn.push_back(consume());
                    while (peek().has_value() && std::isdigit(peek().value())) {
                        tkn.push_back(consume());
                    }
                    tokens.push_back({.type = tokentype::int_lit, .value = tkn});
                    tkn.clear();
                } else if (std::isspace(current)) {
                    consume();
                } else {
                    std::cerr << "Error: Unrecognized character '" << current << "'" << std::endl;
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
            return m_src.at(m_index + offset); // Return character at current position
        }
    }

    // Function to consume a character and move to the next one
    inline char consume() {
        return m_src.at(m_index++); // Return current character and increment index
    }

    const std::string m_src; // Source code string
    size_t m_index = 0;      // Current position in the source code
};
