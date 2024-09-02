#pragma once


//#include "AlifCore_Interpreter.h"
//#include "AlifCore_State.h"













#define ALIF_DEFAULT_RECURSION_LIMIT 1000 // 40












extern void alifEval_initGIL(AlifThread*, AlifIntT); // 131
extern void alifEval_finiGIL(AlifInterpreter*); // 132

extern void alifEval_acquireLock(AlifThread*); // 134

extern void alifEval_releaseLock(AlifInterpreter*, AlifThread*, AlifIntT); // 136





static inline AlifIntT alifEval_isGILEnabled(AlifThread* _thread) { // 145
	GILDureRunState* gil = _thread->interpreter->eval.gil_;
	return alifAtomic_loadIntRelaxed(&gil->enabled) != 0;
}






// 279
#define ALIF_GIL_DROP_REQUEST_BIT (1U << 0)
#define ALIF_SIGNALS_PENDING_BIT (1U << 1)
#define ALIF_CALLS_TO_DO_BIT (1U << 2)
#define ALIF_ASYNC_EXCEPTION_BIT (1U << 3)
#define ALIF_GC_SCHEDULED_BIT (1U << 4)
#define ALIF_EVAL_PLEASE_STOP_BIT (1U << 5)
#define ALIF_EVAL_EXPLICIT_MERGE_BIT (1U << 6)



static inline void alifUnset_evalBreakerBit(AlifThread* _thread, uintptr_t _bit) { // 297
	alifAtomic_andUintptr(&_thread->evalBreaker, ~_bit);
}

static inline AlifIntT alifEval_breakerBitIsSet(AlifThread* _thread, uintptr_t _bit) { // 303
	uintptr_t b = alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker);
	return (b & _bit) != 0;
}
