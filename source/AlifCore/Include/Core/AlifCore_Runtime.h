#pragma once

#include "AlifCore_Audit.h"
#include "AlifCore_ObjectState.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_FloatObject.h"
#include "AlifCore_Parser.h"
#include "AlifCore_Hash.h"
#include "AlifCore_Thread.h"
#include "AlifCore_Signal.h"
#include "AlifCore_Import.h"
#include "AlifCore_UStrObject.h"







class AlifRuntime { // 159
public:
	AlifIntT selfInitialized{};
	AlifIntT preinitializing{};
	AlifIntT preinitialized{};
	AlifIntT coreInitialized{};

	AlifIntT initialized{};

	AlifThread* finalizing_{};

	unsigned long finalizingID{};

	class AlifInterpreters {
	public:
		AlifMutex mutex{};
		AlifInterpreter* head{};
		AlifInterpreter* main{};
		AlifIntT nextID{};
	}interpreters;

	AlifIntT mainThreadID{};
	AlifThread* mainThread{};

	AlifThreadRuntimeState threads{};
	SignalsRuntimeState signals{};



	AlifTssT autoTSSKey{};
	AlifTssT trashTSSKey{};

	AlifWStringList origArgv{};

	AlifParserRuntimeState parser{};
	ImportRuntimeState imports{};
	EvalRuntimeState eval{};
	class GITStateRuntimeState {
	public:
		AlifIntT checkEnabled{};
		AlifInterpreter* autoInterpreterState;
	}gilState;
	class GetArgsRuntimeState {
	public:
		AlifArgParser* staticParsers{};
	}getArgs;
	RefTracerRuntimeState refTracer{};

	AlifRWMutex stopTheWorldMutex{};
	StopTheWorldState stopTheWorld{};

	AlifPreConfig preConfig{};

	AlifOpenCodeHookFunction openCodeHook{};
	void* openCodeUserdata{};
	struct {
		AlifMutex mutex{};
		AlifAuditHookEntry* head{};
	} auditHooks{};

	AlifFloatRuntimeState floatState{};
	AlifUnicodeRuntimeState unicodeState{};
	TypesRuntimeState types{};

	AlifCachedObjects cachedObjects{};
	AlifStaticObjects staticObjects{};

	AlifInterpreter mainInterpreter{};
};

extern AlifRuntime _alifRuntime_; // 318

extern AlifStatus _alifRuntimeState_init(AlifRuntime*); // 320
extern void _alifRuntimeState_fini(AlifRuntime*); // 321


extern AlifStatus _alifRuntime_initialize(); // 329




extern void _alifRuntime_finalize(void);

static inline AlifThread* alifRuntimeState_getFinalizing(AlifRuntime* _runtime) { // 383
	return (AlifThread*)alifAtomic_loadPtrRelaxed(&_runtime->finalizing_);
}

static inline unsigned long alifRuntimeState_getFinalizingID(AlifRuntime* _runtime) { // 388
	return ALIFATOMIC_LOAD_ULONG_RELAXED(&_runtime->finalizingID);
}
