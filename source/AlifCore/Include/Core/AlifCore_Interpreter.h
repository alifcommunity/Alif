#pragma once

#include "AlifCore_EvalState.h"
#include "AlifCore_ThreadState.h"
#include "AlifCore_TypeID.h"
#include "AlifCore_Memory.h"

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
	AlifConfig config{};

	//AlifFrameEvalFunction evalFrame{};

	AlifTypeIDPool typeIDs{};

	AlifGCDureRun gc{};

	AlifObject* builtins{};

	//class ImportState imports;

	BRCState brc{};  // biased reference counting state

	StopTheWorldState stopTheWorld{};

	AlifMemory* memory_{};

	//AlifObjectState objectState{};

	 AlifMemInterpFreeQueue memFreeQueue{};


	//class TypesState types;

	AlifThreadImpl initialThread{};
};




extern const AlifConfig* alifInterpreter_getConfig(AlifInterpreter*); // 329



AlifIntT alifInterpreter_new(AlifThread*, AlifInterpreter**); // 399

static inline AlifThread* alifInterpreter_getFinalizing(AlifInterpreter* _interp) { //  289
	return (AlifThread*)alifAtomic_loadPtrRelaxed(&_interp->finalizing);
}
