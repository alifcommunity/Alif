#pragma once









//#include <stdbool.h>



#include "alifCore_atomic.h" 
#include "alifCore_ceval_state.h"
#include "alifCore_code.h"   






#include "alifCore_gc.h"          


#include "alifCore_import.h"
#include "alifCore_instruments.h"  


#include "alifCore_obmalloc.h"  

#include "alifCore_typeObject.h" 















class IS {
public:



	class CEvalState cEval;

	AlifInterpreterState* next;

	int64_t iD;
	int64_t idRefcount;
	int requiresIdref;
	AlifThreadTypeLock iDmutex;





	int initialized;
	int finalizing;

	uint64_t monitoringVersion;
	uint64_t lastRestartVersion;
	class Alifthreads {
	public:
		uint64_t nextUniqueID;

		AlifThreadState* head;

		long count;



		size_t stacksize;
	}thread;




	class AlifRuntimeState* runtime; // تم إضافة class ليعمل البرنامج






	AlifAtomicAddress alifFinalizing;

	GCRuntimeState gc;














	AlifObject* sysDict;


	AlifObject* builtins;

	//ImportState imports;


	GilRuntimeState gil;






	AlifObject* codecSearchPath;
	AlifObject* codecSearchCache;
	AlifObject* codecErrorRegistry;
	int codecsInitialized;

	AlifConfig config;
	unsigned long featureFlags;

	AlifObject* dict;  

	AlifObject* sysDictCopy;
	AlifObject* builtinsCopy;

	//AlifFrameEvalFunction evalFrame;

	//AlifFunctionWatchCallBack funcWatchers[FUNC_MAX_WATCHERS];

	uint8_t activeFuncWatchers;

	AlifSizeT coExtraUserCount;
	//FreeFunc coExtraFreeFuncs[MAX_COEXTRA_USERS];

#ifdef HAVE_FORK
	AlifObject* beforeForkers;
	AlifObject* afterForkersParent;
	AlifObject* afterForkersChild;
#endif

	//WarningsRuntimeState warnings;
	//AtexitState atexit;

	ObmallocState obmalloc;

	////AlifObject* auditHooks;
	//AlifTypeWatchCallBack typeWatchers[TYPE_MAX_WATCHERS];
	//AlifCodeWatchCallBack codeWatchers[CODE_MAX_WATCHERS];

	uint8_t activeCodeWatchers;

	//AlifObjectState objectState;
	//AlifUnicodeState unicode;
	//AlifFloatState floatState;
	//AlifLongState longState;
	//DtoaState dtoa;
	//AlifFuncState funcState;


	//AlifSliceObject* sliceCache;

	//AlifTupleState tuple;
	//AlifListState list;
	//AlifDictState dictState;
	//AlifAsyncGenState asyncGen;
	//AlfiContextState context;
	//AlifExcState excState;

	//ASTState ast;
	TypesState types;
	//CallableCache callableCache;
	//AlifOptimizerObject* optimizer;
	uint16_t optimizerResumeThreshold;
	uint16_t optimizerBackedgeThreshold;

	AlifGlobalMonitors monitors;
	bool fOpcodeTraceSet;
	bool sysProfileInitialized;
	bool sysTraceInitialized;
	AlifSizeT sysProfilingThreads; 
	AlifSizeT sysTracingThreads; 
	AlifObject* monitoringCallables[ALIFMONITORING_TOOL_IDS][ALIFMONITORING_EVENTS];
	AlifObject* monitoringToolNames[ALIFMONITORING_TOOL_IDS];

	//AlifInterpCachedObjects cachedObjects;
	//AlifInterpStaticObjects staticObjects;


	AlifThreadState initialThread;
};







































extern const AlifConfig* alifInterpreterState_getConfig(AlifInterpreterState*);

















































#define ALIFRTFLAGS_USEMAIN_OBMALLOC (1UL << 5)
