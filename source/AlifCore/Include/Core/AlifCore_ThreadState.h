#pragma once

#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_FreeListState.h"
#include "AlifCore_Mimalloc.h"
#include "AlifCore_QSBR.h"

class AlifThreadImpl { // 20
public:
	AlifThread base{};

	QSBRThreadState* qsbr{};

	LListNode memFreeQueue{};

	GCThreadState gc{};
	MimallocThreadState mimalloc{};

	AlifFreeLists freeLists{};
	BRCThreadState brc{};

	class {
	public:
		AlifSizeT* vals{};
		AlifSizeT size{};
		AlifIntT isFinalized{};
	}refCounts{};

};
