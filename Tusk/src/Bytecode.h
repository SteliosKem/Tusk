#pragma once
#include <vector>
#include <string>
#include "Value.h"

namespace Tusk {
	enum class Instruction : uint8_t {
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		RETURN,
		VAL_INDEX,
		POP,
		LOG,
		NEGATE,
		EQUAL,
		NOT_EQUAL,
		GREATER,
		LESS,
		GREATER_EQUAL,
		LESS_EQUAL,
		NOT,
		AND,
		OR,
		VOID,
		MAKE_GLOBAL,
		GET_GLOBAL,
		SET_GLOBAL,
		JUMP,
		JUMP_IF_FALSE,
		SET_LOCAL,
		GET_LOCAL,
	};

	struct Unit {
	public:
		Unit() = default;
		Unit(size_t vec_size) { m_bytecode.reserve(vec_size); }

		void write_byte(uint8_t byte) { m_bytecode.push_back(byte); }
		uint8_t write_value(Value value) { m_values.push_back(value); return m_values.size() - 1; }
		std::vector<Value>& get_values() { return m_values; }
		uint8_t operator[](uint8_t index) const { if (index < m_bytecode.size()) return m_bytecode[index]; }
		const std::vector<Value>& get_values() const { return m_values; }
		int64_t index() const { return m_bytecode.size(); }
		std::string disassemble() const;
	private:
		std::vector<uint8_t> m_bytecode;
		std::vector<Value> m_values;
	};
}