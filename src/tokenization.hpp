#pragma once // Ensures this header file is processed only once during compilation

#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <cctype>

// Enum representing different types of tokens
enum class tokentype { exit, int_lit, semi };

// Token structure representing a token with its type and optional value
struct token {
    tokentype type;
    std::optional<std::string> value{}; // Optional string value, used only for int literals
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
        while (peak().has_value()) {
            if (std::isalpha(peak().value())) { // Check if character is alphabetic
                tkn.push_back(consume());       // Append character and advance position

                // Continue consuming alphanumeric characters
                while (peak().has_value() && std::isalnum(peak().value())) {
                    tkn.push_back(consume());
                }

                // Check for the "exit" keyword
                if (tkn == "exit") {
                    tokens.push_back({.type = tokentype::exit});
                    tkn.clear();
                    continue;
                } else {
                    std::cerr << "Error: Unrecognized token" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else if (std::isdigit(peak().value())) { // Check if character is a digit
                tkn.push_back(consume());

                // Continue consuming numeric characters
                while (peak().has_value() && std::isdigit(peak().value())) {
                    tkn.push_back(consume());
                }

                tokens.push_back({.type = tokentype::int_lit, .value = tkn}); // Store integer token
                tkn.clear();
            } else if (peak().value() == ';') { // Check for semicolon
                tokens.push_back({.type = tokentype::semi});
                consume(); // Consume semicolon character
                continue;
            } else if (std::isspace(peak().value())) { // Ignore whitespace
                consume();
                continue;
            } else { // Unrecognized character error
                std::cerr << "Error: Unrecognized character" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        return tokens;
    }

  private:
    // Function to peek ahead in the source string without consuming characters
    [[nodiscard]] inline std::optional<char> peak(int ahead = 0) const {
        if (m_index + ahead >= m_src.length()) {
            return {}; // Return empty optional if out of bounds
        } else {
            return m_src.at(m_index + ahead); // Return character at current position
        }
    }

    // Function to consume a character and move to the next one
    inline char consume() {
        return m_src.at(m_index++);
    }

    const std::string m_src; // Source code string
    size_t m_index = 0;      // Current position in the source code
};
