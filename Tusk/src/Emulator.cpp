#include "Emulator.h"
#include "Bytecode.h"
#include <iostream>

namespace Tusk {
	inline bool is_num(const Value& val) {
		return val.is<int64_t>() || val.is<double>();
	}

	inline uint8_t Emulator::next_byte() {
		return (*m_bytes)[m_instruction_index++];			// Return byte at index and advance it
	}

	inline const Value& Emulator::read_value() {
		return (*m_bytes).get_values()[next_byte()];		// Get the constant index and pass it to the constant pool to return the value
	}

	void Emulator::push_stack(const Value& val) {			// Push value to the stack
		m_stack.push_back(val);
	}

	Value Emulator::pop_stack() {							// Pop stack value and return it
		Value to_ret = m_stack[m_stack.size() - 1];
		m_stack.pop_back();
		return to_ret;
	}

	void Emulator::binary_operation(TokenType operation) {
		Value b = pop_stack();
		Value a = pop_stack();

		switch (operation) {
		case TokenType::PLUS:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				+ (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		case TokenType::MINUS:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				- (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		case TokenType::STAR:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				* (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		case TokenType::SLASH:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				/ (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		}
	}

	Result Emulator::run(const Unit* unit) {
		m_bytes = unit;
		Result res = run();
		m_instruction_index = 0;
		m_stack.clear();
		return res;
	}

	Result Emulator::run() {
 		Instruction instruction;
		while (true) {
			switch (instruction = (Instruction)next_byte()) {
			case Instruction::VAL_INDEX:
				m_stack.push_back(read_value());
				break;
			case Instruction::ADD:
				if (is_num(stack_top(1)) && is_num(stack_top()))
					binary_operation(TokenType::PLUS);
				else
					m_error_handler.report_error("Operands must be numbers", {}, ErrorType::RUNTIME_ERROR);
				break;
			case Instruction::SUBTRACT:
				binary_operation(TokenType::MINUS);
				break;
			case Instruction::MULTIPLY:
				binary_operation(TokenType::STAR);
				break;
			case Instruction::DIVIDE:
				binary_operation(TokenType::SLASH);
				break;
			case Instruction::RETURN:
				//std::cout << pop_stack();
				return Result::OK;
			case Instruction::LOG:
				std::cout << stack_top();
				break;
			case Instruction::POP:
				pop_stack();
				break;
			case Instruction::NEGATE:
				Value val = pop_stack();
				push_stack(- (val.is<int64_t>() ? val.get<int64_t>() : val.get<double>()));
				break;
			}

		}
	}
}