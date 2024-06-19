#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_AlifEval.h"
#include "AlifCore_Code.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_AlifState.h"

#include "AlifCore_Frame.h"

#include "OpCode.h"


AlifObject* alifEval_evalCode(AlifObject* _co) { // 572
	AlifThread* thread = alifThread_get();
	//if (_locals == nullptr) {
	//	_locals = _globals;
	//}
	//AlifObject* builtins = alifEval_builtinsFromGlobals(thread, _globals);
	//if (builtins == nullptr) return nullptr;

	AlifFrameConstructor desc = {
		nullptr,
		nullptr,
		((AlifCodeObject*)_co)->name,
		((AlifCodeObject*)_co)->name,
		_co,
		nullptr,
		nullptr,
		nullptr
	};
	//AlifFunctionObject* func = alifFunction_fromConstructor(&desc);
	//if (func == nullptr) return nullptr;

	//AlifObject* res = alifEval_vector(thread, func, _locals, nullptr, 0, nullptr);
	//ALIF_DECREF(func);
	//return res;
	return nullptr; //
}




AlifObject* alifEval_vector(AlifThread* _thread, AlifFunctionObject* _func, AlifObject* _locals,
	AlifObject* const* _args, AlifUSizeT _argcount, AlifObject* _kwnames) { // 1793

	ALIF_INCREF(_func);
	ALIF_XINCREF(_locals);
	for (AlifUSizeT i = 0; i < _argcount; i++) {
		ALIF_INCREF(_args[i]);
	}
	if (_kwnames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwnames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIF_INCREF(_args[i + _argcount]);
		}
	}
	//AlifInterpreterFrame* frame = alifEvalFrameInitAndPush(
	//	_thread, _func, _locals, _args, _argcount, _kwnames);
	//if (frame == nullptr) return nullptr;

	//return alifEval_evalFrame(_thread, frame, 0);
	return nullptr; //
}
