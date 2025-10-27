#include "alif.h"

#include "AlifCore_Frame.h"
#include "AlifCore_Function.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Compile.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_Errors.h"
#include "AlifCore_Runtime.h"
#include "AlifCore_SysModule.h"






static AlifObject* no_intrinsic1(AlifThread* _thread, AlifObject* _unused) { // 18
	_alifErr_setString(_thread, _alifExcSystemError_, "خطأ في وظيفة جوهرية");
	return nullptr;
}



static AlifObject* stopIteration_error(AlifThread* tstate, AlifObject* exc) { // 141
	AlifInterpreterFrame* frame = tstate->currentFrame;
	const char* msg = nullptr;
	if (alifErr_givenExceptionMatches(exc, _alifExcStopIteration_)) {
		msg = "generator raised StopIteration";
		if (_alifFrame_getCode(frame)->flags & CO_ASYNC_GENERATOR) {
			msg = "async generator raised StopIteration";
		}
		else if (_alifFrame_getCode(frame)->flags & CO_COROUTINE) {
			msg = "coroutine raised StopIteration";
		}
	}
	else if ((_alifFrame_getCode(frame)->flags & CO_ASYNC_GENERATOR) and
		alifErr_givenExceptionMatches(exc, _alifExcStopAsyncIteration_)) {
		/* code in `gen` raised a StopAsyncIteration error:
		raise a RuntimeError.
		*/
		msg = "async generator raised StopAsyncIteration";
	}
	if (msg != nullptr) {
		AlifObject* message = _alifUStr_fromASCII(msg, strlen(msg));
		if (message == nullptr) {
			return nullptr;
		}
		AlifObject* error = alifObject_callOneArg(_alifExcRuntimeError_, message);
		if (error == nullptr) {
			ALIF_DECREF(message);
			return nullptr;
		}
		alifException_setCause(error, ALIF_NEWREF(exc));
		alifException_setContext(error, ALIF_NEWREF(exc));
		ALIF_DECREF(message);
		return error;
	}
	return ALIF_NEWREF(exc);
}



static AlifObject* list_toTuple(AlifThread* unused, AlifObject* v) { // 191
	return _alifTuple_fromArray(((AlifListObject*)v)->item, ALIF_SIZE(v));
}






const IntrinsicFunc1Info _alifIntrinsicsUnaryFunctions_[] = { // 209
	{ no_intrinsic1, "INTRINSIC_1_INVALID" },
	{nullptr},//{print_expr, "INTRINSIC_PRINT"},
	{nullptr},//{import_star, "INTRINSIC_IMPORT_STAR"},
	{stopIteration_error, "INTRINSIC_STOPITERATION_ERROR"},
	{nullptr},//{_alif_asyncGenValueWrapperNew, "INTRINSIC_ASYNC_GEN_WRAP"},
	{nullptr},//{unary_pos, "INTRINSIC_UNARY_POSITIVE"},
	{list_toTuple, "INTRINSIC_LIST_TO_TUPLE"},
	//{make_typeVar, "INTRINSIC_TYPEVAR"},
	//{_alif_makeParamSpec, "INTRINSIC_PARAMSPEC"},
	//{_alif_makeTypeVarTuple, "INTRINSIC_TYPEVARTUPLE"},
	//{_alif_subScriptGeneric, "INTRINSIC_SUBSCRIPT_GENERIC"},
	//{ _alif_makeTypeAlias, "INTRINSIC_TYPEALIAS" },
};
