#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>

enum class TokenType {
    _return,
    int_lit,
    semi,
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string& str) {
    std::vector<Token> tokens {};

    std::string buf;
    for (int i = 0; i < str.length(); i++) {
        char c = str.at(i);
        // check if the first character of the thing we are reading in is either a character, number or whitespace
        if (std::isalpha(c)) {
            buf.push_back(c);
            i++;
            while (std::isalnum(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;

            if (buf == "return") {
                tokens.push_back({.type = TokenType::_return});
                buf.clear();
                continue;
            }
            else {
                std::cerr << "You messed up!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (std::isdigit(c)) {
            buf.push_back(c);
            i++;
            while (std::isdigit(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;

            tokens.push_back({.type = TokenType::int_lit, .value = buf});
            buf.clear();
        }
        else if (c == ';') {
            tokens.push_back({.type = TokenType::semi});

        }
        else if (std::isspace(c)) {
            continue;
        }
        else {
            std::cerr << "You are cooked my guy!" << std::endl;
            exit(EXIT_FAILURE);
        }

    }
    return tokens;
}

void printTokenInfo(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << "Token: ";
        switch (token.type) {
            case TokenType::_return:
                std::cout << "return";
                break;
            case TokenType::int_lit:
                std::cout << "integer literal";
                if (token.value) {
                    std::cout << " (" << *token.value << ")";
                }
                break;
            case TokenType::semi:
                std::cout << "semicolon";
                break;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::string tokens_to_asm(std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _start\nstart:\n";
    for (int i = 0; i < tokens.size(); i++) {
        const Token& token = tokens.at(i);
        if (token.type == TokenType::_return) {
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi) {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }
    return output.str();
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "vorynix <input.vx>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        if (!input) {
            std::cerr << "Error: Unable to open file " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents);
    // Print information about the tokens
    std::cout << "Token Data:" << std::endl;
    printTokenInfo(tokens);

    std::cout << "x86_64 Assembly:" << std::endl;
    std::cout << tokens_to_asm(tokens) << std::endl;

    {
        std::fstream file("out.asm", std::ios::out);
        file << tokens_to_asm(tokens) << std::endl;
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}
