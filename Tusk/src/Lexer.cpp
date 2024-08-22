#include "pch.h"
#include "lexer.h"
#include <string>
#include <iostream>
#include <format>

namespace Tusk {
    Token Lexer::lex() {
        next();

        while (m_current_char == ' ' || m_current_char == '\n' || m_current_char == '\t' || m_current_char == '/') {    // Skip whitespace
            if (m_current_char == '\n')                                                                           // Change line on newline character
                m_line++;
            else if (m_current_char == '/') {                                                                     // Skip comments
                if (match('/')) {
                    while (m_current_char != '\n' && m_index < m_source.size())
                        next();
                }
                else if (m_index < m_source.size() && match('*')) {
                    next();
                    next();
                    while (m_index < m_source.size()) {
                        if (m_current_char == '\n') {
                            m_line++;
                            next();
                        }
                        else if (m_current_char == '*' && match('/')) {
                            next();
                            next();
                            break;
                        }
                        else {
                            next();
                        }
                    }
                }
                else
                    break;
            }
            next();
        }

        if (is_digit(m_current_char)) return (number());                        // Make number token
        if (is_alpha(m_current_char)) return (identifier());                    // Make identifier/keyword token

        int old_index = m_index;                                                      // Keep track of starting index
        switch (m_current_char) {                                                     // Make single/double character tokens or string token
        case '(': return (Token{ TokenType::L_PAR, "", m_line, old_index, m_index });
        case ')': return (Token{ TokenType::R_PAR, "", m_line, old_index, m_index });
        case '{': return (Token{ TokenType::L_BRACE, "", m_line, old_index, m_index });
        case '}': return (Token{ TokenType::R_BRACE, "", m_line, old_index, m_index });
        case '[': return (Token{ TokenType::L_BRACK, "", m_line, old_index, m_index });
        case ']': return (Token{ TokenType::R_BRACK, "", m_line, old_index, m_index });
        case ';': return (Token{ TokenType::SEMICOLON, "", m_line, old_index, m_index });
        case ',': return (Token{ TokenType::COMMA, "", m_line, old_index, m_index });
        case '.': return (Token{ TokenType::DOT, "", m_line, old_index, m_index });
        case '^': return (Token{ TokenType::CAP, "", m_line, old_index, m_index });
        case '&': return (Token{ TokenType::AMPERSAND, "", m_line, old_index, m_index });
        case '~': return (Token{ TokenType::TILDE, "", m_line, old_index, m_index });
        case '%': return (Token{
            match('=') ? TokenType::PERCENT_EQUAL : TokenType::PERCENT, "", m_line, old_index, m_index });
        case '-': return (Token{
            match('=') ? TokenType::MINUS_EQUAL : match('>') ? TokenType::ARROW : match('-') ? TokenType::MINUS_MINUS : TokenType::MINUS, "", m_line, old_index, m_index });
        case '+': return (Token{
            match('=') ? TokenType::PLUS_EQUAL : match('+') ? TokenType::PLUS_PLUS : TokenType::PLUS, "", m_line, old_index, m_index });
        case '/': return (Token{
            match('=') ? TokenType::SLASH_EQUAL : TokenType::SLASH, "", m_line, old_index, m_index });
        case '*': return (Token{
            match('=') ? TokenType::STAR_EQUAL : TokenType::STAR, "", m_line, old_index, m_index });
        case '!':
            return (Token{
                match('=') ? TokenType::BANG_EQUAL : TokenType::BANG, "", m_line, old_index, m_index });
        case '=':
            return (Token{
                match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL, "", m_line, old_index, m_index });
        case '<':
            return (Token{
                match('=') ? TokenType::LESS_EQUAL : match('<') ? TokenType::L_SHIFT : TokenType::LESS, "", m_line, old_index, m_index });
        case '>':
            return (Token{
                match('=') ? TokenType::GREATER_EQUAL : match('>') ? TokenType::R_SHIFT : TokenType::GREATER, "", m_line, old_index, m_index });
        case '"':
            return (string());
        case '\0':
            return Token{ TokenType::_EOF, "", m_line, m_index, m_index };
        }
        m_error_handler.report_error(std::format("Unexpected character '{0}'", m_current_char), { m_line, old_index, m_index }, ErrorType::SYNTAX_ERROR);
        return Token{ TokenType::ERROR, "", m_line, old_index, m_index };

    }

