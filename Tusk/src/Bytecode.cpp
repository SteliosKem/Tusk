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
			case Instruction::POP:
				out += instruction_str("POP");
				break;
			case Instruction::LOG:
				out += instruction_str("LOG");
				break;
			case Instruction::NEGATE:
				out += instruction_str("NEGATE");
				break;
			case Instruction::GREATER:
				out += instruction_str("GREATER");
				break;
			case Instruction::GREATER_EQUAL:
				out += instruction_str("GREATER_EQUAL");
				break;
			case Instruction::LESS:
				out += instruction_str("LESS");
				break;
			case Instruction::LESS_EQUAL:
				out += instruction_str("LESS_EQUAL");
				break;
			case Instruction::EQUAL:
				out += instruction_str("EQUAL");
				break;
			case Instruction::NOT_EQUAL:
				out += instruction_str("NOT_EQUAL");
				break;
			case Instruction::NOT:
				out += instruction_str("NOT");
				break;
			case Instruction::AND:
				out += instruction_str("AND");
				break;
			case Instruction::OR:
				out += instruction_str("OR");
				break;
			case Instruction::VOID:
				out += instruction_str("VOID");
				break;
			case Instruction::MAKE_GLOBAL:
				out += instruction_str("MAKE_GLOBAL");
				break;
			case Instruction::GET_GLOBAL:
				out += instruction_str("GET_GLOBAL");
				break;
			case Instruction::SET_GLOBAL:
				out += instruction_str("SET_GLOBAL");
				break;
			default:
				break;
			}
			i++;
		}

		return out;
	}

}