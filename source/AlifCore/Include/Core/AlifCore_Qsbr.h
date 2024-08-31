#pragma once

#include "AlifCore_Lock.h"

#define QSBR_OFFLINE 0 // 28
#define QSBR_INITIAL 1 // 29
#define QSBR_INCR    2 // 30


#define QSBR_LT(a, b) ((int64_t)((a)-(b)) < 0) // 35
#define QSBR_LEQ(a, b) ((int64_t)((a)-(b)) <= 0) // 36

class QsbrShared; // 38
class AlifThreadImpl;   // 39


class QsbrThreadState { // 1213
public:
	uint64_t seq{};

	class QsbrShared* shared{};

	AlifThread* tstate{};

	AlifIntT deferrals{};

	bool allocated{};
	class QsbrThreadState* freeListNext{};
};


class QsbrPad { // 61
public:
	class QsbrThreadState qsbr {};
	char __padding[64 - sizeof(class QsbrThreadState)]{};
};


class QsbrShared { //  67
public:
	uint64_t wrSeq{};

	uint64_t rdSeq{};

	class QsbrPad* array{};
	AlifSizeT size{};

	AlifMutex mutex{};
	class QsbrThreadState* freeList{};
};


static inline uint64_t alifQsbr_sharedCurrent(class QsbrShared* _shared)
{
	return alifAtomic_loadUint64Acquire(&_shared->wrSeq);
}

uint64_t alifQsbr_deferredAdvance(class QsbrThreadState*); // 117

bool alifQsbr_poll(class QsbrThreadState* , uint64_t ); // 122
