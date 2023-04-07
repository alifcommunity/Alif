#pragma once

#include "Types.h"
#include "Object.h"

#include <vector>

class Container
{
public:
	std::vector<InstructionsType>* instructions_{};
	std::vector<AlifObject*>* data_{};
};