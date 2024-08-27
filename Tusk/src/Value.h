#pragma once
#include <variant>
#include <memory>
#include <iostream>

namespace Tusk {
	struct Unit;
	enum class ObjectType {
		OBJECT,
		STRING,
		FUNCTION,
		CLASS,
		ENUM,
		VOID
	};

	struct ValueObject {
		virtual ObjectType get_type() const { return ObjectType::OBJECT; }
	};

	struct VoidValue : public ValueObject {
		ObjectType get_type() const override { return ObjectType::VOID; }
	};

	struct String : public ValueObject {
		std::string string;

		String(const std::string& str = "") : string{ str } {}
		ObjectType get_type() const override { return ObjectType::STRING; }
	};

	struct Function : public ValueObject {
		std::string function_name{ "" };
		uint32_t arg_count{ 0 };
		std::shared_ptr<Unit> code_unit;

		Function(const std::string& str = "", uint32_t arg_count = 0) : function_name{ str }, arg_count{ arg_count } {}
		ObjectType get_type() const override { return ObjectType::FUNCTION; }
	};

	class Value {
	public:
		Value(int64_t integer) : m_value{ integer } {}
		Value(double real) : m_value{ real } {}
		Value(bool boolean) : m_value{ boolean } {}
		Value(const std::shared_ptr<ValueObject>& object) : m_value{ object } {}
		Value(const std::string& str) : m_value{ std::make_shared<String>(str) } {}
		Value(nullptr_t) : m_value{ std::make_shared<VoidValue>() } {}
		Value() = default;

		template<typename T>
		bool is() const {
			return std::holds_alternative<T>(m_value);
		}

		template<typename T>
		T get() const {
			return std::get<T>(m_value);
		}

		template<typename T>
		std::shared_ptr<T> get_object() const {
			return std::static_pointer_cast<T>(std::get<std::shared_ptr<ValueObject>>(m_value));
		}

		ObjectType get_object_type() const {
			return std::get<std::shared_ptr<ValueObject>>(m_value)->get_type();
		}

		template<typename T>
		T get_if_is() const {
			if (is<T>())
				return std::get<T>(m_value);
			else
				return nullptr;
		}

		std::type_info const& get_type() {
			return std::visit([](auto&& x)->decltype(auto) { return typeid(x); }, m_value);
		}

		void set(const std::variant<int64_t, double, bool, std::shared_ptr<ValueObject>>& val) { m_value = val; }

		friend std::ostream& operator<<(std::ostream& os, const Value& value) {
			if (value.is<int64_t>())
				os << value.get<int64_t>();
			else if (value.is<double>())
				os << value.get<double>();
			else if (value.is<bool>())
				os << (value.get<bool>() ? "true" : "false");
			else if (value.is<std::shared_ptr<ValueObject>>()) {
				switch (value.get_object_type()) {
				case ObjectType::STRING:
					os << '"' << value.get_object<String>()->string << '"';
					break;
				case ObjectType::VOID:
					os << "void";
					break;
				case ObjectType::FUNCTION:
					os << "<function " + value.get_object<Function>()->function_name + ">";
					break;
				}
			}
			return os;
		}

		operator std::variant<int64_t, double, bool, std::shared_ptr<ValueObject>>() const { return m_value; }
		const std::variant<int64_t, double, bool, std::shared_ptr<ValueObject>>& get_variant() const { return m_value; }
	private:
		std::variant<int64_t, double, bool, std::shared_ptr<ValueObject>> m_value;
	};
}