#include "pch.h"
#include <iostream>
#include <string>
#include "Lexer.h"

// This file is for testing in production. It will be stripped out when this project becomes a library.

using namespace Tusk;

int main()
{
    std::string in;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, in);
        Lexer lexer(in);
        const std::vector<Token>& tokens = lexer.analyze();
        for (const Token& tok : tokens) {
            std::cout << (int)tok.type << '\n';
        }
    }
}