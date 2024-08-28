#pragma once
#include <vector>
#include "Token.h"
#include "Value.h"
#include <memory>
#include <iostream>
#include "Error.h"

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
		BOOL_VALUE,
		NAME,
		VOID,
		STRING,
		CALL,

		//STATEMENTS
		COMPOUNT_STATEMENT,
		LOG_STATEMENT,
		EXPRESSION_STATEMENT,
		VARIABLE_DECLARATION,
		ASSIGNMENT,
		IF_STATEMENT,
		VOID_STATEMENT,
		WHILE_STATEMENT,
		BREAK_STATEMENT,
		CONTINUE_STATEMENT,
		FUNCTION_DECLARATION,
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
		Token operator_token;
		std::shared_ptr<Expression> right_expression;

		UnaryOperation(Token tok, std::shared_ptr<Expression> right)
			: right_expression{ right }, operator_token{ tok } {}
		NodeType get_type() const override { return NodeType::UNARY_OPERATION; }
		std::string to_string() const override { return "(" + std::to_string((int)operator_token.type) + " " + right_expression->to_string() + ")"; }
	};

	struct Number : public Expression {
		/*std::variant<int64_t, double> value;

		Number(const std::variant<int64_t, double>& val) : value{val} {}
		NodeType get_type() const override { return NodeType::NUMBER_VALUE; }
		std::string to_string() const override { return std::to_string( std::holds_alternative<int64_t>(value) ? std::get<int64_t>(value) : std::get<double>(value)); }*/
		Value value;

		Number(const Value& val) : value{val} {}
		NodeType get_type() const override { return NodeType::NUMBER_VALUE; }
		std::string to_string() const override { return std::to_string(value.is<int64_t>() ? value.get<int64_t>() : value.get<double>()); }
	};

	struct BoolValue : public Expression {
		bool value;

		BoolValue(bool val) : value{ val } {}
		NodeType get_type() const override { return NodeType::BOOL_VALUE; }
		std::string to_string() const override { return value ? "true" : "false"; }
	};

	struct StringLiteral : public Expression {
		std::string value;

		StringLiteral(const std::string& val) : value{ val } {}
		NodeType get_type() const override { return NodeType::STRING; }
		std::string to_string() const override { return '"' + value + '"'; }
	};

	struct Name : public Expression {
		std::string string;

		Name(const std::string& val) : string{ val } {}
		NodeType get_type() const override { return NodeType::NAME; }
		std::string to_string() const override { return string; }
	};

	struct Void : public Expression {
		Void() = default;
		NodeType get_type() const override { return NodeType::VOID; }
		std::string to_string() const override { return "void"; }
	};

	struct Call : Expression {
		std::shared_ptr<Name> name;
		std::vector<std::shared_ptr<Expression>> parameters;

		Call(const std::shared_ptr<Name>& val, const std::vector<std::shared_ptr<Expression>>& vec) : name{ val }, parameters { vec } {}
		NodeType get_type() const override { return NodeType::CALL; }
		std::string to_string() const override {
			std::string str{ "(" };
			if(!parameters.empty())
				for (const auto& param : parameters)
					str += param->to_string() + ", ";
			return "Call " + name->string + str + ")";
		}
	};

	// STATEMENTS

	struct Statement : public ASTNode {
		NodeType get_type() const override { return NodeType::STATEMENT; }
		std::string to_string() const override { return "Statement"; }
	};

	struct CompountStatement : public Statement {
		std::vector<std::shared_ptr<Statement>> statements;

		CompountStatement(const std::vector<std::shared_ptr<Statement>>& stmts) : statements{ stmts } {}
		NodeType get_type() const override { return NodeType::COMPOUNT_STATEMENT; }
		std::string to_string() const override { std::string i; for (const auto& s : statements) i += s->to_string(); return "Compount Statement: " + i; }
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

	struct VariableDeclaration : public Statement {
		std::string variable_name{ "" };
		std::shared_ptr<Expression> value{ nullptr };		// OPTIONAL, WILL DO NULL CHECKING

		VariableDeclaration(const std::string& name = "", const std::shared_ptr<Expression>& expr = nullptr) : variable_name{name}, value{expr} {}
		NodeType get_type() const override { return NodeType::VARIABLE_DECLARATION; }
		std::string to_string() const override { return "Variable declaration '" + variable_name + "' " + (value ? value->to_string() : "") + "\n"; }
	};

	struct Assignment : public Statement {
		std::string name{ "" };
		std::shared_ptr<Expression> expression;

		Assignment(const std::string& name, const std::shared_ptr<Expression>& expr) : name{ name }, expression{ expr } {}
		NodeType get_type() const override { return NodeType::ASSIGNMENT; }
		std::string to_string() const override { return "Assignment " + expression->to_string() + " to: " + name; }
	};

	struct IfStatement : public Statement {
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Statement> body;
		std::shared_ptr<Statement> else_body{ nullptr };

		IfStatement(const std::shared_ptr<Expression>& expr, const std::shared_ptr<Statement>& body, const std::shared_ptr<Statement>& else_body = nullptr)
			: condition{ expr }, body{ body }, else_body{ else_body } {}
		NodeType get_type() const override { return NodeType::IF_STATEMENT; }
		std::string to_string() const override { return "If " + condition->to_string() + " then: " + body->to_string() + (else_body ? " else " + else_body->to_string() : ""); }
	};

	struct VoidStatement : public Statement {
		VoidStatement() = default;
		NodeType get_type() const override { return NodeType::VOID_STATEMENT; }
		std::string to_string() const override { return "Empty Statement"; }
	};

	struct WhileStatement : public Statement {
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Statement> body;

		WhileStatement(const std::shared_ptr<Expression>& expr, const std::shared_ptr<Statement>& body, const std::shared_ptr<Statement>& else_body = nullptr)
			: condition{ expr }, body{ body } {}
		NodeType get_type() const override { return NodeType::WHILE_STATEMENT; }
		std::string to_string() const override { return "While " + condition->to_string() + " do: " + body->to_string(); }
	};

	struct BreakStatement : public Statement {
		NodeType get_type() const override { return NodeType::BREAK_STATEMENT; }
		std::string to_string() const override { return "Break\n"; }
	};

	struct ContinueStatement : public Statement {
		NodeType get_type() const override { return NodeType::CONTINUE_STATEMENT; }
		std::string to_string() const override { return "Continue\n"; }
	};

	struct Argument {
		std::string name = "";
		std::shared_ptr<Expression> default_value{ nullptr };
	};

	struct FunctionDeclaration : public Statement {
		std::string function_name{ "" };
		std::vector<std::shared_ptr<Argument>> arguments;
		std::shared_ptr<Statement> body;

		FunctionDeclaration(const std::string& name, const std::vector<std::shared_ptr<Argument>>& args, const std::shared_ptr<Statement>& body)
			: function_name{ name }, arguments{ args }, body{ body } {}
		NodeType get_type() const override { return NodeType::FUNCTION_DECLARATION; }
		std::string to_string() const override { 
			std::string args = "";
			for (const auto& arg : arguments)
				args += arg->name + ", ";
			return "Function declaration '" + function_name + "' (" + args + ") " + body->to_string(); }
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
		Parser(const std::vector<Token>& tokens, ErrorHandler& handler) : m_tokens{ tokens }, m_error_handler{ handler } {}

		const std::shared_ptr<AST>& parse();
	private:
		const std::vector<Token> m_tokens;
		int m_current_index{ 0 };
		ErrorHandler& m_error_handler;

		std::shared_ptr<AST> m_final_tree{ nullptr };

		// UTIL
		void advance() { if(m_current_index < m_tokens.size() - 1) m_current_index++; }
		const Token& current_token() const { return m_tokens[m_current_index]; }
		const Token& peek(uint32_t depth = 1) const { return m_tokens[m_current_index + depth]; }
		void consume(TokenType type, const std::string& error);

		// EXPRESSIONS
		std::shared_ptr<Expression> expression();
		std::shared_ptr<Expression> logical_and();
		std::shared_ptr<Expression> equality();
		std::shared_ptr<Expression> relational();
		std::shared_ptr<Expression> arithmetic();
		std::shared_ptr<Expression> term();
		std::shared_ptr<Expression> factor();

		// STATEMENTS
		std::shared_ptr<Statement> statement();
		std::shared_ptr<Statement> compount_statement();
		std::shared_ptr<Statement> log_statement();
		std::shared_ptr<Statement> expression_statement();
		std::shared_ptr<Statement> variable_declaration();
		std::shared_ptr<Statement> assignment();
		std::shared_ptr<Statement> if_statement();
		std::shared_ptr<Statement> while_statement();
		std::shared_ptr<Statement> function();
	};
}