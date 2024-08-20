#pragma once
#include <vector>
#include "Token.h"
#include "Value.h"
#include <memory>


namespace Tusk {
	

	enum class NodeType {
		// DEFAULTS
		NODE,
		EXPRESSION,
		STATEMENT,

		// EXPRESSIONS
		NUMBER_VALUE,
		BINARY_OPERATION,
		UNARY_OPERATION,

		//STATEMENTS
		LOG_STATEMENT,
		EXPRESSION_STATEMENT,
	};

	struct ASTNode {
		virtual ~ASTNode() = default;
		virtual NodeType get_type() const { return NodeType::NODE; }
		virtual std::string to_string() const { return "Tree"; }
	};

	// EXPRESSIONS

	struct Expression : public ASTNode {
		NodeType get_type() const override { return NodeType::EXPRESSION; }
		std::string to_string() const override { return "Expression"; }
	};

	struct BinaryOperation : public Expression {
		std::shared_ptr<Expression> left_expression;
		Token operator_token;
		std::shared_ptr<Expression> right_expression;

		BinaryOperation(std::shared_ptr<Expression> left, Token tok, std::shared_ptr<Expression> right)
			: left_expression{left}, right_expression{right}, operator_token{tok} {}
		NodeType get_type() const override { return NodeType::BINARY_OPERATION; }
		std::string to_string() const override { 
			return "(" + left_expression->to_string() + " " + std::to_string((int)operator_token.type) + " " + right_expression->to_string() + ")";
		}
	};

	struct UnaryOperation : public Expression {
		Token m_operator_token;
		std::shared_ptr<Expression> m_right_expression;

		UnaryOperation(Token tok, std::shared_ptr<Expression> right)
			: m_right_expression{ right }, m_operator_token{ tok } {}
		NodeType get_type() const override { return NodeType::UNARY_OPERATION; }
		std::string to_string() const override { return "(" + std::to_string((int)m_operator_token.type) + " " + m_right_expression->to_string() + ")"; }
	};

	struct Number : public Expression {
		Value value;

		Number(Value val) : value{val} {}
		NodeType get_type() const override { return NodeType::NUMBER_VALUE; }
		std::string to_string() const override { return std::to_string( value ); }
	};

	// STATEMENTS

	struct Statement : public ASTNode {
		NodeType get_type() const override { return NodeType::STATEMENT; }
		std::string to_string() const override { return "Statement"; }
	};

	struct LogStatement : public Statement {
		std::shared_ptr<Expression> output;

		LogStatement(const std::shared_ptr<Expression>& out) : output{ out } {}
		NodeType get_type() const override { return NodeType::LOG_STATEMENT; }
		std::string to_string() const override { return "Log " + output->to_string() + "\n"; }
	};

	struct ExpressionStatement : public Statement {
		std::shared_ptr<Expression> expression;

		ExpressionStatement(const std::shared_ptr<Expression>& expr) : expression{ expr } {}
		NodeType get_type() const override { return NodeType::EXPRESSION_STATEMENT; }
		std::string to_string() const override { return "Expression " + expression->to_string() + "\n"; }
	};

	// TREE
	struct AST : ASTNode {
		std::vector<std::shared_ptr<Statement>> statements;
		//NodeType get_type() const override { return NodeType::NODE; }
		std::string to_string() const override { 
			std::string str{};
			for (const std::shared_ptr<Statement>& stmt : statements)
				str += stmt->to_string() + '\n';
			return str;
		}
	};

	class Parser {
	public:
		Parser(const std::vector<Token>& tokens) : m_tokens{ tokens } {}

		const std::shared_ptr<AST>& parse();
	private:
		const std::vector<Token> m_tokens;
		int m_current_index{ -1 };
		

		std::shared_ptr<AST> m_final_tree{ nullptr };

		// UTIL
		void advance() { m_current_index++; }
		const Token& current_token() const { return m_tokens[m_current_index]; }
		const Token& peek(uint32_t depth = 1) const { return m_tokens[m_current_index + depth]; }
		void consume(TokenType type, const std::string& error);

		// EXPRESSIONS
		std::shared_ptr<Expression> expression();
		std::shared_ptr<Expression> term();
		std::shared_ptr<Expression> factor();

		// STATEMENTS
		std::shared_ptr<Statement> statement();
		std::shared_ptr<Statement> log_statement();
		std::shared_ptr<Statement> expression_statement();
	};
}