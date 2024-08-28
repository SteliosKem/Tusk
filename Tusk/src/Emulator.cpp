#include "Emulator.h"
#include "Bytecode.h"
#include <iostream>

namespace Tusk {
	inline bool is_true(const Value& val) {
		if (val.is<bool>())
			return val.get<bool>();
		else return false;
	}

	inline bool is_num(const Value& val) {
		return val.is<int64_t>() || val.is<double>();
	}

	inline uint8_t Emulator::next_byte() {
		return (*bytes())[instruction_index()++];			// Return byte at index and advance it
	}

	inline const Value& Emulator::read_value() {
		return (*bytes()).get_values()[next_byte()];		// Get the constant index and pass it to the constant pool to return the value
	}

	void Emulator::push_stack(const Value& val) {			// Push value to the stack
		m_stack.push_back(val);
	}

	Value Emulator::pop_stack() {							// Pop stack value and return it
		Value to_ret = m_stack[m_stack.size() - 1];
		m_stack.pop_back();
		return to_ret;
	}

	Result Emulator::binary_operation(TokenType operation) {
		if (!is_num(stack_top(1)) || !is_num(stack_top())) {
			m_error_handler.report_error("Operands must be numbers", {}, ErrorType::RUNTIME_ERROR);
			return Result::RUNTIME_ERROR;
		}
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
		case TokenType::LESS:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				< (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		case TokenType::GREATER:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				> (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		case TokenType::GREATER_EQUAL:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				>= (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		case TokenType::LESS_EQUAL:
			push_stack((a.is<double>() ? a.get<double>() : a.get<int64_t>())
				<= (b.is<double>() ? b.get<double>() : b.get<int64_t>()));
			break;
		}
		return Result::OK;
	}

	bool Emulator::equality() {
		Value b = pop_stack();
		Value a = pop_stack();

		if (is_num(a) && is_num(b)) {
			return (a.is<double>() ? a.get<double>() : a.get<int64_t>())
				== (b.is<double>() ? b.get<double>() : b.get<int64_t>());
		}
		if (b.get_type() != a.get_type())
				return false;
		if (b.is<bool>())
			return a.get<bool>() == b.get<bool>();
	}

	Result Emulator::run(const Unit* unit) {
		push_data({unit});
		Result res = run();
		//m_instruction_index = 0;
		pop_data();
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
				if (binary_operation(TokenType::PLUS) != Result::OK)
					return Result::RUNTIME_ERROR;
				break;
			case Instruction::SUBTRACT:
				if (binary_operation(TokenType::MINUS) != Result::OK)
					return Result::RUNTIME_ERROR;
				break;
			case Instruction::MULTIPLY:
				if (binary_operation(TokenType::STAR) != Result::OK)
					return Result::RUNTIME_ERROR;
				break;
			case Instruction::DIVIDE:
				if (binary_operation(TokenType::SLASH) != Result::OK)
					return Result::RUNTIME_ERROR;
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
			case Instruction::NEGATE: {
				Value val = pop_stack();
				if (val.is<int64_t>() || val.is<double>())
					push_stack(-(val.is<int64_t>() ? val.get<int64_t>() : val.get<double>()));
				else {
					m_error_handler.report_error("Operand must be number", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				break;
			}
			case Instruction::LESS:
				if (binary_operation(TokenType::LESS) != Result::OK)
					return Result::RUNTIME_ERROR;
				break;
			case Instruction::GREATER:
				if (binary_operation(TokenType::GREATER) != Result::OK)
					return Result::RUNTIME_ERROR;
				break;
			case Instruction::GREATER_EQUAL:
				if (binary_operation(TokenType::GREATER_EQUAL) != Result::OK)
					return Result::RUNTIME_ERROR;
				break;
			case Instruction::LESS_EQUAL:
				if (binary_operation(TokenType::LESS_EQUAL) != Result::OK)
					return Result::RUNTIME_ERROR;
				break;
			case Instruction::EQUAL:
				push_stack(equality());
				break;
			case Instruction::NOT_EQUAL:
				push_stack(!equality());
				break;
			case Instruction::NOT: {
				Value val = pop_stack();
				if (val.is<bool>())
					push_stack(!val.get<bool>());
				else {
					m_error_handler.report_error("Operand must be boolean", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				break;
			}
			case Instruction::VOID:
				push_stack(Value(std::make_shared<VoidValue>()));
				break;
			case Instruction::MAKE_GLOBAL: {
				std::shared_ptr<String> val = read_value().get_object<String>();
				if (m_global_table.find(val->string) != m_global_table.end()) {
					m_error_handler.report_error("Global name '" + val->string + "' already exists", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				m_global_table[val->string] = pop_stack();
				break;
			}
			case Instruction::GET_GLOBAL: {
				std::shared_ptr<String> val = read_value().get_object<String>();
				push_stack(m_global_table[val->string]);
				break;
			}
			case Instruction::SET_GLOBAL: {
				std::shared_ptr<String> val = read_value().get_object<String>();
				m_global_table[val->string] = pop_stack();
				break;
			}
			case Instruction::JUMP_IF_FALSE: {
				int64_t val = read_value().get<int64_t>();
				if (!is_true(pop_stack()))
					instruction_index() = val;
				break;
			}
			case Instruction::JUMP: {
				instruction_index() = read_value().get<int64_t>();
				break;
			}
			case Instruction::SET_LOCAL: {
				int64_t val = read_value().get<int64_t>();
				m_stack[val] = stack_top();
				break;
			}
			case Instruction::GET_LOCAL: {
				int64_t val = read_value().get<int64_t>();
				push_stack(m_stack[val]);
				break;
			}
			case Instruction::CALL: {
				call();
				break;
			}
			}

		}
	}

	Result Emulator::call() {
		Value val = pop_stack();
		if (!(val.is<std::shared_ptr<ValueObject>>() && val.get<std::shared_ptr<ValueObject>>()->get_type() == ObjectType::FUNCTION)) {
			m_error_handler.report_error("Cannot call non-function objects", {}, ErrorType::RUNTIME_ERROR);
			return Result::RUNTIME_ERROR;
		}
		std::shared_ptr<Function> func = val.get_object<Function>();
		push_data({ func->code_unit.get(), 0});
		Result res = run();
		pop_data();
		push_stack(Value(nullptr));
		return res;
	}
}