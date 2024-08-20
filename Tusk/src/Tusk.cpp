#include "pch.h"
#include <iostream>
#include <string>
#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"
#include "Emulator.h"

// This file is for testing in production. It will be stripped out when this project becomes a library.

using namespace Tusk;

int main()
{
    Emulator emulator{};
    std::string in;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, in);
        Lexer lexer(in);
        const std::vector<Token>& tokens = lexer.analyze();
        std::cout << "TOKENS:\n";
        for (const Token& tok : tokens) {
            std::cout << (int)tok.type << '\n';
        }

        Parser parser(tokens);
        const std::shared_ptr<AST>& ast = parser.parse();
        std::cout << "NODES:\n";
        std::cout << ast->to_string() << '\n';

        Compiler compiler(ast);
        const Unit& byte_code = compiler.compile();
        std::cout << "BYTECODE:\n" << byte_code.disassemble();

        emulator.run(&byte_code);
    }
}