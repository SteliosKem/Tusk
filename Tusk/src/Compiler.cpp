#include "pch.h"
#include "Compiler.h"
#include "Standard.h"

namespace Tusk {
	void Compiler::write(uint8_t byte) {
		if (m_unit_stack.empty())
			m_bytecode_out.write_byte(byte);
		else
			m_unit_stack[m_unit_stack.size() - 1]->write_byte(byte);
	}

	void Compiler::write(uint8_t byte_a, uint8_t byte_b) {
		if (m_unit_stack.empty()) {
			m_bytecode_out.write_byte(byte_a);
			m_bytecode_out.write_byte(byte_b);
		}
		else {
			m_unit_stack[m_unit_stack.size() - 1]->write_byte(byte_a);
			m_unit_stack[m_unit_stack.size() - 1]->write_byte(byte_b);
		}
		
	}

	uint8_t Compiler::add_constant(Value value) {
		if (m_unit_stack.empty())
			return m_bytecode_out.write_value(value);
		else
			return m_unit_stack[m_unit_stack.size() - 1]->write_value(value);
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
		case NodeType::LVALUE:
			lvalue(std::static_pointer_cast<LValue>(expression));
			break;
		case NodeType::LVALUE_START:
			lvalue_start(std::static_pointer_cast<LValueStartNode>(expression));
			break;
		case NodeType::CALL:
			call(std::static_pointer_cast<Call>(expression));
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
		if(std::find(m_globals.begin(), m_globals.end(), name->string) != m_globals.end() 
			|| std::find(Standard::standard_functions.begin(), Standard::standard_functions.end(), name->string) != Standard::standard_functions.end())
			write((uint8_t)Instruction::GET_GLOBAL, add_constant(Value(std::make_shared<StringObject>(name->string))));
		else if ((local_idx = find_local(name->string)) != -1)
			write((uint8_t)Instruction::GET_LOCAL, add_constant(Value(local_idx)));
		else {
			m_error_handler.report_error("Name '" + name->string + "' does not exist in this scope", {}, ErrorType::COMPILE_ERROR);
		}
	}

	void Compiler::lvalue(const std::shared_ptr<LValue>& l_value) {
		if (l_value->name->get_type() == NodeType::NAME)
			write(add_constant(Value(std::static_pointer_cast<Name>(l_value->name)->string)));
		else {
			/*std::shared_ptr<Call> call = std::static_pointer_cast<Call>(l_value->name);
			write(add_constant(Value(call->name->string)));
			for (const auto& param : call->parameters)
				expression(param);
			write((uint8_t)Instruction::CALL, (uint8_t)call->parameters.size());*/
			call(std::static_pointer_cast<Call>(l_value->name), true);
		}
		if (l_value->access) {
			if (m_set_member && !l_value->access->access)
				write((uint8_t)Instruction::SET_MEMBER);
			else
				if (l_value->access->name->get_type() != NodeType::CALL)
					write((uint8_t)Instruction::GET_MEMBER);
			lvalue(l_value->access);
		}
	}
	void Compiler::lvalue_start(const std::shared_ptr<LValueStartNode>& l_value) {
		std::shared_ptr<LValue> lval = l_value->lvalue;
		if (!m_in_class_decl) {
			switch (lval->name->get_type()) {
			case NodeType::NAME:
				if (std::static_pointer_cast<Name>(lval->name)->string == "this") {
					m_error_handler.report_error("Cannot use this outside of a class", {}, ErrorType::COMPILE_ERROR);
					return;
				}
				break;
			case NodeType::CALL:
				if (std::static_pointer_cast<Call>(lval->name)->name->string == "this") {
					m_error_handler.report_error("Cannot use this outside of a class", {}, ErrorType::COMPILE_ERROR);
					return;
				}
			}
		}
		if (lval->name->get_type() == NodeType::NAME) {
			name(std::static_pointer_cast<Name>(lval->name));
		}
		else
			call(std::static_pointer_cast<Call>(lval->name));

		if (lval->access) {
			if (m_set_member && !lval->access->access)
				write((uint8_t)Instruction::SET_MEMBER);
			else
				if(lval->access->name->get_type() != NodeType::CALL)
					write((uint8_t)Instruction::GET_MEMBER);
			lvalue(lval->access);
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
		case NodeType::COMPOUND_ASSIGNMENT:
			compound_assignment(std::static_pointer_cast<CompoundAssignment>(statement));
			break;
		case NodeType::IF_STATEMENT:
			if_statement(std::static_pointer_cast<IfStatement>(statement));
			break;
		case NodeType::WHILE_STATEMENT:
			while_statement(std::static_pointer_cast<WhileStatement>(statement));
			break;
		case NodeType::COMPOUND_STATEMENT:
			compound_statement(std::static_pointer_cast<CompoundStatement>(statement));
			break;
		case NodeType::BREAK_STATEMENT:
			break_statement(std::static_pointer_cast<BreakStatement>(statement));
			break;
		case NodeType::CONTINUE_STATEMENT:
			continue_statement(std::static_pointer_cast<ContinueStatement>(statement));
			break;
		case NodeType::FUNCTION_DECLARATION:
			function_declaration(std::static_pointer_cast<FunctionDeclaration>(statement));
			break;
		case NodeType::RETURN_STATEMENT:
			return_statement(std::static_pointer_cast<ReturnStatement>(statement));
			break;
		case NodeType::CLASS_DECLARATION:
			class_declaration(std::static_pointer_cast<ClassDeclaration>(statement));
			break;
		case NodeType::ENUM_DECLARATION:
			enum_declaration(std::static_pointer_cast<EnumDeclaration>(statement));
			break;
		case NodeType::VOID_STATEMENT:
			break;
		}
	}
	void Compiler::log_statement(const std::shared_ptr<LogStatement>& log_statement) {
		expression(log_statement->output);
		if(log_statement->log_line)
			write((uint8_t)Instruction::LOGL, (uint8_t)Instruction::POP);
		else
			write((uint8_t)Instruction::LOG, (uint8_t)Instruction::POP);
	}

