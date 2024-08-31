#include "Parser.h"
#include <iostream>
#include <vector>

namespace Tusk {
	const std::shared_ptr<AST>& Parser::parse() {
		m_final_tree = std::make_shared<AST>();
		while (current_token().type != TokenType::_EOF) {
			m_final_tree->statements.push_back(statement());
		}
			
		return m_final_tree;
	}

	void Parser::consume(TokenType type, const std::string& error) {
		if (current_token().type != type) {
			m_error_handler.report_error(error, {current_token().line}, ErrorType::COMPILE_ERROR);
		}
		else
			advance();
	}

	std::shared_ptr<Expression> Parser::expression() {
		std::shared_ptr<Expression> left{ logical_and() };
		std::shared_ptr<Expression> right{ nullptr };
		const Token* token{ nullptr };
		while (current_token().type == TokenType::OR) {
			token = &current_token();
			advance();
			right = logical_and();
			left = std::make_shared<BinaryOperation>(left, *token, right);
		}
		return left;
	}

	std::shared_ptr<Expression> Parser::logical_and() {
		std::shared_ptr<Expression> left{ equality() };
		std::shared_ptr<Expression> right{ nullptr };
		const Token* token{ nullptr };
		while (current_token().type == TokenType::AND) {
			token = &current_token();
			advance();
			right = equality();
			left = std::make_shared<BinaryOperation>(left, *token, right);
		}
		return left;
	}

	std::shared_ptr<Expression> Parser::equality() {
		std::shared_ptr<Expression> left{ relational() };
		std::shared_ptr<Expression> right{ nullptr };
		const Token* token{ nullptr };
		while (current_token().type == TokenType::EQUAL_EQUAL || current_token().type == TokenType::BANG_EQUAL) {
			token = &current_token();
			advance();
			right = relational();
			left = std::make_shared<BinaryOperation>(left, *token, right);
		}
		return left;
	}

	std::shared_ptr<Expression> Parser::relational() {
		std::shared_ptr<Expression> left{ arithmetic() };
		std::shared_ptr<Expression> right{ nullptr };
		const Token* token{ nullptr };
		while (current_token().type == TokenType::LESS || current_token().type == TokenType::GREATER
			|| current_token().type == TokenType::LESS_EQUAL || current_token().type == TokenType::GREATER_EQUAL) {
			token = &current_token();
			advance();
			right = arithmetic();
			left = std::make_shared<BinaryOperation>(left, *token, right);
		}
		return left;
	}

	std::shared_ptr<Expression> Parser::arithmetic() {
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
		std::shared_ptr<Expression> to_ret{ nullptr };
		switch(current_token().type) {
		case TokenType::INT:
			to_ret = std::make_shared<Number>(Value{ (int64_t)stol(current_token().value) });
			advance();
			return to_ret;
		case TokenType::FLOAT:
			to_ret = std::make_shared<Number>(Value{ stod(current_token().value) });
			advance();
			return to_ret;
		case TokenType::L_PAR:
			advance();
			to_ret = expression();
			consume(TokenType::R_PAR, "Expected ')'");
			return to_ret;
		case TokenType::MINUS: {
			Token tok = current_token();
			advance();
			to_ret = std::make_shared<UnaryOperation>(tok, factor());
			return to_ret;
		}
		case TokenType::TRUE:
			advance();
			return std::make_shared<BoolValue>(true);
		case TokenType::FALSE:
			advance();
			return std::make_shared<BoolValue>(false);
		case TokenType::BANG: {
			Token tok = current_token();
			advance();
			to_ret = std::make_shared<UnaryOperation>(tok, factor());
			return to_ret;
		}
		case TokenType::ID: {
			/*const std::string& str = current_token().value;
			to_ret = std::make_shared<Name>(str);
			std::vector<std::shared_ptr<Expression>> expr{ };
			advance();
			const Token* tok = &current_token();
			if (current_token().type == TokenType::L_PAR) {
				advance();
				while (current_token().type != TokenType::R_PAR && current_token().type != TokenType::_EOF) {
					tok = &current_token();
					expr.push_back(expression());

					if (current_token().type == TokenType::COMMA)
						advance();
				}
				consume(TokenType::R_PAR, "Expected ')'");
				to_ret = std::make_shared<Call>(std::static_pointer_cast<Name>(to_ret), expr);
			}
			
			return to_ret;*/
			return std::make_shared<LValueStartNode>(identifier());
		}
						  
		case TokenType::VOID: {
			to_ret = std::make_shared<Void>();
			advance();
			return to_ret;
		}
		case TokenType::STR: {
			to_ret = std::make_shared<StringLiteral>(current_token().value);
			advance();
			return to_ret;
		}
		case TokenType::KEYWORD:
			if (current_token().value == "this")
				return std::make_shared<LValueStartNode>(identifier());
			// INTENTIONAL FALLTHROUGH
		default:
			m_error_handler.report_error("Expected expression", {current_token().line}, ErrorType::COMPILE_ERROR);
		}
		return nullptr;
	}

