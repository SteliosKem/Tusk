#include "Compiler.h"

namespace Tusk {
	void Compiler::write(uint8_t byte) {
		m_bytecode_out.write_byte(byte);
	}

	void Compiler::write(uint8_t byte_a, uint8_t byte_b) {
		m_bytecode_out.write_byte(byte_a);
		m_bytecode_out.write_byte(byte_b);
	}

	uint8_t Compiler::add_constant(Value value) {
		return m_bytecode_out.write_value(value);
	}

	void Compiler::expression(const std::shared_ptr<Expression>& expression) {
		switch (expression->get_type()) {
		case NodeType::BINARY_OPERATION:
			binary_operation(std::static_pointer_cast<BinaryOperation>(expression));
			break;
		case NodeType::UNARY_OPERATION:
			unary_operation(std::static_pointer_cast<UnaryOperation>(expression));
			break;
		case NodeType::NUMBER_VALUE:
			number(std::static_pointer_cast<Number>(expression));
			break;
		case NodeType::BOOL_VALUE:
			boolean(std::static_pointer_cast<BoolValue>(expression));
			break;
		case NodeType::NAME:
			name(std::static_pointer_cast<Name>(expression));
			break;
		case NodeType::VOID:
			write((uint8_t)Instruction::VOID);
			break;
		case NodeType::STRING:
			write((uint8_t)Instruction::VAL_INDEX, add_constant(std::static_pointer_cast<StringLiteral>(expression)->value));
			break;
		}
	}

	const Unit& Compiler::compile() {
		for (std::shared_ptr<Statement> stmt : m_ast->statements) {
			statement(stmt);
		}
		write((uint8_t)Instruction::RETURN);
		return m_bytecode_out;
	}

	void Compiler::number(const std::shared_ptr<Number>& number) {
		write((uint8_t)Instruction::VAL_INDEX, add_constant(number->value));
	}

	void Compiler::boolean(const std::shared_ptr<BoolValue>& boolean) {
		write((uint8_t)Instruction::VAL_INDEX, add_constant(boolean->value));
	}

	int32_t Compiler::find_local(const std::string& name) {
		for (int32_t i = 0; i < m_locals.size(); i++)
			if (m_locals[i].name == name)
				return i;
		return -1;
	}

	void Compiler::name(const std::shared_ptr<Name>& name) {
		int64_t local_idx = -1;
		if(std::find(m_globals.begin(), m_globals.end(), name->string) != m_globals.end())
			write((uint8_t)Instruction::GET_GLOBAL, add_constant(Value(std::make_shared<String>(name->string))));
		else if ((local_idx = find_local(name->string)) != -1)
			write((uint8_t)Instruction::GET_LOCAL, add_constant(Value(local_idx)));
		else {
			m_error_handler.report_error("Name '" + name->string + "' does not exist in this scope", {}, ErrorType::COMPILE_ERROR);
		}
	}

	void Compiler::binary_operation(const std::shared_ptr<BinaryOperation>& operation) {
		expression(operation->left_expression);
		expression(operation->right_expression);
		switch (operation->operator_token.type)
		{
		case TokenType::PLUS:
			write((uint8_t)Instruction::ADD);
			break;
		case TokenType::MINUS:
			write((uint8_t)Instruction::SUBTRACT);
			break;
		case TokenType::STAR:
			write((uint8_t)Instruction::MULTIPLY);
			break;
		case TokenType::SLASH:
			write((uint8_t)Instruction::DIVIDE);
			break;
		case TokenType::EQUAL_EQUAL:
			write((uint8_t)Instruction::EQUAL);
			break;
		case TokenType::BANG_EQUAL:
			write((uint8_t)Instruction::NOT_EQUAL);
			break;
		case TokenType::GREATER:
			write((uint8_t)Instruction::GREATER);
			break;
		case TokenType::LESS:
			write((uint8_t)Instruction::LESS);
			break;
		case TokenType::GREATER_EQUAL:
			write((uint8_t)Instruction::GREATER_EQUAL);
			break;
		case TokenType::LESS_EQUAL:
			write((uint8_t)Instruction::LESS_EQUAL);
			break;
		default:
			break;
		}
		
	}

	void Compiler::unary_operation(const std::shared_ptr<UnaryOperation>& operation) {
		expression(operation->right_expression);
		switch (operation->operator_token.type)
		{
		case TokenType::MINUS:
			write((uint8_t)Instruction::NEGATE);
			break;
		case TokenType::BANG:
			write((uint8_t)Instruction::NOT);
			break;
		default:
			break;
		}

	}

