#pragma once

#include "AlifCore_Interpreter.h"
#include "AlifCore_FloatObject.h"
#include "AlifCore_Parser.h"
#include "AlifCore_Hash.h"
#include "AlifCore_Thread.h"
#include "AlifCore_Signal.h"
#include "AlifCore_Import.h"
#include "AlifCore_UStrObject.h"





class GetArgsDureRunState {
public:
	AlifArgParser* staticParsers{};
};

struct GILStateDureRunState { // 34
public:
	AlifIntT checkEnabled{};
	AlifInterpreter* autoInterpreterState{};
};


class AlifAuditHookEntry {
public:
	class AlifAuditHookEntry* next{};
	AlifAuditHookFunction hookCFunction{};
	void* userData{};
};


class RefTracerDureRunState { // 198
public:
	AlifRefTracer tracerFunc{};
	void* tracerData{};
};

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

	AlifThreadDureRunState threads{};
	SignalsDureRunState signals{};



	AlifTssT autoTSSKey{};
	AlifTssT trashTSSKey{};

	AlifWStringList origArgv{};

	AlifParserDureRunState parser{};
	ImportDureRunState imports{};
	EvalDureRunState eval{};
	GILStateDureRunState gilState{};
	GetArgsDureRunState getArgs{};
	RefTracerDureRunState refTracer{};

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
	TypesDureRunState types{};

	AlifCachedObjects cachedObjects{};
	AlifStaticObjects staticObjects{};

	AlifInterpreter mainInterpreter{};
};

extern AlifRuntime _alifRuntime_; // 318

extern AlifStatus _alifRuntimeState_init(AlifRuntime*); // 320
extern void _alifRuntimeState_fini(AlifRuntime*); // 321


extern AlifStatus _alifRuntime_initialize(); // 329




extern void _alifRuntime_finalize(void);

static inline AlifThread* alifDureRunState_getFinalizing(AlifRuntime* _dureRun) { // 383
	return (AlifThread*)alifAtomic_loadPtrRelaxed(&_dureRun->finalizing_);
}

static inline unsigned long alifDureRunState_getFinalizingID(AlifRuntime* _dureRun) { // 388
	return ALIFATOMIC_LOAD_ULONG_RELAXED(&_dureRun->finalizingID);
}
