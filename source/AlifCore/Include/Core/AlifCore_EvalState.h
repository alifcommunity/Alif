#pragma once

#include "AlifCore_Lock.h"
#include "AlifCore_GIL.h"





typedef AlifIntT (*AlifPendingCallFunc)(void*); // 15

class PendingCall { // 17
public:
	AlifPendingCallFunc func{};
	void* arg{};
	AlifIntT flags{};
};

#define PENDINGCALLSARRAYSIZE 300 // 23



class PendingCalls { // 43
public:
	AlifThread* handlingThread{};
	AlifMutex mutex{};
	int32_t npending;
	int32_t max{};
	int32_t maxLoop{};
	PendingCall calls[PENDINGCALLSARRAYSIZE]{};
	AlifIntT first{};
	AlifIntT next{};
};









class EvalDureRunState { // 83
public:
	class {
	public:
#ifdef ALIF_HAVE_PERF_TRAMPOLINE
		PerfStatusT status{};
		AlifIntT perfTrampolineType{};
		AlifSizeT extraCodeIndex{};
		CodeArenaST* codeArena{};
		TrampolineApiST trampolineAPI{};
		FILE* mapFile{};
		AlifSizeT persistAfterFork{};
#else
		AlifIntT notUsed{};
#endif
	} perf;

	PendingCalls pendingMainThread{};
	AlifMutex sysTraceProfileMutex{};
};






class AlifEval { // 119
public:
	uintptr_t instrumentationVersion{};
	AlifIntT recursionLimit{};
	GILDureRunState* gil_{};
	AlifIntT ownGIL{};
	PendingCalls pending{};
};
