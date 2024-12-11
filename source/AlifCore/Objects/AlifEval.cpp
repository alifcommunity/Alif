#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Backoff.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"

#include "AlifCore_Object.h"

#include "AlifCore_State.h"




















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
