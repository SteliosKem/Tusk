#include "Compiler.h"

namespace Tusk {
	void Compiler::write(uint8_t byte) {
		m_bytecode_out.write_byte(byte);
	}

	void Compiler::write(uint8_t byte_a, uint8_t byte_b) {
		m_bytecode_out.write_byte(byte_a);
		m_bytecode_out.write_byte(byte_b);
	}

	uint8_t Compiler::add_constant(Value value) {
		return m_bytecode_out.write_value(value);
	}

	void Compiler::expression(const std::shared_ptr<Expression>& expression) {
		switch (expression->get_type()) {
		case NodeType::BINARY_OPERATION:
			binary_operation(std::static_pointer_cast<BinaryOperation>(expression));
			break;
		case NodeType::UNARY_OPERATION:
			unary_operation(std::static_pointer_cast<UnaryOperation>(expression));
			break;
		case NodeType::NUMBER_VALUE:
			number(std::static_pointer_cast<Number>(expression));
			break;
		case NodeType::BOOL_VALUE:
			boolean(std::static_pointer_cast<BoolValue>(expression));
			break;
		case NodeType::NAME:
			name(std::static_pointer_cast<Name>(expression));
			break;
		case NodeType::VOID:
			write((uint8_t)Instruction::VOID);
			break;
		case NodeType::STRING:
			write((uint8_t)Instruction::VAL_INDEX, add_constant(std::static_pointer_cast<StringLiteral>(expression)->value));
			break;
		}
	}

	const Unit& Compiler::compile() {
		for (std::shared_ptr<Statement> stmt : m_ast->statements) {
			statement(stmt);
		}
		write((uint8_t)Instruction::RETURN);
		return m_bytecode_out;
	}

	void Compiler::number(const std::shared_ptr<Number>& number) {
		write((uint8_t)Instruction::VAL_INDEX, add_constant(number->value));
	}

	void Compiler::boolean(const std::shared_ptr<BoolValue>& boolean) {
		write((uint8_t)Instruction::VAL_INDEX, add_constant(boolean->value));
	}

	void Compiler::name(const std::shared_ptr<Name>& name) {
		write((uint8_t)Instruction::GET_GLOBAL, add_constant(Value(std::make_shared<String>(name->string))));
	}

	void Compiler::binary_operation(const std::shared_ptr<BinaryOperation>& operation) {
		expression(operation->left_expression);
		expression(operation->right_expression);
		switch (operation->operator_token.type)
		{
		case TokenType::PLUS:
			write((uint8_t)Instruction::ADD);
			break;
		case TokenType::MINUS:
			write((uint8_t)Instruction::SUBTRACT);
			break;
		case TokenType::STAR:
			write((uint8_t)Instruction::MULTIPLY);
			break;
		case TokenType::SLASH:
			write((uint8_t)Instruction::DIVIDE);
			break;
		case TokenType::EQUAL_EQUAL:
			write((uint8_t)Instruction::EQUAL);
			break;
		case TokenType::BANG_EQUAL:
			write((uint8_t)Instruction::NOT_EQUAL);
			break;
		case TokenType::GREATER:
			write((uint8_t)Instruction::GREATER);
			break;
		case TokenType::LESS:
			write((uint8_t)Instruction::LESS);
			break;
		case TokenType::GREATER_EQUAL:
			write((uint8_t)Instruction::GREATER_EQUAL);
			break;
		case TokenType::LESS_EQUAL:
			write((uint8_t)Instruction::LESS_EQUAL);
			break;
		default:
			break;
		}
		
	}

	void Compiler::unary_operation(const std::shared_ptr<UnaryOperation>& operation) {
		expression(operation->right_expression);
		switch (operation->operator_token.type)
		{
		case TokenType::MINUS:
			write((uint8_t)Instruction::NEGATE);
			break;
		default:
			break;
		}

	}

	void Compiler::statement(const std::shared_ptr<Statement>& statement) {
		switch (statement->get_type()) {
		case NodeType::LOG_STATEMENT:
			log_statement(std::static_pointer_cast<LogStatement>(statement));
			break;
		case NodeType::EXPRESSION_STATEMENT:
			expression_statement(std::static_pointer_cast<ExpressionStatement>(statement));
			break;
		case NodeType::VARIABLE_DECLARATION:
			variable_declaration(std::static_pointer_cast<VariableDeclaration>(statement));
			break;
		case NodeType::ASSIGNMENT:
			assignment(std::static_pointer_cast<Assignment>(statement));
			break;
		}
	}
	void Compiler::log_statement(const std::shared_ptr<LogStatement>& log_statement) {
		expression(log_statement->output);
		write((uint8_t)Instruction::LOG, (uint8_t)Instruction::POP);
	}
	void Compiler::expression_statement(const std::shared_ptr<ExpressionStatement>& expression_statement) {
		expression(expression_statement->expression);
		write((uint8_t)Instruction::POP);
	}

	void Compiler::variable_declaration(const std::shared_ptr<VariableDeclaration>& variable_decl) {
		if (variable_decl->value)
			expression(variable_decl->value);
		else
			write((uint8_t)Instruction::VOID);
		write((uint8_t)Instruction::MAKE_GLOBAL, add_constant(Value(std::make_shared<String>(variable_decl->variable_name))));
	}

	void Compiler::assignment(const std::shared_ptr<Assignment>& assignment) {
		expression(assignment->expression);
		write((uint8_t)Instruction::SET_GLOBAL, add_constant(Value(std::make_shared<String>(assignment->name))));
	}
}