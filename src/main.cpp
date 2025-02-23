#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include <string>

// Custom header files
#include "tokenization.hpp"
#include "parser.hpp"
#include "generation.hpp"

/*
=> int main(int argc, char *argv[]) is a standard function signature for the main function, and it is used to pass
   command-line arguments to the program when it is executed.
=> argc stands for argument count. It is an integer that indicates the number of command-line arguments passed to the
   program.
=> argv stands for argument vector. It is an array of C-style strings (char *), where each element of the array is a
   command-line argument passed to the program.
  - argv[0] is the name of the program itself,
  - argv[1] is the first argument passed by the user.
=> argc tells you how many arguments were passed while argv gives you access to each argument passed to the program.
*/
int main(int argc, char *argv[]) {
    // Check if exactly one argument (excluding program name) is provided
    if (argc != 2) {
        std::cerr << "Invalid Input. Correct syntax: " << std::endl;
        std::cerr << "quark <input.qrk>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in); // Open file in input mode

        // Check if file opened successfully
        if (!input.is_open()) {
            std::cerr << "Error: Unable to open file " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }

        /*
          input.rdbuf() retrieves the underlying buffer of the std::fstream object input, which holds the contents of
          the file. The << operator is used to copy the entire contents of the file (from input.rdbuf()) into
          contents_stream. The rdbuf() function returns a pointer to the stream's internal buffer, which allows
          direct access to the raw data in the file and transfer it into another stream, in this case, contents_stream.
        */
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    // Tokenization process
    tokenizer obj_tokenizer(std::move(contents));
    std::vector<token> tokens = obj_tokenizer.tokenize();

    // Parsing process
    parser obj_parser(std::move(tokens));
    std::optional<node_exit> tree = obj_parser.parse();

    // Check if parsing resulted in an exit statement
    if (!tree.has_value()) {
        std::cerr << "Error: No exit statement found" << std::endl;
        return EXIT_FAILURE;
    }

    // Code generation process
    generator obj_generator(tree.value());
    {
        std::fstream file("out.asm", std::ios::out);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to create output file out.asm" << std::endl;
            return EXIT_FAILURE;
        }
        file << obj_generator.generate();
    }

    // Execute system commands to assemble and link the generated assembly code
    system("rm -f out.o out");                // Remove old output files
    system("nasm -f elf64 out.asm -o out.o"); // Assemble
    system("ld -o out out.o");                // Link

    return EXIT_SUCCESS;
}
