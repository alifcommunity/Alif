#pragma once

#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_FreeList.h"

#include "AlifCore_QSBR.h"

class AlifThreadImpl { // 20
public:
	AlifThread base{};

	QSBRThreadState* qsbr{};
	LListNode memFreeQueue{};

	class GCThreadState gc{};

	AlifFreeLists freeLists{};
	BRCThreadState brc{};

	class {
	public:
		AlifSizeT* refCounts{};
		AlifSizeT size{};
		AlifIntT isFinalized{};
	}types{};
};
