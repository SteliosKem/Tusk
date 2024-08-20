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
		case NodeType::NUMBER_VALUE:
			number(std::static_pointer_cast<Number>(expression));
			break;
		}
	}

	const Unit& Compiler::compile() {
		expression(std::static_pointer_cast<Expression>(m_ast));
		return m_bytecode_out;
	}

	void Compiler::number(const std::shared_ptr<Number>& number) {
		write((uint8_t)Instruction::VAL_INDEX, add_constant(number->value));
	}

	void Compiler::binary_operation(const std::shared_ptr<BinaryOperation>& operation) {
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
		default:
			break;
		}
		expression(operation->left_expression);
		expression(operation->right_expression);
	}
}