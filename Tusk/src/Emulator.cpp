#include "pch.h"
#include "Emulator.h"
#include "Bytecode.h"

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
				if (m_call_stack.size() != 1) {
					m_return_value_register = pop_stack();
				}
				return Result::OK;
			case Instruction::LOG:
				std::cout << stack_top();
				break;
			case Instruction::LOGL:
				std::cout << stack_top() << '\n';
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
				push_stack(Value(std::make_shared<VoidObject>()));
				break;
			case Instruction::MAKE_GLOBAL: {
				std::shared_ptr<StringObject> val = read_value().get_object<StringObject>();
				if (m_global_table.find(val->string) != m_global_table.end()) {
					m_error_handler.report_error("Global name '" + val->string + "' already exists", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				m_global_table[val->string] = pop_stack();
				break;
			}
			case Instruction::GET_GLOBAL: {
				std::shared_ptr<StringObject> val = read_value().get_object<StringObject>();
				push_stack(m_global_table[val->string]);
				break;
			}
			case Instruction::SET_GLOBAL: {
				std::shared_ptr<StringObject> val = read_value().get_object<StringObject>();
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
				m_stack[val + m_call_stack[m_call_stack.size() - 1].stack_size_before_args] = stack_top();
				break;
			}
			case Instruction::GET_LOCAL: {
				int64_t val = read_value().get<int64_t>();
				push_stack(m_stack[val + m_call_stack[m_call_stack.size() - 1].stack_size_before_args]);
				break;
			}
			case Instruction::CALL: {
				uint8_t arg_count = next_byte();
				Result res = call(stack_top(arg_count), arg_count);
				if (res != Result::OK)
					return res;
				break;
			}
			case Instruction::GET_MEMBER: {
				Value val = pop_stack();
				if (val.is<std::shared_ptr<ValueObject>>()) {
					switch (val.get_object_type())
					{
					case ObjectType::INSTANCE: {
						std::shared_ptr<InstanceObject> instance = val.get_object<InstanceObject>();
						const std::string& name = read_value().get_object<StringObject>()->string;
						if (instance->public_members.find(name) == instance->public_members.end()) {
							m_error_handler.report_error("Instance of class " + instance->class_ref.class_name + " does not have member " + name, {}, ErrorType::RUNTIME_ERROR);
							return Result::RUNTIME_ERROR;
						}
						else
							push_stack(instance->public_members[name]);
						break;
					}
					case ObjectType::ENUM: {
						std::shared_ptr<EnumObject> instance = val.get_object<EnumObject>();
						const std::string& name = read_value().get_object<StringObject>()->string;
						if (std::find(instance->values.begin(), instance->values.end(), name) == instance->values.end()) {
							m_error_handler.report_error("Enum " + instance->name + " does not have value " + name, {}, ErrorType::RUNTIME_ERROR);
							return Result::RUNTIME_ERROR;
						}
						else
							push_stack(Value(std::make_shared<EnumValue>(instance, name)));
						break;
					}
					default:
						m_error_handler.report_error("Cannot access members of a non-instance or non-enum", {}, ErrorType::RUNTIME_ERROR);
						return Result::RUNTIME_ERROR;
					}
				}
				else {
					m_error_handler.report_error("Cannot access members of a non-instance or non-enum", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				
				break;
			}
			case Instruction::SET_MEMBER: {
				Value val = pop_stack();
				if (!(val.is<std::shared_ptr<ValueObject>>() && val.get_object_type() == ObjectType::INSTANCE)) {
					m_error_handler.report_error("Cannot set members of a non-instance", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				std::shared_ptr<InstanceObject> instance = val.get_object<InstanceObject>();
				const std::string& name = read_value().get_object<StringObject>()->string;
				//if (instance->public_members.find(name) == instance->public_members.end()) {
				//	m_error_handler.report_error("Instance of class " + instance->class_ref.class_name + " does not have member " + name, {}, ErrorType::RUNTIME_ERROR);
				//	return Result::RUNTIME_ERROR;
				//}
				//else
				instance->public_members[name] = pop_stack();
				break;
			}
			case Instruction::MAKE_MEMBER: {
				std::shared_ptr<StringObject> val = read_value().get_object<StringObject>();
				if (m_global_table.find(val->string) != m_global_table.end()) {
					m_error_handler.report_error("Global name '" + val->string + "' already exists", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				stack_top().get_object<ClassObject>()->public_members[val->string] = pop_stack();
				break;
			}
			case Instruction::METHOD_CALL: {
				uint8_t arg_count = next_byte();
				const std::string& name = read_value().get_object<StringObject>()->string;
				Value val = stack_top(arg_count);
				if (!(val.is<std::shared_ptr<ValueObject>>() && val.get_object_type() == ObjectType::INSTANCE)) {
					m_error_handler.report_error("Cannot access members of a non-instance", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				std::shared_ptr<InstanceObject> instance = val.get_object<InstanceObject>();
				if (instance->public_members.find(name) == instance->public_members.end()) {
					m_error_handler.report_error("Instance of class " + instance->class_ref.class_name + " does not have member " + name, {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				if (instance->public_members[name].get_object_type() != ObjectType::FUNCTION) {
					m_error_handler.report_error("Cannot call non-function and non-class objects", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				Result res = call_method(name, arg_count);
				if (res != Result::OK)
					return res;
				break;
			}
			case Instruction::INHERIT: {
				Value val = pop_stack();
				if (!(val.is<std::shared_ptr<ValueObject>>() && val.get_object_type() == ObjectType::CLASS)) {
					m_error_handler.report_error("Cannot inherit from non-class objects", {}, ErrorType::RUNTIME_ERROR);
					return Result::RUNTIME_ERROR;
				}
				std::shared_ptr<ClassObject> _class = stack_top().get_object<ClassObject>();
				for (const auto& [key, value] : val.get_object<ClassObject>()->public_members) {
					_class->public_members[key] = value;
				}
			}

			}

		}
	}

	Result Emulator::call(const Value& value_to_call, uint8_t arg_count) {
		if (value_to_call.is<std::shared_ptr<ValueObject>>()) {
			switch (value_to_call.get_object_type()) {
			case ObjectType::FUNCTION: {
				std::shared_ptr<FunctionObject> func = value_to_call.get_object<FunctionObject>();
				push_data({ func->code_unit.get(), 0 });
				m_call_stack.push_back(CallInfo{ func->function_name, m_stack.size() - arg_count - 1 });
				Result res = run();
				m_call_stack.pop_back();
				pop_data();

				push_stack(m_return_value_register);
				return res;
			}
			case ObjectType::CLASS: {
				std::shared_ptr<ClassObject> class_obj = value_to_call.get_object<ClassObject>();
				std::shared_ptr<InstanceObject> instance = std::make_shared<InstanceObject>(*class_obj.get());
				instance->private_members = class_obj->private_members;
				instance->public_members = class_obj->public_members;
				Result res = Result::OK;
				m_stack[m_stack.size() - arg_count - 1] = Value(instance);
				if (instance->public_members.find("make") != instance->public_members.end()) {
					res = call(instance->public_members["make"], arg_count);
					pop_stack();
				}
				return res;
			}
			default:
				break;
			}
		}
		m_error_handler.report_error("Cannot call non-function and non-class objects", {}, ErrorType::RUNTIME_ERROR);
		return Result::RUNTIME_ERROR;
	}

	Result Emulator::call_method(const std::string& name, uint8_t arg_count) {
		Value value_to_call = stack_top(arg_count);
		if (value_to_call.is<std::shared_ptr<ValueObject>>()) {
			if(value_to_call.get_object_type() == ObjectType::INSTANCE) {
				std::shared_ptr<FunctionObject> func = value_to_call.get_object<InstanceObject>()->public_members[name].get_object<FunctionObject>();
				push_data({ func->code_unit.get(), 0 });
				m_call_stack.push_back(CallInfo{ func->function_name, m_stack.size() - arg_count - 1 });
				Result res = run();
				m_call_stack.pop_back();
				pop_data();
				pop_stack();
				push_stack(m_return_value_register);
				return res;
			}
		}
		m_error_handler.report_error("Cannot call non-function and non-class objects", {}, ErrorType::RUNTIME_ERROR);
		return Result::RUNTIME_ERROR;
	}
}