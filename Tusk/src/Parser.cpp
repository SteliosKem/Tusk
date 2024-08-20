#include "Parser.h"
#include <iostream>

namespace Tusk {
	const std::shared_ptr<AST>& Parser::parse() {
		advance();
		m_final_tree = std::make_shared<AST>();
		while (current_token().type != TokenType::_EOF) {
			m_final_tree->statements.push_back(statement());
		}
			
		return m_final_tree;
	}

	void Parser::consume(TokenType type, const std::string& error) {
		if (peek().type != type)
			std::cout << error;
		else {
			advance();
			advance();
		}
	}

	std::shared_ptr<Expression> Parser::expression() {
		std::shared_ptr<Expression> left{ term() };
		std::shared_ptr<Expression> right{ nullptr };
		const Token* token{ nullptr };
		while(current_token().type == TokenType::PLUS || current_token().type == TokenType::MINUS) {
			token = &current_token();
			advance();
			right = term();
			left = std::make_shared<BinaryOperation>(left, *token, right);
		}
		return left;
	}

	std::shared_ptr<Expression> Parser::term() {
		std::shared_ptr<Expression> left{ factor() };
		std::shared_ptr<Expression> right{ nullptr };
		const Token* token{ nullptr };
		while (current_token().type == TokenType::STAR || current_token().type == TokenType::SLASH) {
			token = &current_token();
			advance();
			right = factor();
			left = std::make_shared<BinaryOperation>(left, *token, right);
		}
		return left;
	}

	std::shared_ptr<Expression> Parser::factor() {
		//std::shared_ptr<Expression> left{ factor() };
		switch(current_token().type) {
		case TokenType::INT:
			std::shared_ptr<Expression> to_ret = std::make_shared<Number>(stod(current_token().value));
			advance();
			return to_ret;
		}
		return nullptr;
	}

	std::shared_ptr<Statement> Parser::statement() {
		std::shared_ptr<Statement> stmt{ nullptr };
		const Token& tok = current_token();
		if (tok.type == TokenType::KEYWORD) {
			if (tok.value == "log")
				stmt = log_statement();
		}
		else
			stmt = expression_statement();

		//consume(TokenType::SEMICOLON, "Expected ';'");
		return stmt;
	}

	std::shared_ptr<Statement> Parser::log_statement() {
		advance();
		return std::make_shared<LogStatement>( expression() );
	}

	std::shared_ptr<Statement> Parser::expression_statement() {
		return std::make_shared<ExpressionStatement>(expression());
	}
}