	std::shared_ptr<LValue> Parser::identifier() {
		std::shared_ptr<LValue> left = std::make_shared<LValue>();
		left->name = std::make_shared<Name>(current_token().value);
		const std::string& name = current_token().value;
		advance();
		if (current_token().type == TokenType::L_PAR) {
			std::vector<std::shared_ptr<Expression>> expr{ };
			advance();
			const Token* tok = &current_token();
			while (current_token().type != TokenType::R_PAR && current_token().type != TokenType::_EOF) {
				tok = &current_token();
				expr.push_back(expression());

				if (current_token().type == TokenType::COMMA)
					advance();
			}
			consume(TokenType::R_PAR, "Expected ')'");
			left->name = std::make_shared<Call>(std::make_shared<Name>(name), expr);
		}
		if (current_token().type == TokenType::DOT) {
			advance();
			left->access = identifier();
		}
		return left;
	}

	std::shared_ptr<Statement> Parser::statement() {
		bool expect_semicolon = true;
		std::shared_ptr<Statement> stmt{ nullptr };
		const Token& tok = current_token();
		if (tok.type == TokenType::L_BRACE) {
			stmt = compount_statement();
			expect_semicolon = false;
		}
		else if (tok.type == TokenType::KEYWORD) {
			if (tok.value == "log")
				stmt = log_statement();
			else if (tok.value == "let")
				stmt = variable_declaration();
			else if (tok.value == "break") {
				stmt = std::make_shared<BreakStatement>();
				advance();
			}
			else if (tok.value == "continue") {
				stmt = std::make_shared<ContinueStatement>();
				advance();
			}
			else if (tok.value == "return") {
				stmt = return_statement();
			}
			else if (tok.value == "if") {
				stmt = if_statement();
				expect_semicolon = false;
			}
			else if (tok.value == "fn") {
				stmt = function();
				expect_semicolon = false;
			}
			else if (tok.value == "class") {
				stmt = class_declaration();
				expect_semicolon = false;
			}
			else if (tok.value == "while") {
				stmt = while_statement();
				expect_semicolon = false;
			}
			else {
				m_error_handler.report_error("Unexpected '" + tok.value + "'", {tok.line}, ErrorType::COMPILE_ERROR);
				return nullptr;
			}
		}
		else if (tok.type == TokenType::ID)
			stmt = assignment();
		else if (current_token().type == TokenType::SEMICOLON) {
			advance();
			return std::make_shared<VoidStatement>();
		}
		else
			stmt = expression_statement();

		if(expect_semicolon)
			consume(TokenType::SEMICOLON, "Expected ';'");
		return stmt;
	}

	std::shared_ptr<Statement> Parser::log_statement() {
		advance();
		return std::make_shared<LogStatement>( expression() );
	}

	std::shared_ptr<Statement> Parser::expression_statement() {
		return std::make_shared<ExpressionStatement>(expression());
	}

	std::shared_ptr<Statement> Parser::variable_declaration(bool allow_set_value) {
		advance();
		if (current_token().type != TokenType::ID) {
			m_error_handler.report_error("Expected identifier", { current_token().line }, ErrorType::COMPILE_ERROR);
			return nullptr;
		}
		std::shared_ptr<VariableDeclaration> declaration = std::make_shared<VariableDeclaration>();

		declaration->variable_name = current_token().value;
		advance();
		if (!allow_set_value && current_token().type == TokenType::EQUAL) {
			m_error_handler.report_error("Setting values of class variables is not allowed", { current_token().line }, ErrorType::COMPILE_ERROR);
			return declaration;
		}
		if (current_token().type != TokenType::SEMICOLON) {
			consume(TokenType::EQUAL, "Expected '='");
			declaration->value = expression();
		}

		return declaration;
	}

	std::shared_ptr<Statement> Parser::assignment() {
		//const std::string& name = current_token().value;
		//advance();
		//advance();
		//return std::make_shared<Assignment>(name, expression());
		int current_index = m_current_index;
		std::shared_ptr<LValueStartNode> lval = std::make_shared<LValueStartNode>(identifier());

		if (current_token().type == TokenType::EQUAL) {
			advance();
			return std::make_shared<Assignment>(lval, expression());
		}
		else {
			m_current_index = current_index;
			return expression_statement();
		}
	}

