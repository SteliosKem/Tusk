#pragma once
#include "Value.h"
#include <cstdint>
#include <vector>
#include <memory>
#include "Bytecode.h"
#include "Token.h"
#include "Error.h"
#include <unordered_map>

namespace Tusk {
	namespace Standard {
		static Value read(int arg_count, Value* arguments);
	}

	enum class OperationType {
		ARITHMETIC,
		RELATIONAL,
		LOGICAL
	};

	enum class Result {
		OK,
		RUNTIME_ERROR
	};

	class Emulator {
	public:
		Emulator(ErrorHandler& handler) : m_error_handler(handler) { m_call_stack.push_back({}); init(); }
		Emulator(const Unit* bytes, ErrorHandler& handler) : m_error_handler{ handler } { m_call_stack.push_back({}); init(); }
		
		Result run(const Unit* bytes);
		const std::unordered_map<std::string, Value>& get_global_table() { return m_global_table; }
	private:
		void init();
		Result run();

		struct CallInfo {
			std::string function_name{"script"};
			uint64_t stack_size_before_args{ 0 };
		};

		std::vector<Value> m_stack;
		std::vector<CallInfo> m_call_stack;

		Value m_return_value_register;
		//const Unit* m_bytes{ nullptr };
		//uint32_t* m_instruction_index{ 0 };
		const Unit* bytes() { return m_program_data[m_program_data.size() - 1].bytes; }
		uint32_t& instruction_index() { return m_program_data[m_program_data.size() - 1].instruction_index; }
		ErrorHandler& m_error_handler;

		struct ProgramData {
			const Unit* bytes{ nullptr };
			uint32_t instruction_index{ 0 };
		};

		std::vector<ProgramData> m_program_data;
		void push_data(const ProgramData& data) {
			m_program_data.push_back(data);
			//m_instruction_index = &m_program_data[m_program_data.size() - 1].instruction_index;
			//m_bytes = m_program_data[m_program_data.size() - 1].bytes;
		}
		void pop_data() {
			m_program_data.pop_back();
			//m_instruction_index = &m_program_data[m_program_data.size() - 1].instruction_index;
			//m_bytes = m_program_data[m_program_data.size() - 1].bytes;
		}

		// TABLE
		std::unordered_map<std::string, Value> m_global_table;

		// UTIL
		uint8_t next_byte();
		const Value& read_value();
		void push_stack(const Value& val);
		Value pop_stack();
		Value& stack_top(uint32_t offset = 0) { return m_stack[m_stack.size() - 1 - offset]; }

		//
		Result binary_operation(TokenType operation);
		void str_concatenate(const std::shared_ptr<StringObject>& str1, const std::shared_ptr<StringObject>& str2);
		bool equality();
		Result call(const Value& value_to_call, uint8_t arg_count);
		Result call_method(const std::string& name, uint8_t arg_count);

		void make_standard_fn(const std::string& name, StandardFnType func);
	};
}