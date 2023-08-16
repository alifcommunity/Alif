#pragma once













#include "alifCore_atomic.h"
#include "alifCore_cevalState.h"














#include "alifCore_alifMemory.h"


#include "alifCore_unicodeObject.h"














class IS {
public:
	CevalState ceval;

	AlifInterpreterState* next;

	int64_t id;
	int64_t id_refcount;
	int requires_idref;
	AlifThreadTypeLock idMutex;

	int initialized;
	int finalizing;

	uint64_t monitoringVersion;
	uint64_t lastRestartVersion;
	struct AlifThreads {
		uint64_t nextUniqueId;
		AlifThreadState* head;
		long count;

		size_t stackSize;
	} threads;

	AlifRuntimeState* runtime;

	AlifAtomicAddress finalizing;

	GcRuntimeState gc;

	AlifObject* sysdict;

	AlifObject* builtins;

	ImportState imports;

	GilRuntimeState gil;


	AlifObject* codecSearchPath;
	AlifObject* codecSearchCache;
	AlifObject* codecErrorRegistry;
	int codecsInitialized;

	AlifConfig config;
	unsigned long featureFlags;

	AlifObject* dict; 

	AlifObject* sysdictCopy;
	AlifObject* builtinsCopy;
	AlifFrameEvalFunction evalFrame;

	AlifFunctionWatchCallback funcWatchers[FUNC_MAX_WATCHERS];
	uint8_t activeFuncWatchers;

	AlifSizeT coExtraUserCount;
	FreeFunc coExtraFreeFuncs[MAX_CO_EXTRA_USERS];

#ifdef HAVE_FORK
	AlifObject* beforeForkers;
	AlifObject* afterForkersParent;
	AlifObject* afterForkersChild;
#endif

	WarningsRuntimeState warnings;
	AtexitState atexit;

	ObmallocState obmalloc;

	AlifObject* auditHooks;
	AlifTypeWatchCallback typeWatchers[TYPE_MAX_WATCHERS];
	AlifCodeWatchCallback codeWatchers[CODE_MAX_WATCHERS];
	uint8_t active_code_watchers;

	AlifObjectState objectState;
	AlifUnicodeState unicode;
	AlifFloatState floatState;
	AlifLongState longState;
	DtoaState dtoa;
	AlifFuncState funcState;

	AlifSliceObject* sliceCache;

	AlifTupleState tuple;
	AlifListState list;
	AlifDictState dictState;
	AlifAsyncGenState asyncGen;
	AlifContextState context;
	AlifExcState excState;

	AstState ast;
	TypesState types;
	CallableCache callableCache;
	AlifOptimizerObject* optimizer;
	uint16_t optimizerResumeThreshold;
	uint16_t optimizerBackedgeThreshold;

	AlifMonitors monitors;
	bool fOpcodeTraceSet;
	bool sysProfileInitialized;
	bool sysTraceInitialized;
	AlifSizeT sysProfilingThreads; 
	AlifSizeT sysTracingThreads;
	AlifObject* monitoringCallables[ALIF_MONITORING_TOOL_IDS][ALIF_MONITORING_EVENTS];
	AlifObject* monitoringToolNames[ALIF_MONITORING_TOOL_IDS];

	AlifInterpCachedObjects cachedObjects;
	AlifInterpStaticObjects staticObjects;

	AlifThreadState initialThread;
};