	void Compiler::expression_statement(const std::shared_ptr<ExpressionStatement>& expression_statement) {
		expression(expression_statement->expression);
		write((uint8_t)Instruction::POP);
	}

	void Compiler::variable_declaration(const std::shared_ptr<VariableDeclaration>& variable_decl, bool make_member) {
		if (variable_decl->value)
			expression(variable_decl->value);
		else
			write((uint8_t)Instruction::VOID);
		if (make_member)
			write((uint8_t)Instruction::MAKE_MEMBER, add_constant(Value(variable_decl->variable_name)));
		else
			make_name(variable_decl->variable_name);
	}

	void Compiler::make_name(const std::string& name) {
		if (m_current_scope == -1) {
			if (std::find(m_globals.begin(), m_globals.end(), name) != m_globals.end()) {
				m_error_handler.report_error("Global name '" + name + "' already exists", {}, ErrorType::COMPILE_ERROR);
			}

			write((uint8_t)Instruction::MAKE_GLOBAL, add_constant(Value(std::make_shared<StringObject>(name))));
			m_globals.push_back(name);
		}
		else {
			m_locals.push_back(LocalName{ name, (uint8_t)(m_locals.size() - 1), m_current_scope });
			//write((uint8_t)Instruction::SET_LOCAL, add_constant((int64_t)(m_locals.size() - 1)));
		}
	}

	void Compiler::assignment(const std::shared_ptr<Assignment>& assignment) {
		std::shared_ptr<LValue> lval = assignment->lvalue->lvalue;
		expression(assignment->expression);
		int64_t local_idx = -1;
		if (!lval->access && lval->name->get_type() == NodeType::NAME) {
			std::shared_ptr<Name> name = std::static_pointer_cast<Name>(lval->name);
			if (std::find(m_globals.begin(), m_globals.end(), name->string) != m_globals.end())
				write((uint8_t)Instruction::SET_GLOBAL, add_constant(Value(std::make_shared<StringObject>(name->string))));
			else if ((local_idx = find_local(name->string)) != -1)
				write((uint8_t)Instruction::SET_LOCAL, add_constant(Value(local_idx)));
			else
				m_error_handler.report_error("Name '" + name->string + "' does not exist in this scope", {}, ErrorType::COMPILE_ERROR);
		}
		else {
			m_set_member = true;
			lvalue_start(assignment->lvalue);
			m_set_member = false;
		}
	}

