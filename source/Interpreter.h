#pragma once

#include "Compiler.h"
#include "AlifMemory.h"
#include "AlifStack.h"
#include "AddrFuncs.h"
#include "AlifNamesTable.h"

class Interpreter {
public:

	Interpreter(AlifArray<Container*>* _containers, AlifMemory* _alifMemory, AlifNamesTable* _namesTable);

	void run_code();
};