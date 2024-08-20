#include "Bytecode.h"

namespace Tusk {
	static std::string instruction_str(const std::string& name) {
		return name + '\n';
	}

	static std::string index_str(uint8_t index) {
		return "INDEX {" + std::to_string((int)index) + "}\n";
	}

	std::string Unit::disassemble() const {
		std::string out = "";
		int i = 0;
		uint8_t instruction;
		while (i < m_bytecode.size()) {
			instruction = m_bytecode[i];
			switch ((Instruction)instruction) {
			case Instruction::ADD:
				out += instruction_str("ADD");
				break;
			case Instruction::SUBTRACT:
				out += instruction_str("SUBTRACT");
				break;
			case Instruction::MULTIPLY:
				out += instruction_str("MULTIPLY");
				break;
			case Instruction::DIVIDE:
				out += instruction_str("DIVIDE");
				break;
			case Instruction::VAL_INDEX:
				out += index_str(m_bytecode[++i]);
				break;
			default:
				break;
			}
			i++;
		}

		return out;
	}

}