	void Compiler::compound_assignment(const std::shared_ptr<CompoundAssignment>& compound) {
		std::shared_ptr<LValue> lval = compound->lvalue->lvalue;
		bool is_global = false;
		bool is_member = false;
		int64_t local_idx = -1;
		if (!lval->access && lval->name->get_type() == NodeType::NAME) {
			std::shared_ptr<Name> name = std::static_pointer_cast<Name>(lval->name);
			if (std::find(m_globals.begin(), m_globals.end(), name->string) != m_globals.end()) {
				write((uint8_t)Instruction::GET_GLOBAL, add_constant(Value(std::make_shared<StringObject>(name->string))));
				is_global = true;
			}
			else if ((local_idx = find_local(name->string)) != -1)
				write((uint8_t)Instruction::GET_LOCAL, add_constant(Value(local_idx)));
			else
				m_error_handler.report_error("Name '" + name->string + "' does not exist in this scope", {}, ErrorType::COMPILE_ERROR);
		}
		else {
			lvalue_start(compound->lvalue);
			is_member = true;
		}
		expression(compound->expression);
		switch (compound->action) {
		case CompoundAssignment::ADD:
			write((uint8_t)Instruction::ADD);
			break;
		case CompoundAssignment::MULTIPLY:
			write((uint8_t)Instruction::MULTIPLY);
			break;
		case CompoundAssignment::SUBTRACT:
			write((uint8_t)Instruction::SUBTRACT);
			break;
		case CompoundAssignment::DIVIDE:
			write((uint8_t)Instruction::DIVIDE);
			break;
		}
		if (is_member) {
			m_set_member = true;
			lvalue_start(compound->lvalue);
			m_set_member = false;
		}
		else if (is_global) {
			std::shared_ptr<Name> name = std::static_pointer_cast<Name>(lval->name);
			write((uint8_t)Instruction::SET_GLOBAL, add_constant(Value(std::make_shared<StringObject>(name->string))));
		}
		else
			write((uint8_t)Instruction::SET_LOCAL, add_constant(Value(local_idx)));
	}

	void Compiler::if_statement(const std::shared_ptr<IfStatement>& stmt) {
		expression(stmt->condition);
		uint8_t false_index = add_constant(current_unit()->index());
		write((uint8_t)Instruction::JUMP_IF_FALSE, false_index);
		statement(stmt->body);
		uint8_t end_index{ 0 };
		if (stmt->else_body) {
			end_index = add_constant(current_unit()->index());
			write((uint8_t)Instruction::JUMP, end_index);
		}
		
		current_unit()->get_values()[false_index] = current_unit()->index();
		if (stmt->else_body) {
			
			statement(stmt->else_body);
			current_unit()->get_values()[end_index] = current_unit()->index();
		}
	}

	void Compiler::while_statement(const std::shared_ptr<WhileStatement>& stmt) {
		uint8_t top_of_loop = add_constant(current_unit()->index());
		expression(stmt->condition);
		uint8_t false_index = add_constant(current_unit()->index());
		m_loop_stack.push_back({top_of_loop, false_index});
		write((uint8_t)Instruction::JUMP_IF_FALSE, false_index);
		statement(stmt->body);
		write((uint8_t)Instruction::JUMP, top_of_loop);
		current_unit()->get_values()[false_index] = current_unit()->index();
		
	}

