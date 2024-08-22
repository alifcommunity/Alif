#pragma once

//#include "AlifCore_BRC.h"
#include "AlifCore_LList.h" // use AlifCore_BRC.h instade
#include "AlifCore_FreeList.h"


class AlifThreadImpl {
public:
	// semi-public fields are in AlifThread.
	AlifThread base{};

	LListNode memFreeQueue{};


	class {
	public:
		AlifSizeT* refCounts{};
		AlifSizeT size{};
		AlifIntT isFinalized{};
	} types{};
};
