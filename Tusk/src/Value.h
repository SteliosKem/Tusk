#pragma once
#include <variant>
#include <memory>
#include <iostream>
#include <unordered_map>

namespace Tusk {
	struct Unit;
	enum class ObjectType {
		OBJECT,
		STRING,
		FUNCTION,
		CLASS,
		INSTANCE,
		ENUM,
		VOID
	};

	struct ValueObject {
		virtual ObjectType get_type() const { return ObjectType::OBJECT; }
	};

	struct VoidObject : public ValueObject {
		ObjectType get_type() const override { return ObjectType::VOID; }
	};

	struct StringObject : public ValueObject {
		std::string string;

		StringObject(const std::string& str = "") : string{ str } {}
		ObjectType get_type() const override { return ObjectType::STRING; }
	};

	struct FunctionObject : public ValueObject {
		std::string function_name{ "" };
		uint32_t arg_count{ 0 };
		std::shared_ptr<Unit> code_unit;

		FunctionObject(const std::string& str = "", uint32_t arg_count = 0) : function_name{ str }, arg_count{ arg_count } {}
		ObjectType get_type() const override { return ObjectType::FUNCTION; }
	};

	class Value;
	struct ClassObject : public ValueObject {
		std::string class_name{ "" };
		std::unordered_map<std::string, Value> public_members;
		std::unordered_map<std::string, Value> private_members;

		ClassObject(const std::string& str = "") : class_name{ str } {}
		ObjectType get_type() const override { return ObjectType::CLASS; }
	};

	struct InstanceObject : public ValueObject {
		ClassObject& class_ref;
		std::unordered_map<std::string, Value> public_members;
		std::unordered_map<std::string, Value> private_members;

		InstanceObject(ClassObject& class_ref) : class_ref{ class_ref } { }
		ObjectType get_type() const override { return ObjectType::INSTANCE; }
	};

	class Value {
	public:
		Value(int64_t integer) : m_value{ integer } {}
		Value(double real) : m_value{ real } {}
		Value(bool boolean) : m_value{ boolean } {}
		Value(const std::shared_ptr<ValueObject>& object) : m_value{ object } {}
		Value(const std::string& str) : m_value{ std::make_shared<StringObject>(str) } {}
		Value(nullptr_t) : m_value{ std::make_shared<VoidObject>() } {}
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
					os << '"' << value.get_object<StringObject>()->string << '"';
					break;
				case ObjectType::VOID:
					os << "void";
					break;
				case ObjectType::FUNCTION:
					os << "<function " + value.get_object<FunctionObject>()->function_name + ">";
					break;
				case ObjectType::CLASS:
					os << "<class " + value.get_object<ClassObject>()->class_name + ">";
					break;
				case ObjectType::INSTANCE:
					os << "<" + value.get_object<InstanceObject>()->class_ref.class_name + " instance>";
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