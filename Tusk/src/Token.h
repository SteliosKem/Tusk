#pragma once
#include <string>

enum class TokenType {
	L_PAR, R_PAR, L_BRACE, R_BRACE, L_BRACK, R_BRACK, COMMA,		// 
	DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, CAP, TILDE,			// SINGLE CHARACTER TOKENS
	PERCENT, AMPERSAND, PIPE,										//

	BANG, BANG_EQUAL, EQUAL, EQUAL_EQUAL, GREATER, OR,				//
	GREATER_EQUAL, LESS, LESS_EQUAL, PLUS_EQUAL, MINUS_EQUAL,		// DOUBLE CHARACTER TOKENS
	STAR_EQUAL, SLASH_EQUAL, PLUS_PLUS, MINUS_MINUS, AND,			//
	R_SHIFT, L_SHIFT, ARROW, PERCENT_EQUAL,

	ID, STR, TRUE, FALSE, KEYWORD, TYPE,							// TYPE TOKENS
	INT, FLOAT,

	ERROR, _EOF														// SPECIAL TOKENS
};

struct Token {
	TokenType type = TokenType::_EOF;
	std::string value = "";
	int line = 0;
	int start_idx = 0;
	int end_idx = 0;
};