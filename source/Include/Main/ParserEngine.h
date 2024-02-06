#pragma once

#include "alif.h"
#include "AlifCore_Memory.h"
#include "AlifCore_AST.h"

Module* alifParser_astFromFile(FILE*, AlifObj*, int, AlifMemory*);
