#pragma once

#include "Compiler.h"
#include "AlifMemory.h"
#include "AlifStack.h"

class Interpreter {
public:

	Interpreter(AlifArray<Container*>* _containers, AlifMemory* _alifMemory);

	void run_code();
};