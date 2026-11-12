#pragma once











ALIFAPI_FUNC(AlifInterpreter*) alifInterpreter_get(); // 26

ALIFAPI_FUNC(int64_t) alifInterpreter_getID(AlifInterpreter*); // 36


ALIFAPI_FUNC(AlifThread*) alifThreadState_new(AlifInterpreter*); // 48
ALIFAPI_FUNC(void) alifThreadState_clear(AlifThread*);
ALIFAPI_FUNC(void) alifThreadState_delete(AlifThread*); // 50

ALIFAPI_FUNC(AlifThread*) alifThread_get(); // 60

#define ALIFTHREADSTATE_GET() alifThread_get() // 63

ALIFAPI_FUNC(AlifThread*) alifThreadState_swap(AlifThread*); // 65

ALIFAPI_FUNC(AlifObject*) alifThreadState_getDict(); // 66




/* ----------------------------------------------------------------------------------------------------------- */



class AlifErrStackItem { // 32
public:
	AlifObject* excValue{};
	AlifErrStackItem* previousItem{};
};



class AlifStackChunk { // 52
public:
	AlifStackChunk* previous{};
	AlifUSizeT size{};
	AlifUSizeT top{};
	AlifObject* data[1]{}; /* Variable sized */
};



class AlifThread { // 59
public:
	AlifThread* prev{};
	AlifThread* next{};
	AlifInterpreter* interpreter{};

	uintptr_t evalBreaker{};

	class {
	public:
		AlifUIntT initialized : 1;

		AlifUIntT bound : 1;
		AlifUIntT unbound : 1;
		AlifUIntT boundGILState : 1;
		AlifUIntT active : 1;

		AlifUIntT finalizing : 1;
		AlifUIntT cleared : 1;
		AlifUIntT finalized : 1;
		AlifUIntT holdsGIL : 1;
		/* padding to align to 4 bytes */
		AlifUIntT : 23;
	} status{};

	AlifIntT state{};

	class AlifInterpreterFrame* currentFrame{};

	AlifIntT alifRecursionRemaining{};
	AlifIntT alifRecursionLimit{};

	AlifIntT cppRecursionRemaining{};
	AlifIntT recursionHeadroom{}; /* Allow 50 more calls to handle any errors. */

	AlifIntT tracing{};


	AlifObject* currentException{};

	AlifErrStackItem* excInfo{};

	AlifObject* dict{};

	AlifUSizeT threadID{};

	unsigned long nativeThreadID{};

	AlifObject* deleteLater{};

	uintptr_t criticalSection{};

	AlifIntT coroutineOriginTrackingDepth{};

	AlifSizeT id{};

	AlifStackChunk* dataStackChunk{};
	AlifObject** dataStackTop{};
	AlifObject** dataStackLimit{};

	AlifErrStackItem excState{};

	uint64_t dictGlobalVersion{};
};











#define ALIFCPP_RECURSION_LIMIT 3000 // 214


ALIFAPI_FUNC(AlifInterpreter*) alifInterpreterState_main(void); // 268
ALIFAPI_FUNC(AlifInterpreter*) alifInterpreter_head(); // 269

typedef AlifObject* (*AlifFrameEvalFunction)(AlifThread*,
	class AlifInterpreterFrame*, AlifIntT); // 271
