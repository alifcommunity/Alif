#pragma once

#include "AlifCore_FreeList.h"


class AlifThreadImpl {
public:
	// semi-public fields are in AlifThread.
	AlifThread base{};

	LListNode memFreeQueue{};
};
