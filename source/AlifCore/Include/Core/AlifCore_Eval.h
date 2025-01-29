#pragma once


#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"










AlifIntT alifEval_makePendingCalls(AlifThread*); // 37


#define ALIF_DEFAULT_RECURSION_LIMIT 1000 // 40

// 48
#define ALIF_PENDING_MAINTHREADONLY 1
#define ALIF_PENDING_RAWFREE 2




extern AlifObject* _alifEval_getBuiltins(AlifThread*); // 85
extern AlifObject* _alifEval_builtinsFromGlobals(AlifThread*, AlifObject*); // 86



static inline AlifObject* alifEval_evalFrame(AlifThread* _tstate,
	class AlifInterpreterFrame* _frame, AlifIntT _throwflag) { // 114
	if (_tstate->interpreter->evalFrame == nullptr) {
		return alifEval_evalFrameDefault(_tstate, _frame, _throwflag);
	}
	return _tstate->interpreter->evalFrame(_tstate, _frame, _throwflag);
}




extern AlifObject* alifEval_vector(AlifThread*, AlifFunctionObject*,
	AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*); // 125



extern void alifEval_initGIL(AlifThread*, AlifIntT); // 131
extern void alifEval_finiGIL(AlifInterpreter*); // 132

extern void alifEval_acquireLock(AlifThread*); // 134

extern void alifEval_releaseLock(AlifInterpreter*, AlifThread*, AlifIntT); // 136





static inline AlifIntT alifEval_isGILEnabled(AlifThread* _thread) { // 145
	GILDureRunState* gil = _thread->interpreter->eval.gil_;
	return alifAtomic_loadIntRelaxed(&gil->enabled) != 0;
}


#ifdef USE_STACKCHECK // 187
static inline AlifIntT alif_makeRecCheck(AlifThread* _thread) {
	return (_thread->cppRecursionRemaining-- < 0
		or (_thread->cppRecursionRemaining & 63) == 0);
}
#else
static inline AlifIntT alif_makeRecCheck(AlifThread* _thread) {
	return _thread->cppRecursionRemaining-- < 0;
}
#endif

AlifIntT alif_checkRecursiveCall(AlifThread*, const char*); // 202

AlifIntT _alif_checkRecursiveCallAlif(AlifThread*); // 206

static inline AlifIntT _alif_enterRecursiveCallThread(AlifThread* _thread,
	const char* where) { // 209
	return (alif_makeRecCheck(_thread) and alif_checkRecursiveCall(_thread, where));
}

static inline AlifIntT _alif_enterRecursiveCall(const char* _where) { // 219
	AlifThread* thread = _alifThread_get();
	return _alif_enterRecursiveCallThread(thread, _where);
}

static inline void _alif_leaveRecursiveCallThread(AlifThread* _thread) { // 224
	_thread->cppRecursionRemaining++;
}

static inline void _alif_leaveRecursiveCall(void) { // 228
	AlifThread* thread = _alifThread_get();
	_alif_leaveRecursiveCallThread(thread);
}


AlifObject* _alifEval_importFrom(AlifThread*, AlifObject*, AlifObject*); // 262
AlifObject* _alifEval_importName(AlifThread*, AlifInterpreterFrame*, AlifObject*, AlifObject*, AlifObject*); // 263
AlifIntT _alifEval_unpackIterableStackRef(AlifThread*, AlifStackRef, AlifIntT, AlifIntT, AlifStackRef*); // 267
void _alifEval_frameClearAndPop(AlifThread* _thread, AlifInterpreterFrame*); // 268
AlifObject** _alifObjectArray_fromStackRefArray(AlifStackRef*, AlifSizeT, AlifObject**); // 269


void _alifObjectArray_free(AlifObject**, AlifObject**); // 271

void _alifEval_loadGlobalStackRef(AlifObject*, AlifObject*, AlifObject*, AlifStackRef*); // 274

AlifObject* _alifEval_loadName(AlifThread*, AlifInterpreterFrame*, AlifObject*); // 276

// 279
#define ALIF_GIL_DROP_REQUEST_BIT (1U << 0)
#define ALIF_SIGNALS_PENDING_BIT (1U << 1)
#define ALIF_CALLS_TO_DO_BIT (1U << 2)
#define ALIF_ASYNC_EXCEPTION_BIT (1U << 3)
#define ALIF_GC_SCHEDULED_BIT (1U << 4)
#define ALIF_EVAL_PLEASE_STOP_BIT (1U << 5)
#define ALIF_EVAL_EXPLICIT_MERGE_BIT (1U << 6)

#define ALIF_EVAL_EVENTS_BITS 8
#define ALIF_EVAL_EVENTS_MASK ((1 << ALIF_EVAL_EVENTS_BITS)-1)


static inline void alifSet_evalBreakerBit(AlifThread* _thread, uintptr_t _bit) { // 291
	alifAtomic_orUintptr(&_thread->evalBreaker, _bit);
}

static inline void alifUnset_evalBreakerBit(AlifThread* _thread, uintptr_t _bit) { // 297
	alifAtomic_andUintptr(&_thread->evalBreaker, ~_bit);
}

static inline AlifIntT alifEval_breakerBitIsSet(AlifThread* _thread, uintptr_t _bit) { // 303
	uintptr_t b = alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker);
	return (b & _bit) != 0;
}



void alifSet_evalBreakerBitAll(AlifInterpreter*, uintptr_t); // 312
void alifUnset_evalBreakerBitAll(AlifInterpreter*, uintptr_t); // 313
