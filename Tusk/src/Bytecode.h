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
		VAL_INDEX
	};

	struct Unit {
	public:
		Unit() = default;
		Unit(size_t vec_size) { m_bytecode.reserve(vec_size); }

		void write_byte(uint8_t byte) { m_bytecode.push_back(byte); }
		uint8_t write_value(Value value) { m_values.push_back(value); return m_values.size() - 1; }
		uint8_t operator[](uint8_t index) const { if(index < m_bytecode.size()) return m_bytecode[index]; }
		std::string disassemble() const;
	private:
		std::vector<uint8_t> m_bytecode;
		std::vector<Value> m_values;
	};
}