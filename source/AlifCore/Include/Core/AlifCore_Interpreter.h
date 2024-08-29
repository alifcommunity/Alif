#pragma once

#include "AlifCore_EvalState.h"
#include "AlifCore_ThreadState.h"
#include "AlifCore_TypeID.h"


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

	GILDureRunState gil_{};


	BRCState brc{};  // biased reference counting state

	AlifMemory* memory_{};

	//AlifObjectState objectState{};

	//class TypesState types;

	AlifThreadImpl initialThread{};
};




extern const AlifConfig* alifInterpreter_getConfig(AlifInterpreter*); // 329



AlifIntT alifInterpreter_new(AlifThread*, AlifInterpreter**); // 399
