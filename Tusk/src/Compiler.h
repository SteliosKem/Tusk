#pragma once
#include "Bytecode.h"
#include "Parser.h"
#include "Value.h"


namespace Tusk {
	class Optimizer {};

	class Compiler {
	public:
		Compiler(const std::shared_ptr<ASTNode>& tree) : m_ast{ tree } {}

		const Unit& compile();
	private:
		std::shared_ptr<ASTNode> m_ast;
		Unit m_bytecode_out;

		// UTILS
		void write(uint8_t byte);						// Writes one byte to the bytecode
		void write(uint8_t byte_a, uint8_t byte_b);		// Writes two bytes to the bytecode
		uint8_t add_constant(Value value);				// Adds a constant to the constant pool and returns its index

		//
		void expression(const std::shared_ptr<Expression>& expression);
		void binary_operation(const std::shared_ptr<BinaryOperation>& operation);
		void number(const std::shared_ptr<Number>& number);
	};
}