#include <iostream>
#include <string>
#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"
#include "Emulator.h"
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace Tusk;

void run(const std::string& in, Emulator& emulator, ErrorHandler& handler) {
    Lexer lexer(in, handler);
    const std::vector<Token>& tokens = lexer.analyze();
    if (handler.has_errors())
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
                //std::cout << "BYTECODE:\n" << byte_code.disassemble();
                std::cout << '\n';
                if (emulator.run(&byte_code) != Result::OK)
                    for (const Error& error : handler.get_errors()) {
                        std::cout << ErrorHandler::string_basic_with_type(error) << '\n';
                    }
                std::cout << '\n';
            }
        }
    }
    handler.clear();
}

int main(int argc, char* argv[])
{
    ErrorHandler handler;
    Emulator emulator(handler);

    if (argc > 1) {
        std::ifstream file(argv[1]);
        std::stringstream buffer;

        buffer << file.rdbuf();
        run(buffer.str(), emulator, handler);
    }
    else {
        std::string in;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, in);

            std::ifstream file(in);
            std::stringstream buffer;

            buffer << file.rdbuf();
            std::cout << buffer.str();
            std::cout << std::filesystem::current_path();
            run(buffer.str(), emulator, handler);
            //run(in, emulator, handler);
        }
    }
}