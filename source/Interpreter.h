#pragma once

#include "Compiler.h"
#include "MemoryBlock.h"

#include <stack>

class Interpreter {
public:

	Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data, MemoryBlock* _alifMemory);

	void run_code();
};