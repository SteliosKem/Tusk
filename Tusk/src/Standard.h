#pragma once
#include <array>
#include <string>
#include <Value.h>
#include <iostream>

namespace Tusk::Standard {
	
	const std::array<std::string, 2> standard_functions = {"read", "List"};

	inline FunctionReturn read(int arg_count, Value* arguments) {
		std::string str;
		std::getline(std::cin, str);
		return FunctionReturn(FunctionResult::OK, Value(std::make_shared<StringObject>(str)));
	}

	inline FunctionReturn List(int arg_count, Value* arguments) {
		std::vector<Value> values;
		for (int i = 0; i < arg_count; i++) {
			values.push_back(arguments[i + 1]);
		}
		return FunctionReturn(FunctionResult::OK, Value(std::make_shared<ListValue>(values)));
	}

	namespace ListUtils {
		inline FunctionReturn append(const std::shared_ptr<ListValue>& list, uint8_t arg_count, Value* args) {
			for (int i = 0; i < arg_count; i++) {
				list->values.push_back(args[i]);
			}
			return FunctionReturn(FunctionResult::OK, Value(list));
		}
		inline FunctionReturn add(const std::shared_ptr<ListValue>& list, uint8_t arg_count, Value* args) {
			for (int i = 0; i < arg_count; i++) {
				if (args[i].is<std::shared_ptr<ValueObject>>() && args[i].get_object_type() == ObjectType::LIST)
					for (auto& i : args[i].get_object<ListValue>()->values)
						list->values.push_back(i);
				else
					list->values.push_back(args[i]);
			}
			return FunctionReturn(FunctionResult::OK, Value(list));
		}
		inline FunctionReturn pop(const std::shared_ptr<ListValue>& list, uint8_t arg_count, Value* args) {
			if (arg_count != 0)
				return  FunctionReturn(FunctionResult::ERROR, list->values[args[0].get<int64_t>()], "Method pop() expects 0 arguments");
			else {
				Value top = list->values[list->values.size() - 1];
				list->values.pop_back();
				return FunctionReturn(FunctionResult::OK, top);
			}
		}
		inline FunctionReturn size(const std::shared_ptr<ListValue>& list, uint8_t arg_count, Value* args) {
			if (arg_count != 0)
				return  FunctionReturn(FunctionResult::ERROR, Value(), "Method size() expects 0 arguments");
			else {
				return FunctionReturn(FunctionResult::OK, Value((int64_t)list->values.size()));
			}
		}
		inline FunctionReturn iterate() {}
		inline FunctionReturn get(const std::shared_ptr<ListValue>& list, uint8_t arg_count, Value* args) {
			if (arg_count == 1)
				if (args[0].get_type() != typeid(int64_t))
					return FunctionReturn(FunctionResult::ERROR, Value(), "Only integers allowed as index");
				else
					if (args[0].get<int64_t>() > list->values.size())
						return FunctionReturn(FunctionResult::ERROR, Value(), "Index out of bounds");
					else
						return  FunctionReturn(FunctionResult::OK, list->values[args[0].get<int64_t>()]);
			else if (arg_count == 2); // TODO: Return sublist
			else
				return FunctionReturn(FunctionResult::ERROR, Value(), "Method get() expects 1 argument");
		}
	}
}