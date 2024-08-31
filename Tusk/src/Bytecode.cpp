#include "Bytecode.h"

namespace Tusk {
	static std::string instruction_str(const std::string& name) {
		return name + '\n';
	}

	static std::string complex_str(const std::string& name, uint8_t index) {
		return name + " {" + std::to_string((int)index) + "}\n";
	}

	std::string Unit::disassemble(const Unit& unit) const {
		std::string out = "";
		int i = 0;
		uint8_t instruction;
		for (const Value& val : unit.m_values) {
			if (val.is<std::shared_ptr<ValueObject>>() && val.get<std::shared_ptr<ValueObject>>()->get_type() == ObjectType::FUNCTION) {
				out += "FUNCTION " + val.get_object<FunctionObject>()->function_name + "\n" + disassemble(*val.get_object<FunctionObject>()->code_unit.get()) + "\n\n";
			}
		}
		while (i < unit.m_bytecode.size()) {
			instruction = unit.m_bytecode[i];
			switch ((Instruction)instruction) {
			case Instruction::ADD:
				out += std::to_string(i) + " " + instruction_str("ADD");
				break;
			case Instruction::SUBTRACT:
				out += std::to_string(i) + " " + instruction_str("SUBTRACT");
				break;
			case Instruction::MULTIPLY:
				out += std::to_string(i) + " " + instruction_str("MULTIPLY");
				break;
			case Instruction::DIVIDE:
				out += std::to_string(i) + " " + instruction_str("DIVIDE");
				break;
			case Instruction::VAL_INDEX:
				out += std::to_string(i) + " " + complex_str("INDEX", unit.m_bytecode[++i]);
				break;
			case Instruction::POP:
				out += std::to_string(i) + " " + instruction_str("POP");
				break;
			case Instruction::RETURN:
				out += std::to_string(i) + " " + instruction_str("RETURN");
				break;
			case Instruction::LOG:
				out += std::to_string(i) + " " + instruction_str("LOG");
				break;
			case Instruction::NEGATE:
				out += std::to_string(i) + " " + instruction_str("NEGATE");
				break;
			case Instruction::GREATER:
				out += std::to_string(i) + " " + instruction_str("GREATER");
				break;
			case Instruction::GREATER_EQUAL:
				out += std::to_string(i) + " " + instruction_str("GREATER_EQUAL");
				break;
			case Instruction::LESS:
				out += std::to_string(i) + " " + instruction_str("LESS");
				break;
			case Instruction::LESS_EQUAL:
				out += std::to_string(i) + " " + instruction_str("LESS_EQUAL");
				break;
			case Instruction::EQUAL:
				out += std::to_string(i) + " " + instruction_str("EQUAL");
				break;
			case Instruction::NOT_EQUAL:
				out += std::to_string(i) + " " + instruction_str("NOT_EQUAL");
				break;
			case Instruction::NOT:
				out += std::to_string(i) + " " + instruction_str("NOT");
				break;
			case Instruction::AND:
				out += std::to_string(i) + " " + instruction_str("AND");
				break;
			case Instruction::OR:
				out += std::to_string(i) + " " + instruction_str("OR");
				break;
			case Instruction::VOID:
				out += std::to_string(i) + " " + instruction_str("VOID");
				break;
			case Instruction::MAKE_GLOBAL:
				out += std::to_string(i) + " " + instruction_str("MAKE_GLOBAL");
				break;
			case Instruction::GET_GLOBAL:
				out += std::to_string(i) + " " + instruction_str("GET_GLOBAL");
				break;
			case Instruction::SET_GLOBAL:
				out += std::to_string(i) + " " + instruction_str("SET_GLOBAL");
				break;
			case Instruction::JUMP_IF_FALSE:
				i++;
				out += std::to_string(i) + " " + complex_str("JUMP_IF_FALSE", m_values[unit.m_bytecode[i]].get<int64_t>());
				break;
			case Instruction::JUMP:
				i++;
				out += std::to_string(i) + " " + complex_str("JUMP", m_values[unit.m_bytecode[i]].get<int64_t>());
				break;
			case Instruction::GET_LOCAL:
				i++;
				out += std::to_string(i) + " " + instruction_str("GET_LOCAL");
				break;
			case Instruction::SET_LOCAL:
				i++;
				out += std::to_string(i) + " " + instruction_str("SET_LOCAL");
				break;
			case Instruction::CALL:
				i++;
				out += std::to_string(i) + " " + instruction_str("CALL");
				break;
			case Instruction::GET_MEMBER:
				i++;
				out += std::to_string(i) + " " + instruction_str("GET_MEMBER");
				break;
			case Instruction::SET_MEMBER:
				i++;
				out += std::to_string(i) + " " + instruction_str("SET_MEMBER");
				break;
			case Instruction::METHOD_CALL:
				i++;
				out += std::to_string(i) + " " + instruction_str("METHOD_CALL");
				break;
			default:
				break;
			}
			i++;
		}

		return out;
	}
	std::string Unit::disassemble() const {
		return disassemble(*this);
	}
}