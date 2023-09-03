#pragma once









//#include <stdbool.h>



#include "alifCore_atomic.h" 

#include "alifCore_code.h"   









#include "alifCore_import.h"



#include "alifCore_obmalloc.h"  

















class IS {
public:





	AlifInterpreterState* next;

	int64_t id;
	int64_t idRefcount;
	int requiresIdref;
	AlifThreadTypeLock IDmutex;





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




	class AlifRuntimeState* runtime;






	AlifAtomicAddress alifFinalizing;

	//GCRuntimeState gc;














	//AlifObject* sysdict;


	//AlifObject* builtins;

	//ImportState imports;


	//GilRuntimeState gil;






	//AlifObject* codecSearchPath;
	//AlifObject* codecSearchCache;
	//AlifObject* codecErrorRegistry;
	int codecsInitialized;

	AlifConfig config;
	unsigned long featureFlags;

	//AlifObject* dict;  

	//AlifObject* sysdictCopy;
	//AlifObject* builtinsCopy;

	//AlifFrameEvalFunction evalFrame;

	//AlifFunctionWatchCallBack funcWatchers[FUNC_MAX_WATCHERS];

	uint8_t activeFuncWatchers;

	AlifSizeT co_extra_user_count;
	//FreeFunc coExtraFreeFuncs[MAX_COEXTRA_USERS];

#ifdef HAVE_FORK
	AlifObject* beforeForkers;
	AlifObject* afterForkersParent;
	AlifObject* afterForkersChild;
#endif

	//WarningsRuntimeState warnings;
	//AtexitState atexit;

	ObmallocState obmalloc;

	//AlifObject* auditHooks;
	//AlifTypeWatchCallBack typeWatchers[TYPE_MAX_WATCHERS];
	//AlifCodeWatchCallBack codeWatchers[CODE_MAX_WATCHERS];

	uint8_t activeCodeWatchers;

	//AlifObjectState object_state;
	//AlifUnicodeState unicode;
	//AlifFloatState float_state;
	//AlifLongState long_state;
	//DtoaState dtoa;
	//AlifFuncState func_state;


	//AlifSliceObject* sliceCache;

	//AlifTupleState tuple;
	//AlifListState list;
	//AlifDictState dict_state;
	//AlifSsyncGenState async_gen;
	//AlfiContextState context;
	//AlifExcState exc_state;

	//ASTState ast;
	//TypesState types;
	//CallableCache callableCache;
	//AlifOptimizerObject* optimizer;
	uint16_t optimizerResumeThreshold;
	uint16_t optimizerBackedgeThreshold;

	//AlifMonitors monitors;
	bool fOpcodeTraceSet;
	bool sysProfileInitialized;
	bool sysTraceInitialized;
	AlifSizeT sysProfilingThreads; 
	AlifSizeT sysTracingThreads; 
	//AlifObject* monitoring_callables[ALIFMONITORING_TOOL_IDS][ALIFMONITORING_EVENTS];
	//AlifObject* monitoring_tool_names[ALIFMONITORING_TOOL_IDS];

	//AlifInterpCachedObjects cached_objects;
	//AlifInterpStaticObjects static_objects;


	AlifThreadState initialThread;
};







































extern const AlifConfig* alifInterpreterState_getConfig(AlifInterpreterState*);

















































#define ALIFRTFLAGS_USEMAIN_OBMALLOC (1UL << 5)
