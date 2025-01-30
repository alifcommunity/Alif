#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"
#include "AlifCore_StructSeq.h"
#include "AlifCore_SysModule.h"
#include "AlifCore_Traceback.h"





void _alifErr_setRaisedException(AlifThread* _thread, AlifObject* _exc) { // 24
	AlifObject* old_exc = _thread->currentException;
	_thread->currentException = _exc;
	ALIF_XDECREF(old_exc);
}


static AlifObject* _alifErr_createException(AlifObject* _exceptionType,
	AlifObject* _value) { // 32
	AlifObject* exc{};

	if (_value == nullptr or _value == ALIF_NONE) {
		exc = _alifObject_callNoArgs(_exceptionType);
	}
	else if (ALIFTUPLE_CHECK(_value)) {
		exc = alifObject_call(_exceptionType, _value, nullptr);
	}
	else {
		exc = alifObject_callOneArg(_exceptionType, _value);
	}

	if (exc != nullptr and !ALIFEXCEPTIONINSTANCE_CHECK(exc)) {
		alifErr_format(nullptr /*_alifExcTypeError_*/,
			"calling %R should have returned an instance of "
			"BaseException, not %s",
			_exceptionType, ALIF_TYPE(exc)->name);
		ALIF_CLEAR(exc);
	}

	return exc;
}


void _alifErr_restore(AlifThread* _thread, AlifObject* _type, AlifObject* _value,
	AlifObject* _traceback) { // 58
	if (_type == nullptr) {
		_alifErr_setRaisedException(_thread, nullptr);
		return;
	}
	else {
		AlifObject* exc = _alifErr_createException(_type, _value);
		ALIF_XDECREF(_value);
		if (exc == nullptr) {
			ALIF_DECREF(_type);
			ALIF_XDECREF(_traceback);
			return;
		}
		_value = exc;
	}

	if (_traceback != nullptr and !ALIFTRACEBACK_CHECK(_traceback)) {
		if (_traceback == ALIF_NONE) {
			ALIF_DECREF(ALIF_NONE);
			_traceback = nullptr;
		}
		else {
			//alifErr_setString(_alifExcTypeError_, "traceback must be a Traceback or None");
			ALIF_XDECREF(_value);
			ALIF_DECREF(_type);
			ALIF_XDECREF(_traceback);
			return;
		}
	}
	AlifObject* old_traceback = ((AlifBaseExceptionObject*)_value)->traceback;
	((AlifBaseExceptionObject*)_value)->traceback = _traceback;
	ALIF_XDECREF(old_traceback);
	_alifErr_setRaisedException(_thread, _value);
	ALIF_DECREF(_type);
}










AlifErrStackItem* _alifErr_getTopMostException(AlifThread* _thread) { // 118
	AlifErrStackItem* excInfo = _thread->excInfo;

	while (excInfo->excValue == nullptr
		and excInfo->previousItem != nullptr)
	{
		excInfo = excInfo->previousItem;
	}
	return excInfo;
}



static AlifObject* getNormalization_failureNote(AlifThread* tstate,
	AlifObject* exception, AlifObject* value) { // 132
	AlifObject* args = alifObject_repr(value);
	if (args == nullptr) {
		_alifErr_clear(tstate);
		args = alifUStr_fromFormat("<unknown>");
	}
	AlifObject* note{};
	const char* tpname = ((AlifTypeObject*)exception)->name;
	if (args == nullptr) {
		_alifErr_clear(tstate);
		note = alifUStr_fromFormat("Normalization failed: type=%s", tpname);
	}
	else {
		note = alifUStr_fromFormat("Normalization failed: type=%s args=%S",
			tpname, args);
		ALIF_DECREF(args);
	}
	return note;
}




void _alifErr_setObject(AlifThread* _thread,
	AlifObject* _exception, AlifObject* _value) { // 154
	AlifObject* excValue{};
	AlifObject* tb = nullptr;

	if (_exception != nullptr and
		not ALIFEXCEPTIONCLASS_CHECK(_exception)) {
		//_alifErr_format(_thread, _alifExcSystemError_,
		//	"_alifErr_setObject: "
		//	"exception %R is not a BaseException subclass",
		//	_exception);
		return;
	}
	/* Normalize the exception */
	AlifIntT isSubclass = 0;
	if (_value != nullptr and ALIFEXCEPTIONINSTANCE_CHECK(_value)) {
		isSubclass = alifObject_isSubclass((AlifObject*)ALIF_TYPE(_value), _exception);
		if (isSubclass < 0) {
			return;
		}
	}
	ALIF_XINCREF(_value);
	if (!isSubclass) {
		_alifErr_clear(_thread);

		AlifObject* fixed_value = _alifErr_createException(_exception, _value);
		if (fixed_value == nullptr) {
			AlifObject* exc = _alifErr_getRaisedException(_thread);

			AlifObject* note = getNormalization_failureNote(_thread, _exception, _value);
			ALIF_XDECREF(_value);
			if (note != nullptr) {
				_alifException_addNote(exc, note);
				ALIF_DECREF(note);
			}
			_alifErr_setRaisedException(_thread, exc);
			return;
		}
		ALIF_XSETREF(_value, fixed_value);
	}

	excValue = _alifErr_getTopMostException(_thread)->excValue;
	if (excValue != nullptr and excValue != ALIF_NONE) {
		ALIF_INCREF(excValue);
		if (excValue != _value) {
			AlifObject* o_ = excValue, * context;
			AlifObject* slowObj = o_; 
			AlifIntT slowUpdateToggle = 0;
			while ((context = alifException_getContext(o_))) {
				ALIF_DECREF(context);
				if (context == _value) {
					alifException_setContext(o_, nullptr);
					break;
				}
				o_ = context;
				if (o_ == slowObj) {
					break;
				}
				if (slowUpdateToggle) {
					slowObj = alifException_getContext(slowObj);
					ALIF_DECREF(slowObj);
				}
				slowUpdateToggle = not slowUpdateToggle;
			}
			alifException_setContext(_value, excValue);
		}
		else {
			ALIF_DECREF(excValue);
		}
	}
	if (ALIFEXCEPTIONINSTANCE_CHECK(_value))
		tb = alifException_getTraceback(_value);
	_alifErr_restore(_thread, ALIF_NEWREF(ALIF_TYPE(_value)), _value, tb);
}







