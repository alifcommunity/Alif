#pragma once












ALIFAPI_FUNC(AlifInterpreterState*) alifInterpreterState_new();

















class AlifErrStackItem {
public:












	//AlifObject* excValue;

	class ErrStackitem* previousItem;

};













class TS{
public:

	AlifThreadState* prev;
	AlifThreadState* next;
	AlifInterpreterState* interp;

	class Status{
	public:


		unsigned int initialized : 1;


		unsigned int bound : 1;

		unsigned int unbound : 1;

		unsigned int boundGilstate : 1;

		unsigned int active : 1;


		unsigned int finalizing : 1;
		unsigned int cleared : 1;
		unsigned int finalized : 1;


		unsigned int : 24;
	}status;

	int alifRecursionRemaining;
	int alifRecursionLimit;

	int cRecursionRemaining;
	int recursionHeadroom;





	int tracing;
	int whatEvent; 


	//AlifInterpreterFrame* currentFrame;

	//AlifTraceFunc cProfileFunc;
	//AlifTraceFunc cTraceFunc;
	//AlifObject* cProfileObj;
	//AlifObject* cTraceObj;


	//AlifObject* currentException;




	AlifErrStackItem* excInfo;

	//AlifObject* dict;  

	int gilstateCounter;

	//AlifObject* asyncExc; 
	unsigned long threadID; 





	unsigned long nativeThreadID;

	//AlifTrashCan trash;
























	void (*OnDelete)(void*);
	void* onDeleteData;

	int coroutineOriginTrackingDepth;

	//AlifObject* asyncGenFirstiter;
	//AlifObject* asyncGenFinalizer;

	//AlifObject* context;
	uint64_t contextVer;


	uint64_t iD;

	//AlifStackChunk* dataStackChunk;
	//AlifObject** dataStackTop;
	//AlifObject** dataStackLimit;














	AlifErrStackItem excState;

};

#ifdef __wasi__



#  define ALIFC_RECURSION_LIMIT 500
#else

#  define ALIFC_RECURSION_LIMIT 1500
#endif



































ALIFAPI_FUNC(AlifInterpreterState*) alifInterpreterState_main();
ALIFAPI_FUNC(AlifInterpreterState*) alifInterpreterState_head();
ALIFAPI_FUNC(AlifInterpreterState*) alifInterpreterState_next(AlifInterpreterState*);
