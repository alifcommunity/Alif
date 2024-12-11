#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Backoff.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"

#include "AlifCore_Function.h"

#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeMetaData.h"
#include "AlifCore_Optimizer.h"
#include "AlifCore_OpcodeUtils.h"

#include "AlifCore_State.h"



#include "AlifCore_Frame.h"

















AlifIntT alif_checkRecursiveCall(AlifThread* _thread, const char* _where) { // 305
#ifdef USE_STACKCHECK
	if (alifOS_checkStack()) {
		++_thread->cppRecursionRemaining;
		alifErr_setString(_thread, _alifExcMemoryError_, "Stack overflow");
		return -1;
	}
#endif
	if (_thread->recursionHeadroom) {
		if (_thread->cppRecursionRemaining < -50) {
			//alif_fatalError("Cannot recover from stack overflow.");
		}
	}
	else {
		if (_thread->cppRecursionRemaining <= 0) {
			_thread->recursionHeadroom++;
			//alifErr_format(_thread, _alifExcRecursionError_,
			//	"maximum recursion depth exceeded%s",
			//	where);
			_thread->recursionHeadroom--;
			++_thread->cppRecursionRemaining;
			return -1;
		}
	}
	return 0;
}









AlifObject* alifEval_evalCode(AlifObject* _co,
	AlifObject* _globals, AlifObject* _locals) { // 592
	AlifThread* thread = _alifThread_get();
	if (_locals == nullptr) {
		_locals = _globals;
	}
	AlifObject* builtins = _alifEval_builtinsFromGlobals(thread, _globals); // borrowed ref
	if (builtins == nullptr) {
		return nullptr;
	}
	AlifFrameConstructor desc = {
		.globals = _globals,
		.builtins = builtins,
		.name = ((AlifCodeObject*)_co)->name,
		.qualname = ((AlifCodeObject*)_co)->name,
		.code = _co,
		.defaults = nullptr,
		.kwDefaults = nullptr,
		.closure = nullptr
	};
	AlifFunctionObject* func = _alifFunction_fromConstructor(&desc);
	if (func == nullptr) {
		return nullptr;
	}
	AlifObject* res = alifEval_vector(thread, func, _locals, nullptr, 0, nullptr);
	ALIF_DECREF(func);
	return res;
}










AlifObject* ALIF_HOT_FUNCTION alifEval_evalFrameDefault(AlifThread* _thread,
	AlifInterpreterFrame* _frame, AlifIntT _throwflag) { // 726


}














static AlifInterpreterFrame* _alifEvalFramePushAndInit_unTagged(AlifThread* _thread,
	AlifFunctionObject* _func, AlifObject* _locals, AlifObject* const* _args,
	AlifUSizeT _argCount, AlifObject* _kwNames, AlifInterpreterFrame* _previous) { // 1724
#if defined(ALIF_GIL_DISABLED)
	AlifUSizeT kwCount = _kwNames == nullptr ? 0 : ALIFTUPLE_GET_SIZE(_kwNames);
	AlifUSizeT totalArgCount = _argCount + kwCount;
	AlifStackRef* taggedArgsBuffer = (AlifStackRef*)alifMem_dataAlloc(sizeof(AlifStackRef) * totalArgCount);
	if (taggedArgsBuffer == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	for (AlifUSizeT i = 0; i < _argCount; i++) {
		taggedArgsBuffer[i] = ALIFSTACKREF_FROMPYOBJECTSTEAL(_args[i]);
	}
	for (AlifUSizeT i = 0; i < kwCount; i++) {
		taggedArgsBuffer[_argCount + i] = ALIFSTACKREF_FROMPYOBJECTSTEAL(_args[_argCount + i]);
	}
	AlifInterpreterFrame* res = _alifEval_framePushAndInit(_thread, _func, _locals, (AlifStackRef const*)taggedArgsBuffer, _argCount, _kwNames, _previous);
	alifMem_dataFree(taggedArgsBuffer);
	return res;
#else
	return _alifEval_framePushAndInit(_thread, _func, _locals, (AlifStackRef const*)_args, _argCount, _kwNames, _previous);
#endif
}





AlifObject* alifEval_vector(AlifThread* _tstate, AlifFunctionObject* _func,
	AlifObject* _locals, AlifObject* const* _args, AlifUSizeT _argCount,
	AlifObject* _kwNames) { // 1794
	ALIF_INCREF(_func);
	ALIF_XINCREF(_locals);
	for (AlifUSizeT i = 0; i < _argCount; i++) {
		ALIF_INCREF(_args[i]);
	}
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIF_INCREF(_args[i + _argCount]);
		}
	}
	AlifInterpreterFrame* frame = _alifEvalFramePushAndInit_unTagged(
		_tstate, _func, _locals, _args, _argCount, _kwNames, nullptr);
	if (frame == nullptr) {
		return nullptr;
	}
	return alifEval_evalFrame(_tstate, frame, 0);
}
