#pragma once
#include <vector>
#include "Token.h"

#include <memory>


namespace Tusk {
	using Value = float;

	enum class NodeType {
		// DEFAULTS
		NODE,
		EXPRESSION,
		STATEMENT,

		// EXPRESSIONS
		NUMBER_VALUE,
		BINARY_OPERATION,
		UNARY_OPERATION,
	};

	struct ASTNode {
		virtual ~ASTNode() = default;
		virtual NodeType get_type() const { return NodeType::NODE; }
		virtual std::string to_string() const { return "Tree"; }
	};

	struct Expression : public ASTNode {
		NodeType get_type() const override { return NodeType::EXPRESSION; }
		std::string to_string() const override { return "Expression"; }
	};

	struct BinaryOperation : public Expression {
		std::shared_ptr<Expression> m_left_expression;
		Token m_operator_token;
		std::shared_ptr<Expression> m_right_expression;

		BinaryOperation(std::shared_ptr<Expression> left, Token tok, std::shared_ptr<Expression> right)
			: m_left_expression{left}, m_right_expression{right}, m_operator_token{tok} {}
		NodeType get_type() const override { return NodeType::BINARY_OPERATION; }
		std::string to_string() const override { 
			return "(" + m_left_expression->to_string() + " " + std::to_string((int)m_operator_token.type) + " " + m_right_expression->to_string() + ")";
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

	class Parser {
	public:
		Parser(const std::vector<Token>& tokens) : m_tokens{ tokens } {}

		const std::shared_ptr<ASTNode>& parse();
	private:
		const std::vector<Token> m_tokens;
		int m_current_index{ -1 };
		

		std::shared_ptr<ASTNode> m_final_tree{ nullptr };

		void advance() { m_current_index++; }
		const Token& current_token() const { return m_tokens[m_current_index]; }
		const Token& peek(uint32_t depth) const { return m_tokens[m_current_index + depth]; }

		// EXPRESSIONS
		std::shared_ptr<Expression> expression();
		std::shared_ptr<Expression> term();
		std::shared_ptr<Expression> factor();

		
	};
}