	void Compiler::statement(const std::shared_ptr<Statement>& statement) {
		switch (statement->get_type()) {
		case NodeType::LOG_STATEMENT:
			log_statement(std::static_pointer_cast<LogStatement>(statement));
			break;
		case NodeType::EXPRESSION_STATEMENT:
			expression_statement(std::static_pointer_cast<ExpressionStatement>(statement));
			break;
		case NodeType::VARIABLE_DECLARATION:
			variable_declaration(std::static_pointer_cast<VariableDeclaration>(statement));
			break;
		case NodeType::ASSIGNMENT:
			assignment(std::static_pointer_cast<Assignment>(statement));
			break;
		case NodeType::IF_STATEMENT:
			if_statement(std::static_pointer_cast<IfStatement>(statement));
			break;
		case NodeType::WHILE_STATEMENT:
			while_statement(std::static_pointer_cast<WhileStatement>(statement));
			break;
		case NodeType::COMPOUNT_STATEMENT:
			compount_statement(std::static_pointer_cast<CompountStatement>(statement));
			break;
		case NodeType::BREAK_STATEMENT:
			break_statement(std::static_pointer_cast<BreakStatement>(statement));
			break;
		case NodeType::CONTINUE_STATEMENT:
			continue_statement(std::static_pointer_cast<ContinueStatement>(statement));
			break;
		case NodeType::VOID_STATEMENT:
			break;
		}
	}
	void Compiler::log_statement(const std::shared_ptr<LogStatement>& log_statement) {
		expression(log_statement->output);
		write((uint8_t)Instruction::LOG, (uint8_t)Instruction::POP);
	}
	void Compiler::expression_statement(const std::shared_ptr<ExpressionStatement>& expression_statement) {
		expression(expression_statement->expression);
		write((uint8_t)Instruction::POP);
	}

	void Compiler::variable_declaration(const std::shared_ptr<VariableDeclaration>& variable_decl) {
		if (m_current_scope == -1) {
			if (std::find(m_globals.begin(), m_globals.end(), variable_decl->variable_name) != m_globals.end()) {
				m_error_handler.report_error("Global name '" + variable_decl->variable_name + "' already exists", {}, ErrorType::COMPILE_ERROR);
			}


			if (variable_decl->value)
				expression(variable_decl->value);
			else
				write((uint8_t)Instruction::VOID);
			write((uint8_t)Instruction::MAKE_GLOBAL, add_constant(Value(std::make_shared<String>(variable_decl->variable_name))));
			m_globals.push_back(variable_decl->variable_name);
		}
		else {
			if (variable_decl->value)
				expression(variable_decl->value);
			else
				write((uint8_t)Instruction::VOID);
			m_locals.push_back(LocalName{variable_decl->variable_name, (uint8_t)(m_locals.size() - 1), m_current_scope});
			write((uint8_t)Instruction::SET_LOCAL, add_constant((int64_t)(m_locals.size() - 1)));
		}
	}

	void Compiler::assignment(const std::shared_ptr<Assignment>& assignment) {
		expression(assignment->expression);
		int64_t local_idx = -1;
		if (std::find(m_globals.begin(), m_globals.end(), assignment->name) != m_globals.end())
			write((uint8_t)Instruction::SET_GLOBAL, add_constant(Value(std::make_shared<String>(assignment->name))));
		else if ((local_idx = find_local(assignment->name)) != -1)
			write((uint8_t)Instruction::SET_LOCAL, add_constant(Value(local_idx)));
		else
			m_error_handler.report_error("Name '" + assignment->name + "' does not exist in this scope", {}, ErrorType::COMPILE_ERROR);
	}

	void Compiler::if_statement(const std::shared_ptr<IfStatement>& stmt) {
		expression(stmt->condition);
		uint8_t false_index = add_constant(m_bytecode_out.index());
		write((uint8_t)Instruction::JUMP_IF_FALSE, false_index);
		statement(stmt->body);
		uint8_t end_index{ 0 };
		if (stmt->else_body) {
			end_index = add_constant(m_bytecode_out.index());
			write((uint8_t)Instruction::JUMP, end_index);
		}
		
		m_bytecode_out.get_values()[false_index] = m_bytecode_out.index();
		if (stmt->else_body) {
			
			statement(stmt->else_body);
			m_bytecode_out.get_values()[end_index] = m_bytecode_out.index();
		}
	}

	void Compiler::while_statement(const std::shared_ptr<WhileStatement>& stmt) {
		uint8_t top_of_loop = add_constant(m_bytecode_out.index());
		expression(stmt->condition);
		uint8_t false_index = add_constant(m_bytecode_out.index());
		m_loop_stack.push_back({top_of_loop, false_index});
		write((uint8_t)Instruction::JUMP_IF_FALSE, false_index);
		statement(stmt->body);
		write((uint8_t)Instruction::JUMP, top_of_loop);
		m_bytecode_out.get_values()[false_index] = m_bytecode_out.index();
		
	}

	void Compiler::compount_statement(const std::shared_ptr<CompountStatement>& compount) {
		m_current_scope++;
		for (const auto& stmt : compount->statements) {
			statement(stmt);
		}
		m_current_scope--;
		for (int64_t i = m_locals.size() - 1; m_locals.size() > 0; m_locals.pop_back(), i--) {
			if (m_locals[i].scope_depth <= m_current_scope)
				break;
			write((uint8_t)Instruction::POP);
		}
		
	}

	void Compiler::break_statement(const std::shared_ptr<BreakStatement>& break_stmt) {
		if (m_loop_stack.empty()) {
			m_error_handler.report_error("Cannot use 'break' outside loops", {}, ErrorType::COMPILE_ERROR);
			return;
		}
		write((uint8_t)Instruction::JUMP, m_loop_stack[m_loop_stack.size() - 1].end_index);
	}
	void Compiler::continue_statement(const std::shared_ptr<ContinueStatement>& continue_stmt) {
		if (m_loop_stack.empty()) {
			m_error_handler.report_error("Cannot use 'continue' outside loops", {}, ErrorType::COMPILE_ERROR);
			return;
		}
		write((uint8_t)Instruction::JUMP, m_loop_stack[m_loop_stack.size() - 1].condition_index);
	}
}