	void Compiler::compound_statement(const std::shared_ptr<CompoundStatement>& compound) {
		m_current_scope++;
		for (const auto& stmt : compound->statements) {
			statement(stmt);
		}
		m_current_scope--;
		for (int64_t i = m_locals.size() - 1; m_locals.size() > 0; m_locals.pop_back(), i--) {
			if (m_locals[i].scope_depth <= m_current_scope)
				break;
			if(m_locals[i].name != "this")
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

	void Compiler::function_declaration(const std::shared_ptr<FunctionDeclaration>& function_decl, bool make_member) {
		if (make_member && function_decl->function_name == "make")
			m_in_constructor = true;
		std::shared_ptr<FunctionObject> func = std::make_shared<FunctionObject>();
		func->function_name = function_decl->function_name;
		func->code_unit = std::make_shared<Unit>();
		m_func_stack.push_back(nullptr);
		push_unit(func->code_unit.get());
		m_current_scope++;
		m_locals.push_back(LocalName{ "this", (uint8_t)(m_locals.size() - 1), m_current_scope});
		for (const auto& arg : function_decl->arguments) {
			m_locals.push_back(LocalName{ arg->name, (uint8_t)(m_locals.size() - 1), m_current_scope });
		}
		
		m_current_scope--;
		statement(function_decl->body);
		write((uint8_t)Instruction::VOID, (uint8_t)Instruction::RETURN);
		pop_unit();
		m_func_stack.pop_back();
		func->arg_count = function_decl->arguments.size();
		write((uint8_t)Instruction::VAL_INDEX, add_constant(Value(func)));
		if (make_member)
			write((uint8_t)Instruction::MAKE_MEMBER, add_constant(Value(func->function_name)));
		else
			make_name(function_decl->function_name);

		if (make_member && function_decl->function_name == "make")
			m_in_constructor = false;
	}

	void Compiler::return_statement(const std::shared_ptr<ReturnStatement>& return_stmt) {
		if (m_func_stack.empty()) {
			m_error_handler.report_error("Cannot use return outside a function", {}, ErrorType::COMPILE_ERROR);
			return;
		}
		if (return_stmt->expr) {
			if (m_in_constructor)
				m_error_handler.report_error("Cannot return a value from a constructor", {}, ErrorType::COMPILE_ERROR);
			expression(return_stmt->expr);
		}
		else
			write((uint8_t)Instruction::VOID);
		write((uint8_t)Instruction::RETURN);
	}

	void Compiler::call(const std::shared_ptr<Call>& call, bool invoke_method) {
		if (!invoke_method) {
			name(call->name);
			for (const auto& param : call->parameters)
				expression(param);

			write((uint8_t)Instruction::CALL, (uint8_t)call->parameters.size());
		}
		else {
			for (const auto& param : call->parameters)
				expression(param);
			write((uint8_t)Instruction::METHOD_CALL, (uint8_t)call->parameters.size());
			write((uint8_t)add_constant(Value(std::make_shared<StringObject>(call->name->string))));
		}
	}

	void Compiler::class_declaration(const std::shared_ptr<ClassDeclaration>& class_decl) {
		std::shared_ptr<ClassObject> class_obj = std::make_shared<ClassObject>();
		class_obj->class_name = class_decl->class_name;

		m_in_class_decl = true;

		write((uint8_t)Instruction::VAL_INDEX, add_constant(Value(class_obj)));
		if (class_decl->parent_class != "") {
			name(std::make_shared<Name>(class_decl->parent_class));
			write((uint8_t)Instruction::INHERIT);
		}
		for (const auto& stmt : class_decl->body->statements) {
			switch (stmt->get_type()) {
			case NodeType::FUNCTION_DECLARATION:
				function_declaration(std::static_pointer_cast<FunctionDeclaration>(stmt), true);
				break;
			case NodeType::VARIABLE_DECLARATION:
				variable_declaration(std::static_pointer_cast<VariableDeclaration>(stmt), true);
				break;
			}
		}
		//name();
		
		make_name(class_decl->class_name);
		

		m_in_class_decl = false;
	}
	void Compiler::enum_declaration(const std::shared_ptr<EnumDeclaration>& enum_decl) {
		std::shared_ptr<EnumObject> enum_obj = std::make_shared<EnumObject>();
		enum_obj->name = enum_decl->enum_name;

		write((uint8_t)Instruction::VAL_INDEX, add_constant(Value(enum_obj)));
		for (const auto& val : enum_decl->values) {
			enum_obj->values.push_back(val);
		}

		make_name(enum_obj->name);
	}
}