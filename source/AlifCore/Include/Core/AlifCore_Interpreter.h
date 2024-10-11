#pragma once


#include "AlifCore_DictState.h"
#include "AlifCore_EvalState.h"
#include "AlifCore_GC.h"
#include "AlifCore_TypeID.h"
#include "AlifCore_Memory.h"
#include "AlifCore_ThreadState.h"
#include "AlifCore_Tuple.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Import.h"
#include "AlifCore_TypeObject.h"





class AlifLongState { // 44
public:
	AlifIntT maxStrDigits{};
};


class StopTheWorldState { // 50
public:
	AlifMutex mutex{};
	bool requested{};
	bool worldStopped{};
	bool isGlobal{};

	AlifEvent stopEvent{};
	AlifSizeT threadCountDown{};

	AlifThread* requester{};
};

// 65
#ifdef ALIF_GIL_DISABLED
#  define NUM_WEAKREF_LIST_LOCKS 127
#endif


class AlifInterpreter { // 95
public:

	AlifEval eval{};

	AlifInterpreter* next{};

	AlifIntT id_{};

	AlifIntT initialized{};
	AlifIntT ready{};
	AlifIntT finalizing{};

	class AlifThreads {
	public:
		AlifUSizeT nextUniquID{};
		AlifThread* head{};
		AlifThread* main{};
		AlifUSizeT count{};

		AlifSizeT stackSize{};
	} threads;

	class AlifDureRun* dureRun{};

	AlifThread* finalizing_{};
	unsigned long finalizingID{};

	AlifConfig config{};
	unsigned long featureFlags{};

	//AlifFrameEvalFunction evalFrame{};

	GCDureRunState gc{};

	AlifObject* sysDict{};

	AlifObject* builtins{};

	ImportState imports{};

	GILDureRunState gil_{};


	QSBRShared qsbr{};

#ifdef ALIF_GIL_DISABLED
	MimallocInterpState mimalloc{};
	BRCState brc{};  // biased reference counting state
	AlifTypeIDPool typeIDs{};
	AlifMutex weakrefLocks[NUM_WEAKREF_LIST_LOCKS];
#endif

	StopTheWorldState stopTheWorld{};

	AlifMemory* memory_{};

	AlifInterpCachedObjects cachedObjects{};
	//AlifObjectState objectState{};
	AlifLongState longState{};

	AlifDictState dictState{};
	AlifMemInterpFreeQueue memFreeQueue{};

	TypesState types{};


	//class TypesState types;

	AlifThreadImpl initialThread{};
};



static inline AlifThread* alifInterpreterState_getFinalizing(AlifInterpreter* _interp) { // 288
	return (AlifThread*)alifAtomic_loadPtrRelaxed(&_interp->finalizing_);
}

static inline unsigned long alifInterpreterState_getFinalizingID(AlifInterpreter* _interp) { // 293
	return ALIFATOMIC_LOAD_ULONG_RELAXED(&_interp->finalizingID);
}



extern const AlifConfig* alifInterpreter_getConfig(AlifInterpreter*); // 329

// 379
#define ALIF_RTFLAGS_USE_ALIFMEM (1UL << 5)
#define ALIF_RTFLAGS_MULTI_INTERP_EXTENSIONS (1UL << 8)
#define ALIF_RTFLAGS_THREADS (1UL << 10)
#define ALIF_RTFLAGS_DAEMON_THREADS (1UL << 11)
#define ALIF_RTFLAGS_FORK (1UL << 15)
#define ALIF_RTFLAGS_EXEC (1UL << 16)



AlifIntT alifInterpreter_new(AlifThread*, AlifInterpreter**); // 399

static inline AlifThread* alifInterpreter_getFinalizing(AlifInterpreter* _interp) { //  289
	return (AlifThread*)alifAtomic_loadPtrRelaxed(&_interp->finalizing);
}
