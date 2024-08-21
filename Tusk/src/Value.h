#pragma once
#include <variant>
#include <memory>
#include <iostream>

namespace Tusk {
	struct ValueObject {

	};

	class Value {
	public:
		Value(int64_t integer) : m_value{ integer } {}
		Value(double real) : m_value{ real } {}
		Value(bool boolean) : m_value{ boolean } {}
		Value(const std::shared_ptr<ValueObject>& object) : m_value{ object } {}

		template<typename T>
		bool is() const {
			return std::holds_alternative<T>(m_value);
		}

		template<typename T>
		T get() const {
			return std::get<T>(m_value);
		}

		template<typename T>
		T get_if_is() const {
			if (is<T>())
				return std::get<T>(m_value);
			else
				return nullptr;
		}

		void set(const std::variant<int64_t, double, bool, std::shared_ptr<ValueObject>>& val) { m_value = val; }

		friend std::ostream& operator<<(std::ostream& os, const Value& value) {
			if (value.is<int64_t>())
				os << value.get<int64_t>();
			else if (value.is<double>())
				os << value.get<double>();
			else if (value.is<bool>())
				os << (value.get<bool>() ? "true" : "false");
			return os;
		}
	private:
		std::variant<int64_t, double, bool, std::shared_ptr<ValueObject>> m_value;
	};
}