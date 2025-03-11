/*
    Harded coded (very manually coded)
    take an input file -> read it -> extract contents -> extract tokens -> save tokens in a vector -> write assembly ->
    -> make .asm file -> generate executable file

    To Run :
        => first rename the file to main.cpp (or as mentioned in CMakeLists.txt)

        make # build
        ./querk ../input.qrk # run with input file
        ./out # running executable
        echo $? # checking output

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include <string>

// tokens datatype
enum class tokentype { exit, int_lit, semi };

struct token {
    tokentype type;
    std::optional<std::string>
        value{}; // declaring an optional object that can either contain a std::string or be empty.
};

/*
  going through each letter for example in exit 0;
  we get each and every token out of the input and save it in a vector<token> while assigning thier type.
*/
std::vector<token> tokenize(const std::string &str) {
    std::vector<token> tokens; // to store all the tokens
    std::string tkn;           // for getting individual tokens

    for (size_t i = 0; i < str.size(); ++i) {
        char c = str.at(i);

        if (std::isalpha(c)) { // first index of a token should always be a letter
            tkn.push_back(c);
            while (i + 1 < str.size() && std::isalnum(str.at(i + 1))) { // rest can be alphanumeric
                i++;
                tkn.push_back(str.at(i)); // collecting token in tkn
            }

            if (tkn == "exit") {
                tokens.push_back({.type = tokentype::exit}); // pushing token into the vector
                tkn.clear();
                continue;
            } else { // default
                std::cerr << "oops! an error(1): Unknown token '" << tkn << "'" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (std::isdigit(c)) { // to identify the digit value
            tkn.push_back(c);

            while (i + 1 < str.size() && std::isdigit(str[i + 1])) {
                tkn += str[++i]; // Correctly appends next digit
            }

            tokens.push_back({.type = tokentype::int_lit, .value = tkn});
            tkn.clear();
        } else if (c == ';') { // encountering semi colon (;)
            tokens.push_back({.type = tokentype::semi});
        } else if (std::isspace(c)) { // encountering white space
            continue;
        } else { // default
            std::cerr << "oops! an error(2): Unexpected character '" << c << "'" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return tokens;
}

std::string tokens_to_asm(const std::vector<token> &tokens) {
    std::stringstream output;
    output << "global _start\n_start:\n";

    for (int i = 0; i < tokens.size(); ++i) {
        const token &token = tokens.at(i);
        if (token.type == tokentype::exit) {
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == tokentype::int_lit) {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == tokentype::semi) {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }
    return output.str();
}

/*
=> int main(int argc, char *argv[]) is a standard function signature for the main function, and it is used to pass
command-line arguments to the program when it is executed.
=> argc stands for argument count.It is an integer that indicates the number of command-line arguments passed to the
program.
=> argv stands for argument vector.It is an array of C-style strings (char *), where each element of the array is a
command-line argument passed to the program.
  -argv[0] is the name of the pcarogram itself,
  -argv[1] is the first argument passed by the user.
=>argc tells you how many arguments were passed while argv gives you access to each argument passed to the program.
*/
int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Invalid Input. Correct syntax .. " << std::endl;
        std::cerr << "quark <input.qrk>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(
            argv[1],
            std::ios::in); // std::ios::in is a flag used to specify that a file should be opened in input mode.

        /*
          input.rdbuf() Retrieves the underlying buffer of the std::fstream object input, which holds the contents of
          the file. The << operator./ is used to copy the entire contents of the file (the contents from input.rdbuf())
          into the contents_stream. The rdbuf() function returns a pointer to the stream's internal buffer, which allows
          you to directly access the raw data in the file and transfer it into another stream, in this case,
          contents_stream
          */

        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    std::vector<token> tokens = tokenize(contents);
    // std::cout << tokens_to_asm(tokens) << std::endl;

    // converting the tokens to assembly
    {
        std::fstream file("../build/out.asm", std::ios::out);
        file << tokens_to_asm(tokens);
    }

    // to run the assembly file (creating object n output files)
    system("rm -f out.o out");                // Remove old output
    system("nasm -f elf64 out.asm -o out.o"); // Assemble
    system("ld -o out out.o");                // Link
    return EXIT_SUCCESS;
}
