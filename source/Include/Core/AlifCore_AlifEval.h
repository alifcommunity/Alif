#pragma once

#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifState.h"



static inline AlifObject* alifEval_evalFrame(AlifThread* _thread, AlifInterpreterFrame* _frame, AlifIntT _throwFlag)
{
	if (_thread->interpreter->evalFrame == nullptr) {
		return alifEval_evalFrameDefault(_thread, _frame, _throwFlag);
	}
	return _thread->interpreter->evalFrame(_thread, _frame, _throwFlag);
}

extern AlifObject* alifEval_vector(AlifThread*, AlifFunctionObject*,
	AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*);
