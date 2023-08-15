#pragma once









#include "alifCore_atomic.h"
#include "alifCore_condVar.h"







#undef FORCE_SWITCHING
#define FORCE_SWITCHING


class GilRuntimeState {
public:
	unsigned long interval;
	AlifAtomicAddress lastHolder;
	AlifAtomicInt locked;
	unsigned long switchNumber;

	AlifCondT cond;
	AlifMutexT mutex;
#ifdef FORCE_SWITCHING
	AlifCondT switchCond;
	AlifMutexT switchMutex;
#endif
};