void alifErr_setObject(AlifObject* _exception, AlifObject* _value) { // 246
	AlifThread* thread = _alifThread_get();
	_alifErr_setObject(thread, _exception, _value);
}


void _alifErr_setString(AlifThread* _thread, AlifObject* _exception,
	const char* _string) { // 285
	AlifObject* value = alifUStr_fromString(_string);
	if (value != nullptr) {
		_alifErr_setObject(_thread, _exception, value);
		ALIF_DECREF(value);
	}
}


void alifErr_setString(AlifObject* _exception, const char* _string) { // 296
	AlifThread* thread = _alifThread_get();
	_alifErr_setString(thread, _exception, _string);
}


AlifObject* ALIF_HOT_FUNCTION alifErr_occurred(void) { // 304
	AlifThread* thread = _alifThread_get();
	return _alifErr_occurred(thread);
}


AlifIntT alifErr_givenExceptionMatches(AlifObject* _err, AlifObject* _exc) { // 315
	if (_err == nullptr or _exc == nullptr) {
		return 0;
	}
	if (ALIFTUPLE_CHECK(_exc)) {
		AlifSizeT i{}, n{};
		n = alifTuple_size(_exc);
		for (i = 0; i < n; i++) {
			/* Test recursively */
			if (alifErr_givenExceptionMatches(
				_err, ALIFTUPLE_GET_ITEM(_exc, i)))
			{
				return 1;
			}
		}
		return 0;
	}
	/* err might be an instance, so check its class. */
	if (ALIFEXCEPTIONINSTANCE_CHECK(_err))
		_err = ALIFEXCEPTIONINSTANCE_CLASS(_err);

	if (ALIFEXCEPTIONCLASS_CHECK(_err) and ALIFEXCEPTIONCLASS_CHECK(_exc)) {
		return alifType_isSubType((AlifTypeObject*)_err, (AlifTypeObject*)_exc);
	}

	return _err == _exc;
}

AlifIntT _alifErr_exceptionMatches(AlifThread* _thread, AlifObject* _exc) { // 347
	return alifErr_givenExceptionMatches(_alifErr_occurred(_thread), _exc);
}


AlifIntT alifErr_exceptionMatches(AlifObject* _exc) { // 354
	AlifThread* thread = _alifThread_get();
	return _alifErr_exceptionMatches(thread, _exc);
}


AlifObject* _alifErr_getRaisedException(AlifThread* _thread) { // 483
	AlifObject* exc = _thread->currentException;
	_thread->currentException = nullptr;
	return exc;
}



void _alifErr_clear(AlifThread* _thread) { // 522
	_alifErr_restore(_thread, nullptr, nullptr, nullptr);
}






static AlifObject* _alifErr_formatV(AlifThread* _thread, AlifObject* _exception,
	const char* _format, va_list _vargs) { // 1152
	AlifObject* string{};

	_alifErr_clear(_thread);

	string = alifUStr_fromFormatV(_format, _vargs);
	if (string != nullptr) {
		_alifErr_setObject(_thread, _exception, string);
		ALIF_DECREF(string);
	}
	return nullptr;
}



AlifObject* _alifErr_format(AlifThread* _thread, AlifObject* _exception,
	const char* _format, ...) { // 1179
	va_list vargs{};
	va_start(vargs, _format);
	_alifErr_formatV(_thread, _exception, _format, vargs);
	va_end(vargs);
	return nullptr;
}


AlifObject* alifErr_format(AlifObject* _exception,
	const char* _format, ...) { // 1191
	AlifThread* thread = _alifThread_get();
	va_list vargs{};
	va_start(vargs, _format);
	_alifErr_formatV(thread, _exception, _format, vargs);
	va_end(vargs);
	return NULL;
}
