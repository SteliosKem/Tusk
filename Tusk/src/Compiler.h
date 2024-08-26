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

		// UTILS
		void write(uint8_t byte);						// Writes one byte to the bytecode
		void write(uint8_t byte_a, uint8_t byte_b);		// Writes two bytes to the bytecode
		uint8_t add_constant(Value value);				// Adds a constant to the constant pool and returns its index

		struct LocalName {
			std::string name;
			uint8_t index;
			int32_t scope_depth;
		};

		int32_t find_local(const std::string& name);

		int32_t m_current_scope = -1;

		std::vector<LocalName> m_locals;

		std::vector<std::string> m_globals;
		//std::vector<std::vector<std::string>> m_locals;	// Stack behaviour for nested blocks

		// Expressions
		void expression(const std::shared_ptr<Expression>& expression);
		void binary_operation(const std::shared_ptr<BinaryOperation>& operation);
		void unary_operation(const std::shared_ptr<UnaryOperation>& operation);
		void number(const std::shared_ptr<Number>& number);
		void boolean(const std::shared_ptr<BoolValue>& boolean);
		void name(const std::shared_ptr<Name>& name);

		// Statements
		void statement(const std::shared_ptr<Statement>& statement);
		void log_statement(const std::shared_ptr<LogStatement>& log_statement);
		void expression_statement(const std::shared_ptr<ExpressionStatement>& expression_statement);
		void variable_declaration(const std::shared_ptr<VariableDeclaration>& variable_decl);
		void assignment(const std::shared_ptr<Assignment>& assignment);
		void if_statement(const std::shared_ptr<IfStatement>& statement);
		void while_statement(const std::shared_ptr<WhileStatement>& statement);
		void compount_statement(const std::shared_ptr<CompountStatement>& statement);
	};
}