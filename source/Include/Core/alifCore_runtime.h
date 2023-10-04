#pragma once









//#include "alifCore_atExit.h"       
#include "alifCore_atomic.h"       
//#include "alifCore_cevalState.h"  
//#include "alifCore_faultHandler.h" 
//#include "alifCore_floatObject.h"  
#include "alifCore_import.h"       
#include "alifCore_interp.h"       
//#include "alifCore_objectState.h"
//#include "alifCore_parser.h"
//#include "alifCore_alifHash.h"     
#include "alifCore_alifMem.h"
#include "alifCore_alifThread.h"     
//#include "alifCore_signal.h"       
#include "alifCore_time.h"     
//#include "alifCore_traceMalloc.h"  
//#include "alifCore_typeObject.h"   
//#include "alifCore_unicodeObject.h"











































































































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

	AlifTssT autoTSSKey;

	AlifTssT trashTSSKey;

	AlifWideStringList origArgv;

	//ParserRuntimeState parser;

	//ATExitRuntimeState aTexit;

	//ImportRuntimeState imports;
	//CevalRuntimeState ceval;
	//GilStateRuntimeState gilState;
	//GetArgsRuntimeState getArgs;
	//FileUtilsState fileUtils;
	//FaultHandlerRuntimeState faultHandler;
	//TracemallocRuntimeState traceMalloc;

	AlifPreConfig preConfig;

	////AlifOpenCodeHookFunction openCodeHook; // تم التخلي عن هذه الدالة بسبب عدم الحاجة لها لأن وظيفتها هي حقن برنامج داخل البرنامج
	////void* openCodeUserdata; // تم التخلي عن هذه الدالة
	////class AuditHooks { // تم التخلي عن هذه الدالة بسبب عدم الحاجة لها لأن وظيفتها هي مراقبة النشاطات الضارة
	////public:
	////	AlifThreadTypeLock mutex;
	////	AlifAuditHookEntry* head;
	////};

	//AlifObjectRuntimeState objectState;
	//AlifFloatRuntimeState floatState;
	//AlifUnicodeRuntimeState unicodeState;
	//TypesRuntimeState types;

	//AlifCachedObjects cachedObjects;
	//AlifStaticObjects staticObjects;

	AlifInterpreterState mainInterpreter;
};
































































ALIFAPI_DATA(AlifRuntimeState) alifRuntime;

extern void alifRuntimeState_init(AlifRuntimeState*);
//extern void alifRuntimeState_fini(AlifRuntimeState*);







extern void alifRuntime_initialize();




static inline AlifThreadState* alifRuntimeState_getFinalizing(AlifRuntimeState* _runtime) {
	return (AlifThreadState*)ALIFATOMIC_LOAD_RELAXED(&_runtime->finalizing);
}

static inline void alifRuntimeState_setFinalizing(AlifRuntimeState* _runtime, AlifThreadState* _tState) {
	ALIFATOMIC_STORE_RELAXED(&_runtime->finalizing, (uintptr_t)_tState);
}
