#pragma once
















































































class TS {
public:
	AlifThreadState* prev;
	AlifThreadState* next;
	AlifInterpreterState* interp;

	struct {
		unsigned int initialized : 1;
		unsigned int bound : 1;
		unsigned int unbound : 1;
		unsigned int boundGilstate : 1;
		unsigned int active : 1;

		unsigned int finalizing : 1;
		unsigned int cleared : 1;
		unsigned int finalized : 1;

		unsigned int : 24;
	} status;

	int alifRecursionRemaining;
	int alifRecursionLimit;

	int cRecursionRemaining;
	int recursionHeadroom;

	int tracing;
	int whatEvent;

	AlifCFrame* cframe;

	AlifTraceFunc cProfileFunc;
	AlifTraceFunc cTraceFunc;
	AlifObject* cProfileObj;
	AlifObject* cTraceObj;

	AlifObject* currentException;

	AlifErrStackItem* excInfo;

	AlifObject* dict;

	int gilstateCounter;

	AlifObject* asyncExc;
	unsigned long threadId;

	unsigned long nativeThreadId;

	AlifTrashCan trash;

	void (*onDelete)(void*);
	void* onDeleteData;

	int coroutineOriginTrackingDepth;

	AlifObject* asyncGenFirstiter;
	AlifObject* asyncGenFinalizer;

	AlifObject* context;
	uint64_t contextVer;

	uint64_t id;

	AlifStackChunk* datastackChunk;
	AlifObject** datastackTop;
	AlifObject** datastackLimit;

	AlifErrStackItem excState;

	AlifCFrame rootCFrame;
};
