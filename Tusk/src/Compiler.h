#pragma once
#include "Bytecode.h"
#include "Parser.h"
#include "Value.h"
#include <unordered_map>

namespace Tusk {
	class Optimizer {};

	class Compiler {
	public:
		Compiler(const std::shared_ptr<AST>& tree, ErrorHandler& handler) : m_ast{ tree }, m_error_handler { handler } {}

		const Unit& compile();
	private:
		std::shared_ptr<AST> m_ast;
		Unit m_bytecode_out;
		ErrorHandler& m_error_handler;
		std::vector<Unit*> m_unit_stack;
		std::vector<nullptr_t> m_func_stack;

		// UTILS
		void write(uint8_t byte);						// Writes one byte to the bytecode
		void write(uint8_t byte_a, uint8_t byte_b);		// Writes two bytes to the bytecode
		uint8_t add_constant(Value value);				// Adds a constant to the constant pool and returns its index
		void push_unit(Unit* unit) {					// New unit for functions, write outputs to outermost unit
			m_unit_stack.push_back(unit);
		}
		void pop_unit() {
			m_unit_stack.pop_back();
		}
		Unit* current_unit() {
			if (m_unit_stack.empty())
				return &m_bytecode_out;
			else
				return m_unit_stack[m_unit_stack.size() - 1];
		}

		bool m_set_member = false;

		bool m_in_constructor = false;
		bool m_in_class_decl = false;

		void make_name(const std::string& name);

		struct LocalName {
			std::string name;
			uint8_t index;
			int32_t scope_depth;
		};

		int32_t find_local(const std::string& name);
		int32_t m_current_scope = -1;
		std::vector<LocalName> m_locals;
		std::vector<std::string> m_globals;

		struct Loop {
			uint8_t condition_index;
			uint8_t end_index;
		};

		std::vector<Loop> m_loop_stack;

		// Expressions
		void expression(const std::shared_ptr<Expression>& expression);
		void binary_operation(const std::shared_ptr<BinaryOperation>& operation);
		void unary_operation(const std::shared_ptr<UnaryOperation>& operation);
		void number(const std::shared_ptr<Number>& number);
		void boolean(const std::shared_ptr<BoolValue>& boolean);
		void name(const std::shared_ptr<Name>& name);
		void call(const std::shared_ptr<Call>& call, bool invoke_method = false);
		void lvalue_start(const std::shared_ptr<LValueStartNode>& lvalue_start);
		void lvalue(const std::shared_ptr<LValue>& l_value);

		// Statements
		void statement(const std::shared_ptr<Statement>& statement);
		void log_statement(const std::shared_ptr<LogStatement>& log_statement);
		void expression_statement(const std::shared_ptr<ExpressionStatement>& expression_statement);
		void variable_declaration(const std::shared_ptr<VariableDeclaration>& variable_decl, bool make_member = false);
		void assignment(const std::shared_ptr<Assignment>& assignment);
		void if_statement(const std::shared_ptr<IfStatement>& statement);
		void while_statement(const std::shared_ptr<WhileStatement>& statement);
		void compount_statement(const std::shared_ptr<CompountStatement>& statement);
		void break_statement(const std::shared_ptr<BreakStatement>& break_stmt);
		void continue_statement(const std::shared_ptr<ContinueStatement>& continue_stmt);
		void function_declaration(const std::shared_ptr<FunctionDeclaration>& function_decl, bool make_member = false);
		void return_statement(const std::shared_ptr<ReturnStatement>& return_stmt);
		void class_declaration(const std::shared_ptr<ClassDeclaration>& function_decl);
		void enum_declaration(const std::shared_ptr<EnumDeclaration>& function_decl);
	};
}