#include "pch.h"
#include "Error.h"

namespace Tusk {
	void ErrorHandler::report_error(const std::string& error_msg, const Position& position, ErrorType type) {
		m_errors.push_back({ error_msg, position, type });
	}

	std::string ErrorHandler::string_pretty_formatted(const Error& error) {
		return "";

	}
	std::string ErrorHandler::string_basic(const Error& error) {
		return std::format("{0} at line {1}", error.error_message, error.error_position.line_index);
	}
	std::string ErrorHandler::string_basic_with_type(const Error& error) {
		std::string str{};
		switch (error.error_type) {
		case ErrorType::WARNING:
			str += "WARNING: ";
			break;
		case ErrorType::SYNTAX_ERROR:
			str += "SYNTAX ERROR: ";
			break;
		case ErrorType::COMPILE_ERROR:
			str += "COMPILE ERROR: ";
			break;
		case ErrorType::RUNTIME_ERROR:
			str += "RUNTIME ERROR: ";
			break;
		}

		str += std::format("{0} at line {1}", error.error_message, error.error_position.line_index);
		return str;
	}
}