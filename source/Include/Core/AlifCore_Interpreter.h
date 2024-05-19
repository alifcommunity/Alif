#pragma once

#include "AlifCore_Memory.h"






class AlifInterpreter {
public:

	//CEval cEval;

	AlifInterpreter* next{};

	AlifIntT id_{};
	//AlifIntT idRefCount{};
	//AlifIntT reqIDRef{};

	//AlifThreadTypeLock idMutex{};

//#define ALIF_INTERPRETERSTATE_WHENCE_NOTSET -1
//#define ALIF_INTERPRETERSTATE_WHENCE_UNKNOWN 0
//#define ALIF_INTERPRETERSTATE_WHENCE_RUNTIME 1
//#define ALIF_INTERPRETERSTATE_WHENCE_LEGACY_CAPI 2
//#define ALIF_INTERPRETERSTATE_WHENCE_CAPI 3
//#define ALIF_INTERPRETERSTATE_WHENCE_XI 4
//#define ALIF_INTERPRETERSTATE_WHENCE_STDLIB 5
//#define ALIF_INTERPRETERSTATE_WHENCE_MAX 5
//	long _whence;

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


	AlifMemory* memory_{};
};






AlifIntT alifInterpreter_new(AlifThread*, AlifInterpreter**);