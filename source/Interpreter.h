#pragma once

#include "Compiler.h"

class Interpreter {
	std::vector<InstructionsType>* instructions_;
	std::vector<AlifObject*>* data_;
	std::vector<AlifObject*> memory_; // stack memory
public:
	Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data);

	void run_code();
};