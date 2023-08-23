#pragma once





































































































































class AlifRuntimeState {
public:
	//AlifDebugOffsets debugOffsets;

	int initialized;

	int preinitializing;

	int preinitialized;

	int coreInitialized;

	int initialized2;

	//AlifAtomicAddress finalizing;

	class AlifInterpreters {
	public:
		//AlifThreadTypeLock mutex;
		//AlifInterpreterState* head;
		//AlifInterpreterState* main;
		int64_t next_id;
	};

	unsigned long main_thread;

	class XIDRegistry {
	public:
		//AlifThreadTypeLock mutex;
		//XIDRegitem* head;
	};

	//AlifMemAllocators allocators;
	//ObmallocGlobalState obmalloc;
	//AlifhashRuntimeState alifHashState;
	//TimeRuntimeState time;
	//AlifThreadRuntimeState threads;
	//SignalsRuntimeState signals;

	//AlifTssT autoTSSkey;

	//AlifTssT trashTSSkey;

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

	//AlifPreConfig preConfig;

	//AlifOpenCodeHookFunction openCodeHook;
	void* openCodeUserdata;
	class AuditHooks {
	public:
		//AlifThreadTypeLock mutex;
		//AlifAuditHookEntry* head;
	};

	//AlifObjectRuntimeState objectState;
	//AlifFloatRuntimeState floatState;
	//AlifUnicodeRuntimeState unicodeState;
	//TypesRuntimeState types;

	//AlifCachedObjects cachedObjects;
	//AlifStaticObjects staticObjects;

	//AlifInterpreterState mainInterpreter;
};
