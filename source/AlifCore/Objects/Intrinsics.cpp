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


static AlifIntT import_allFrom(AlifThread* _thread,
	AlifObject* _locals, AlifObject* _v) { // 38
	AlifObject* all{}, * dict{}, * name{}, * value{};
	AlifIntT skip_leading_underscores = 0;
	AlifIntT pos{}, err{};

	if (alifObject_getOptionalAttr(_v, &ALIF_STR(__all__), &all) < 0) {
		return -1; /* Unexpected error */
	}
	if (all == nullptr) {
		if (alifObject_getOptionalAttr(_v, &ALIF_STR(__dict__), &dict) < 0) {
			return -1;
		}
		if (dict == nullptr) {
			_alifErr_setString(_thread, _alifExcImportError_,
				"الكائن من-استورد-* لا يملك __فهرس__ ولا __كل__");
			return -1;
		}
		all = alifMapping_keys(dict);
		ALIF_DECREF(dict);
		if (all == nullptr)
			return -1;
		skip_leading_underscores = 1;
	}

	for (pos = 0, err = 0; ; pos++) {
		name = alifSequence_getItem(all, pos);
		if (name == nullptr) {
			if (!_alifErr_exceptionMatches(_thread, _alifExcIndexError_)) {
				err = -1;
			}
			else {
				_alifErr_clear(_thread);
			}
			break;
		}
		if (!ALIFUSTR_CHECK(name)) {
			AlifObject* modname = alifObject_getAttr(_v, &ALIF_STR(__name__));
			if (modname == nullptr) {
				ALIF_DECREF(name);
				err = -1;
				break;
			}
			if (!ALIFUSTR_CHECK(modname)) {
				_alifErr_format(_thread, _alifExcTypeError_,
					"الوحدة __اسم__ يجب أن تكون نص, وليس %.100s",
					ALIF_TYPE(modname)->name);
			}
			else {
				_alifErr_format(_thread, _alifExcTypeError_,
					"%s في %U.%s يجب أن تكون نص, وليس %.100s",
					skip_leading_underscores ? "مفتاح" : "عنصر",
					modname,
					skip_leading_underscores ? "__فهرس__" : "__كل__",
					ALIF_TYPE(name)->name);
			}
			ALIF_DECREF(modname);
			ALIF_DECREF(name);
			err = -1;
			break;
		}
		if (skip_leading_underscores) {
			if (ALIFUSTR_READ_CHAR(name, 0) == '_') {
				ALIF_DECREF(name);
				continue;
			}
		}
		value = alifObject_getAttr(_v, name);
		if (value == nullptr)
			err = -1;
		else if (ALIFDICT_CHECKEXACT(_locals))
			err = alifDict_setItem(_locals, name, value);
		else
			err = alifObject_setItem(_locals, name, value);
		ALIF_DECREF(name);
		ALIF_XDECREF(value);
		if (err < 0)
			break;
	}
	ALIF_DECREF(all);
	return err;
}

static AlifObject* import_star(AlifThread* _thread,
	AlifObject* _from) { // 122
	AlifInterpreterFrame* frame = _thread->currentFrame;

	AlifObject* locals = _alifFrame_getLocals(frame);
	if (locals == nullptr) {
		_alifErr_setString(_thread, _alifExcSystemError_,
			"لم يتم العثور على اسماء محلية اثناء 'استورد *'");
		return nullptr;
	}
	AlifIntT err = import_allFrom(_thread, locals, _from);
	ALIF_DECREF(locals);
	if (err < 0) {
		return nullptr;
	}
	return ALIF_NONE;
}


static AlifObject* stopIteration_error(AlifThread* tstate, AlifObject* exc) { // 141
	AlifInterpreterFrame* frame = tstate->currentFrame;
	const char* msg = nullptr;
	if (alifErr_givenExceptionMatches(exc, _alifExcStopIteration_)) {
		msg = "المولد اطلق خطأ_توقف_التكرار";
		if (_alifFrame_getCode(frame)->flags & CO_ASYNC_GENERATOR) {
			msg = "المولد المتزامن اطلق خطأ_توقف_التكرار";
		}
		else if (_alifFrame_getCode(frame)->flags & CO_COROUTINE) {
			msg = "الإجراء الفرعي اطلق خطأ_توقف_التكرار";
		}
	}
	else if ((_alifFrame_getCode(frame)->flags & CO_ASYNC_GENERATOR) and
		alifErr_givenExceptionMatches(exc, _alifExcStopAsyncIteration_)) {
		/* code in `gen` raised a StopAsyncIteration error:
		raise a RuntimeError.
		*/
		msg = "المولد المتزامن اطلق خطأ_توقف_التكرار_المتزامن";
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
	{import_star, "INTRINSIC_IMPORT_STAR"},
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
