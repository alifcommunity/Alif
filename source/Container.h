#pragma once

#include "Types.h"
#include "Object.h"
#include "AlifArray.h"

class Container
{
public:
	AlifArray<InstructionsType>* instructions_{};
	AlifArray<AlifObject*>* data_{};
};