#include "Parser.h"
#include <iostream>

namespace Tusk {
	const std::shared_ptr<ASTNode>& Parser::parse() {
		advance();
		//m_final_tree = std::make_shared<ASTNode>();
		//while (current_token().type != TokenType::_EOF) {}
		m_final_tree = expression();
		return m_final_tree;
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
	}
}