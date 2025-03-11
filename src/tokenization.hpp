#pragma once // Ensures this header file is processed only once during compilation

#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <cctype>

// Enum representing different types of tokens
enum class tokentype { exit, int_lit, semi, open_paren, close_paren, ident, let, equals, plus };

// Token structure representing a token with its type and optional value
struct token {
    tokentype type;                   // Type of the token
    std::optional<std::string> value; // Optional string value, used only for identifiers and integer literals
};

class tokenizer {
  public:
    // Constructor: Initializes tokenizer with source code string
    inline tokenizer(std::string src)
        : m_src(std::move(src)) // Move the input string to avoid unnecessary copying
    {}

    // Function to tokenize the input source code
    inline std::vector<token> tokenize() {
        std::string tkn;           // Stores characters of the current token
        std::vector<token> tokens; // Stores all parsed tokens

        // Loop while there are characters to process
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) { // Check if character is alphabetic
                tkn.push_back(consume());       // Append character and advance position

                // Continue consuming alphanumeric characters (identifiers or keywords)
                while (peek().has_value() && std::isalnum(peek().value())) {
                    tkn.push_back(consume());
                }

                // Check for keywords
                if (tkn == "exit") {
                    tokens.push_back({.type = tokentype::exit}); // Store 'exit' keyword token
                    tkn.clear();
                    continue;
                } else if (tkn == "let") {
                    tokens.push_back({.type = tokentype::let}); // Store 'let' keyword token
                    tkn.clear();
                    continue;
                } else {
                    // Store identifier token
                    tokens.push_back({.type = tokentype::ident, .value = tkn});
                    tkn.clear();
                    continue;
                }
            } else if (std::isdigit(peek().value())) { // Check if character is a digit (integer literal)
                tkn.push_back(consume());

                // Continue consuming numeric characters
                while (peek().has_value() && std::isdigit(peek().value())) {
                    tkn.push_back(consume());
                }

                tokens.push_back({.type = tokentype::int_lit, .value = tkn}); // Store integer literal token
                tkn.clear();
            }

            // Check for parentheses ( ) and store them as tokens
            else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = tokentype::close_paren});
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = tokentype::open_paren});
            }

            // Check for semicolon (;)
            else if (peek().value() == ';') {
                consume();                                   // Consume semicolon character
                tokens.push_back({.type = tokentype::semi}); // Store semicolon token
                continue;
            }

            // Check for equals (=)
            else if (peek().value() == '=') {
                consume();                                     // Consume equals character
                tokens.push_back({.type = tokentype::equals}); // Store equals token
                continue;
            }

            // Check for plus (+)
            else if (peek().value() == '+') {
                consume();                                   // Consume plus character
                tokens.push_back({.type = tokentype::plus}); // Store plus token
                continue;
            }

            // Ignore whitespace (spaces, newlines, tabs)
            else if (std::isspace(peek().value())) {
                consume();
                continue;
            }

            // Unrecognized character error
            else {
                std::cerr << "Error: Unrecognized character '" << peek().value() << "'" << std::endl;
                exit(EXIT_FAILURE);
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
