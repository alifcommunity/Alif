#pragma once


#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_FreeList.h"


class AlifThreadImpl {
public:
	AlifThread base{};

	class QSBRThreadState* qsbr{};  // only used by free-threaded build


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
