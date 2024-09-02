#pragma once

#include "AlifCore_Lock.h"
#include "AlifCore_GIL.h"

























class AlifEval { // 119
public:
	uintptr_t instrumentationVersion{};
	AlifIntT recursionLimit{};
	GILDureRunState* gil_{};
	AlifIntT ownGIL{};
};
