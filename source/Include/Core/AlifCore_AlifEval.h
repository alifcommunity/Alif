#pragma once

#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifState.h"



static inline AlifObject* alifEval_evalFrame(AlifThread* _thread,
	AlifInterpreterFrame* _frame, AlifIntT _throwFlag) {

	if (_thread->interpreter->evalFrame == nullptr) {
		return alifEval_evalFrameDefault(_thread, _frame, _throwFlag);
	}
	return _thread->interpreter->evalFrame(_thread, _frame, _throwFlag);
}

extern AlifObject* alifEval_vector(AlifThread*, AlifFunctionObject*,
	AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*);





extern AlifObject* alifEval_getBuiltins(AlifThread*);
extern AlifObject* alifEval_builtinsFromGlobals(AlifThread*, AlifObject*);




static inline AlifIntT alif_makeRecCheck(AlifThread* _thread) { // 195
	return _thread->recursionRemaining-- < 0;
}

AlifIntT alif_checkRecursiveCall(AlifThread*, const wchar_t*);

static inline AlifIntT alif_enterRecursiveCallThread(AlifThread* _thread, const wchar_t* _where) { // 209
	return (alif_makeRecCheck(_thread) and alif_checkRecursiveCall(_thread, _where));
}

static inline void alif_leaveRecursiveCallThread(AlifThread* _thread) { // 224
	_thread->recursionRemaining++;
}
