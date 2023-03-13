#pragma once

#include "Compiler.h"

static std::vector<InstructionsType>* instructions_;
static std::vector<AlifObject*>* data_;
static int stackLevel = 256;
static AlifObject* memory_[256]; // stack memory

class Interpreter {
public:

	Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data);

	void run_code();
};