    const std::vector<Token>& Lexer::analyze() {
        do {
            m_out.push_back(lex());
        } while (m_current_char != '\0');
        return m_out;
    }

    Token Lexer::string() {
        std::string string;
        bool interpolated = false;
        int old_index = m_index;
        next();
        while (m_current_char != '"' && m_current_char != '\0') {               // Make the string body
            if (m_current_char == '\n')
                m_line++;
            string += m_current_char;
            next();
        }

        if (m_current_char == '\0') {                                        // If the lexer reaches the end without closing the string return an error
            m_error_handler.report_error("Unterminated String", {m_line, old_index, m_index}, ErrorType::SYNTAX_ERROR);
            return Token{ TokenType::ERROR, "", m_line, old_index, m_index };
        }
        return Token{ TokenType::STR, string, m_line, old_index, m_index };
    }

    Token Lexer::number() {
        int old_index = m_index;
        std::string num_string;
        bool is_float = false;
        bool has_error = false;
        int dot_index = 0;

        while (is_digit(m_current_char) || m_current_char == '.' || m_current_char == '_') {          // Make number string
            if (m_current_char == '.') {
                if (is_float) {
                    has_error = true;                                                           // If there already was a dot, report error
                    dot_index = m_index;
                }
                else
                    is_float = true;                                                            // Else set the type as float
                num_string += m_current_char;
                next();

            }
            else if (m_current_char == '_')                                                       // Allow underscores in numbers for readability
                next();
            else {

                num_string += m_current_char;
                next();
            }
        }
        back();

        if (has_error) {                                                                        // Error double dot
            m_error_handler.report_error("Unexpected '.'", { m_line, dot_index, dot_index }, ErrorType::SYNTAX_ERROR);
            return Token{ TokenType::ERROR, "", m_line, dot_index, dot_index };
        }
        if (is_float)
            return Token{ TokenType::FLOAT, num_string, m_line, old_index, m_index };
        return Token{ TokenType::INT, num_string, m_line, old_index, m_index };
    }

    Token Lexer::identifier() {
        int old_index = m_index;
        std::string string;

        while (is_alpha(m_current_char) || is_digit(m_current_char)) {                          // Make identifier body
            string += m_current_char;
            next();
        }
        back();

        if (std::find(keywords.begin(), keywords.end(), string) != keywords.end())              // Check if name is identifier or keyword
            return Token{ TokenType::KEYWORD, string, m_line, old_index, m_index };
        //else if (std::find(types.begin(), types.end(), string) != types.end())                // Check if name is type keyword
        //    return Token{ TokenType::TYPE, string, m_line, old_index, m_index };
        else if (string == "and")
            return Token{ TokenType::AND, string, m_line, old_index, m_index };
        else if (string == "or")
            return Token{ TokenType::OR, string, m_line, old_index, m_index };
        else if (string == "true")
            return Token{ TokenType::TRUE, string, m_line, old_index, m_index };
        else if (string == "false")
            return Token{ TokenType::FALSE, string, m_line, old_index, m_index };
        else if (string == "void")
            return Token{ TokenType::VOID, string, m_line, old_index, m_index };
        return Token{ TokenType::ID, string, m_line, old_index, m_index };
    }

    bool Lexer::match(char expected) {                                                          // If next character matches the expected one,
        if (m_index + 1 >= m_source.size() || m_source[m_index + 1] != expected) return false;  // advance and return true, else return false
        next();
        return true;
    }

    void Lexer::back() {
        m_index--;
        m_current_char = m_source[m_index];
    }

    void Lexer::next() {
        m_index++;
        m_current_char = m_index < m_source.size() ? m_source[m_index] : '\0';
    }

    bool Lexer::is_alpha(char character) {
        return (character >= 'a' && character <= 'z') ||
            (character >= 'A' && character <= 'Z') || character == '_';
    }

    bool Lexer::is_digit(char character) {
        return character >= '0' && character <= '9';
    }
}