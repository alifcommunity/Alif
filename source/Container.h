#pragma once

#include "Types.h"
#include "AlifObject.h"
#include "AlifArray.h"

class Container
{
public:
	AlifArray<InstructionsType>* instructions_{};
	AlifArray<AlifObject*>* data_{};
};