#pragma once
#include <vector>
#include <string>

namespace Tusk {
	enum class ErrorType {
		WARNING,
		SYNTAX_ERROR,
		COMPILE_ERROR,
		RUNTIME_ERROR
	};

	struct Position {
		int32_t line_index{ 0 };
		int32_t start_index{ 0 };
		int32_t end_index{ 0 };
	};

	struct Error {
		std::string error_message;
		Position error_position;
		ErrorType error_type;
	};

	class ErrorHandler {
	public:
		ErrorHandler() = default;

		static std::string string_pretty_formatted(const Error& error);
		static std::string string_basic(const Error& error);
		static std::string string_basic_with_type(const Error& error);

		// Add error to list
		void report_error(const std::string& error_msg, const Position& position, ErrorType type);
		void clear() { m_errors.clear(); }

		bool has_errors() const { return !m_errors.empty(); }
		const std::vector<Error>& get_errors() const { return m_errors; }
	private:
		std::vector<Error> m_errors;
	};
}