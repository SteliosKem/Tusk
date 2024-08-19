#pragma once
#include <vector>

namespace Tusk {
	enum class Instruction {
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		RETURN
	};

	struct Unit {
		std::vector<uint8_t> bytes;
	};
}