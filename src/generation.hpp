#pragma once // Ensures this header file is processed only once during compilation

#include "parser.hpp" // Include parser header file
#include <sstream>    // Include sstream for std::stringstream

// Code generator class that generates assembly output from parsed AST
class generator {
  public:
    // Constructor: Initializes generator with a parsed exit node
    inline generator(node_exit root) : m_root(std::move(root)) {}

    // Function to generate assembly code from the parsed AST
    [[nodiscard]] std::string generate() const {
        std::stringstream output;
        output << "global _start\n_start:\n"; // Define global _start label for assembly entry point
        output << "    mov rax, 60\n";        // Syscall number for exit
        output << "    mov rdi, " << m_root.expr.int_lit.value.value() << "\n"; // Exit status
        output << "    syscall";                                                // Perform syscall to exit program
        return output.str();
    }

  private:
    const node_exit m_root; // Root node representing the parsed exit statement
};
