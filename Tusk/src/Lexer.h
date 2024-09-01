#pragma once
#include <string>
#include <vector>
#include <array>
#include "token.h"
#include "Error.h"

namespace Tusk {
	class Lexer {
	public:
		Lexer(const std::string& source, ErrorHandler& handler) : m_source(source), m_error_handler{ handler } {}
		//Lexer() { m_source = ""; }

		const std::vector<Token>& analyze();					// Lexes the source code and returns vector of tokens

		std::array<std::string, 14> keywords{ "return", "let", "fn", "log", "if", "else", 
			"while", "do", "for", "break", "continue", "class", "this", "enum"};
		//std::array<std::string, 15> types{ "isize", "fsize", "i8", "i16", "i32", "i64", "f32", "f64", "u8", "usize", "u16", "u32", "u64", "string", "void" };
	private:
		static bool is_digit(char character);					// Check if a character is a digit
		static bool is_alpha(char character);					// Check if a character is alphanumeric

		void next();											// Advances the index and updates current_char
		void back();
		bool match(char expected);								// Match next character

		Token lex();											// Lex a single Token

		Token string();											// Make string Token
		Token number();											// Make number Token
		Token identifier();										// Make identifier/keyword Token

	private:
		char m_current_char{ '\0' };							// Current character
		int m_index{ -1 };										// Current index
		int m_line{ 1 };										// Current line inside a file

		std::string m_source{};									// The source code to lex

		ErrorHandler& m_error_handler;

		std::vector<Token> m_out;
	};
}