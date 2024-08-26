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
    ErrorHandler handler;
    Emulator emulator(handler);

    std::string in;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, in);
        
        Lexer lexer(in, handler);
        const std::vector<Token>& tokens = lexer.analyze();
        if(handler.has_errors())
            for (const Error& error : handler.get_errors()) {
                std::cout << ErrorHandler::string_basic_with_type(error) << '\n';
            }
        else {
            std::cout << "TOKENS:\n";
            for (const Token& tok : tokens) {
                std::cout << (int)tok.type << '\n';
            }

            Parser parser(tokens, handler);
            const std::shared_ptr<AST>& ast = parser.parse();
            if (handler.has_errors())
                for (const Error& error : handler.get_errors()) {
                    std::cout << ErrorHandler::string_basic_with_type(error) << '\n';
                }
            else {
                std::cout << "NODES:\n";
                std::cout << ast->to_string() << '\n';

                Compiler compiler(ast, handler);
                
                const Unit& byte_code = compiler.compile();
                if (handler.has_errors())
                    for (const Error& error : handler.get_errors()) {
                        std::cout << ErrorHandler::string_basic_with_type(error) << '\n';
                    }
                else {
                    std::cout << "BYTECODE:\n" << byte_code.disassemble();

                    if (emulator.run(&byte_code) != Result::OK)
                        for (const Error& error : handler.get_errors()) {
                            std::cout << ErrorHandler::string_basic_with_type(error) << '\n';
                        }
                }
            }
        }
        handler.clear();
    }
}