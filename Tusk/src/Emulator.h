#pragma once
#include "Value.h"
#include <cstdint>
#include <vector>
#include <memory>
#include "Bytecode.h"
#include "Token.h"
#include "Error.h"

namespace Tusk {
	enum class Result {
		OK,
		RUNTIME_ERROR
	};

	class Emulator {
	public:
		Emulator(ErrorHandler& handler) : m_error_handler(handler) {}
		Emulator(const Unit* bytes, ErrorHandler& handler) : m_bytes{ bytes }, m_error_handler{ handler } {}

		Result run();
		Result run(const Unit* bytes);
	private:
		std::vector<Value> m_stack;
		const Unit* m_bytes{ nullptr };
		uint32_t m_instruction_index{ 0 };
		ErrorHandler& m_error_handler;

		// UTIL
		uint8_t next_byte();
		const Value& read_value();
		void push_stack(const Value& val);
		Value pop_stack();
		Value& stack_top(uint32_t offset = 0) { return m_stack[m_stack.size() - 1 - offset]; }

		//
		void binary_operation(TokenType operation);
	};
}