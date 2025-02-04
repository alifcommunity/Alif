#pragma once


#include "AlifCore_DictState.h"
#include "AlifCore_EvalState.h"
#include "AlifCore_Code.h"
#include "AlifCore_Codecs.h"
#include "AlifCore_GC.h"
#include "AlifCore_UniqueID.h"
#include "AlifCore_DoubleToASCII.h"
#include "AlifCore_Function.h"
#include "AlifCore_GenObject.h"
#include "AlifCore_List.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Optimizer.h"
#include "AlifCore_ThreadState.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Import.h"
#include "AlifCore_Tuple.h"
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
#  define NUM_WEAKREF_LIST_LOCKS 127


class RareEvents { // 75
public:
	uint8_t setClass{};
	uint8_t setBases{};
	uint8_t setEvalFrameFunc{};
	uint8_t builtinDict{};
	uint8_t funcModification{};
};

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

	AlifFrameEvalFunction evalFrame{};

	AlifFunctionWatchCallback funcWatchers[FUNC_MAX_WATCHERS]{};

	uint8_t activeFuncWatchers{};

	GCDureRunState gc{};

	AlifObject* sysDict{};

	AlifObject* builtins{};

	ImportState imports{};

	GILDureRunState gil_{};

	CodecsState codecs{};


	QSBRShared qsbr{};

	MimallocInterpState mimalloc{};
	BRCState brc{};  // biased reference counting state
	AlifUniqueIDPool uniqueIDs{};
	AlifMutex weakrefLocks[NUM_WEAKREF_LIST_LOCKS];

	StopTheWorldState stopTheWorld{};

	AlifMemory* memory_{};


	//AlifTypeWatchCallback typeWatchers[TYPE_MAX_WATCHERS]{};
	AlifCodeWatchCallback codeWatchers[CODE_MAX_WATCHERS]{};
	uint8_t activeCodeWatchers{};

	//AlifObjectState objectState{};
	AlifLongState longState{};
	DToAState dtoa{};
	AlifFuncState funcState{};
	AlifCodeState codeState{};

	AlifDictState dictState{};
	AlifMemInterpFreeQueue memFreeQueue{};

	TypesState types{};


	RareEvents rareEvents{};


	AlifInterpCachedObjects cachedObjects{};

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


static inline AlifThread* alifInterpreter_getFinalizing(AlifInterpreter* _interp) { //  289
	return (AlifThread*)alifAtomic_loadPtrRelaxed(&_interp->finalizing);
}

AlifIntT alifInterpreter_new(AlifThread*, AlifInterpreter**); // 399

 //* alif
#define RARE_EVENT_INTERP_INC(_interp, _name) \
    do { \
        AlifIntT val_ = alifAtomic_loadUint8Relaxed((const uint8_t*)_interp->rareEvents._name); \
        if (val_ < UINT8_MAX) { \
            alifAtomic_storeInt((AlifIntT*)_interp->rareEvents._name, val_ + 1); \
        } \
    } while (0); \
