#pragma once










#include "alifCore_atomic.h"         




#include "alifCore_import.h"
#include "alifCore_interp.h"


#include "alifCore_alifMem.h"       

#include "alifCore_alifThread.h"       

#include "alifCore_time.h"      














































































































class AlifRuntimeState {
public:
	//AlifDebugOffsets debugOffsets;

	int initialized;

	int preinitializing;

	int preinitialized;

	int coreInitialized;

	int initialized2;

	AlifAtomicAddress finalizing;

	class AlifInterpreters {
	public:


		AlifThreadTypeLock mutex;


		AlifInterpreterState* head;


		AlifInterpreterState* main;


		int64_t nextID;


	}alifInterpreters;

	unsigned long mainThread;

	class XIDRegistry {
	public:


		AlifThreadTypeLock mutex;


		//XIDRegitem* head;


	};

	AlifMemAllocators allocators;
	ObmallocGlobalState obmalloc;
	//AlifhashRuntimeState alifHashState;
	TimeRuntimeState time;
	AlifThreadRuntimeState threads;
	//SignalsRuntimeState signals;

	AlifTssT autoTssKey;

	AlifTssT trashTssKey;

	//AlifWideStringList origArgv;

	//ParserRuntimeState parser;

	//ATExitRuntimeState aTexit;

	//ImportRuntimeState imports;
	//CevalRuntimeState ceval;
	//GilStateRuntimeState gilState;
	//GetArgsRuntimeState getArgs;
	//FileUtilsState fileUtils;
	//FaultHandlerRuntimeState faultHandler;
	//TracemallocRuntimeState tracemalloc;

	AlifPreConfig preConfig;

	//AlifOpenCodeHookFunction openCodeHook;
	void* openCodeUserdata;
	class AuditHooks {
	public:
		AlifThreadTypeLock mutex;
		//AlifAuditHookEntry* head;
	};

	//AlifObjectRuntimeState objectState;
	//AlifFloatRuntimeState floatState;
	//AlifUnicodeRuntimeState unicodeState;
	//TypesRuntimeState types;

	//AlifCachedObjects cachedObjects;
	//AlifStaticObjects staticObjects;

	AlifInterpreterState mainInterpreter;
};















































ALIFAPI_DATA(AlifRuntimeState) alifRuntime;















static inline AlifThreadState* alifRuntimeState_getFinalizing(AlifRuntimeState* _runtime) {
	return (AlifThreadState*)ALIFATOMIC_LOAD_RELAXED(&_runtime->finalizing);
}
