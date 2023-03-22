#pragma once

#include <stack>

#include "Compiler.h"

class Interpreter {
public:

	Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data);

	void run_code();
};