#pragma once

#include "Compiler.h"

class Interpreter {
	std::vector<InstructionsType>* instructions_;
	std::vector<AlifObject*>* data_;
	int stackLevel = 256;
	AlifObject* memory_[256]; // stack memory
public:
	Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data);

	void run_code();
};