	std::shared_ptr<Statement> Parser::if_statement() {
		advance();
		std::shared_ptr<Expression> condition = expression();
		consume(TokenType::ARROW, "Expected '->'");
		std::shared_ptr<Statement> body = statement();
		std::shared_ptr<Statement> else_body = nullptr;
		if (current_token().type == TokenType::KEYWORD && current_token().value == "else") {
			advance();
			else_body = statement();
		}
		return std::make_shared<IfStatement>(condition, body, else_body);
	}

	std::shared_ptr<Statement> Parser::while_statement() {
		advance();
		std::shared_ptr<Expression> condition = expression();
		consume(TokenType::ARROW, "Expected '->'");
		std::shared_ptr<Statement> body = statement();

		return std::make_shared<WhileStatement>(condition, body);
	}

	std::shared_ptr<Statement> Parser::compount_statement() {
		advance();
		std::vector<std::shared_ptr<Statement>> statements;
		while (current_token().type != TokenType::R_BRACE && current_token().type != TokenType::_EOF)
			statements.push_back(statement());

		if (current_token().type == TokenType::_EOF) {
			m_error_handler.report_error("Expected '}'", { current_token().line }, ErrorType::COMPILE_ERROR);
			return nullptr;
		}
		else
			advance();

		return std::make_shared<CompountStatement>(statements);
	}

	std::shared_ptr<Statement> Parser::function() {
		advance();
		const Token& tok = current_token();
		consume(TokenType::ID, "Expected identifier");

		std::string name = tok.value;
		std::vector<std::shared_ptr<Argument>> args;
		if (current_token().type == TokenType::L_PAR) {
			advance();
			const Token* tok = &current_token();
			while (current_token().type != TokenType::R_PAR && current_token().type != TokenType::_EOF) {
				tok = &current_token();
				if (tok->type != TokenType::ID) {
					consume(TokenType::ID, "Expected argument name");
					return nullptr;
				}
				else
					advance();
				
				std::shared_ptr<Expression> expr{ nullptr };
				if (current_token().type == TokenType::EQUAL) {
					advance();
					expr = expression();
				}
				if (current_token().type == TokenType::COMMA)
					advance();

				args.push_back(std::make_shared<Argument>(tok->value, expr));
			}
			consume(TokenType::R_PAR, "Expected ')'");
		}
		consume(TokenType::ARROW, "Expected '->'");
		std::shared_ptr<Statement> stmt = statement();

		return std::make_shared<FunctionDeclaration>(name, args, stmt);
	}

	std::shared_ptr<Statement> Parser::return_statement() {
		advance();
		if(current_token().type != TokenType::SEMICOLON)
			return std::make_shared<ReturnStatement>(expression());
		return std::make_shared<ReturnStatement>(nullptr);
	}

	std::shared_ptr<Statement> Parser::class_declaration() {
		advance();
		const Token& tok = current_token();
		consume(TokenType::ID, "Expected identifier");

		std::string name = tok.value;
		if (current_token().type != TokenType::L_BRACE) {
			m_error_handler.report_error("Expected '{'", {current_token().line}, ErrorType::COMPILE_ERROR);
			return nullptr;
		}
		advance();
		std::vector<std::shared_ptr<Statement>> statements;
		while (current_token().type != TokenType::R_BRACE && current_token().type != TokenType::_EOF)
			statements.push_back(class_body());

		if (current_token().type == TokenType::_EOF) {
			m_error_handler.report_error("Expected '}'", { current_token().line }, ErrorType::COMPILE_ERROR);
			return nullptr;
		}
		else
			advance();

		return std::make_shared<ClassDeclaration>(name, std::make_shared<CompountStatement>(statements));
	}

	std::shared_ptr<Statement> Parser::class_body() {
		std::shared_ptr<Statement> stmt{nullptr};
		if (current_token().type == TokenType::KEYWORD) {
			if (current_token().value == "fn")
				stmt = function();
			else if (current_token().value == "let") {
				stmt = variable_declaration(false);
				consume(TokenType::SEMICOLON, "Expected ';'");
			}
			else
				m_error_handler.report_error("Expected function or variable declaration", {current_token().line}, ErrorType::COMPILE_ERROR);
		}
		else
			m_error_handler.report_error("Expected function or variable declaration", { current_token().line }, ErrorType::COMPILE_ERROR);
		return stmt;
	}
}