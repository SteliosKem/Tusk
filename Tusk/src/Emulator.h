#pragma once
#include "Value.h"
#include <cstdint>
#include <vector>
#include <memory>
#include "Bytecode.h"
#include "Token.h"

namespace Tusk {
	enum class Result {
		OK,
		RUNTIME_ERROR
	};

	class Emulator {
	public:
		Emulator() = default;
		Emulator(const Unit* bytes) : m_bytes{bytes} {}

		Result run();
		Result run(const Unit* bytes);
	private:
		std::vector<Value> m_stack;
		const Unit* m_bytes{ nullptr };
		uint32_t m_instruction_index{ 0 };

		// UTIL
		uint8_t next_byte();
		const Value& read_value();
		void push_stack(const Value& val);
		Value pop_stack();

		//
		void binary_operation(TokenType operation);
	};
}