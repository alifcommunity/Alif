#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"
#include "AlifCore_StructSeq.h"
#include "AlifCore_SysModule.h"
#include "AlifCore_Traceback.h"












void _alifErr_setObject(AlifThread* tstate, AlifObject* exception, AlifObject* value) { // 154
	AlifObject* excValue{};
	AlifObject* tb = nullptr;

	if (exception != nullptr and
		!alifExceptionClass_check(exception)) {
		_alifErr_format(tstate, _alifExcSystemError_,
			"_alifErr_setObject: "
			"exception %R is not a BaseException subclass",
			exception);
		return;
	}
	/* Normalize the exception */
	AlifIntT isSubclass = 0;
	if (value != nullptr and alifExceptionInstance_check(value)) {
		isSubclass = alifObject_isSubclass((AlifObject*)ALIF_TYPE(value), exception);
		if (isSubclass < 0) {
			return;
		}
	}
	ALIF_XINCREF(value);
	if (!isSubclass) {
		_alifErr_clear(tstate);

		AlifObject* fixed_value = _alifErr_createException(exception, value);
		if (fixed_value == nullptr) {
			AlifObject* exc = _alifErr_getRaisedException(tstate);

			AlifObject* note = getNormalization_failureNote(tstate, exception, value);
			ALIF_XDECREF(value);
			if (note != nullptr) {
				_alifException_addNote(exc, note);
				ALIF_DECREF(note);
			}
			_alifErr_setRaisedException(tstate, exc);
			return;
		}
		ALIF_XSETREF(value, fixed_value);
	}

	excValue = _alifErr_getTopmostException(tstate)->excValue;
	if (excValue != nullptr && excValue != ALIF_NONE) {
		ALIF_INCREF(excValue);
		if (excValue != value) {
			AlifObject* o = excValue, * context;
			AlifObject* slow_o = o; 
			int slow_update_toggle = 0;
			while ((context = alifException_getContext(o))) {
				ALIF_DECREF(context);
				if (context == value) {
					alifException_setContext(o, nullptr);
					break;
				}
				o = context;
				if (o == slow_o) {
					break;
				}
				if (slow_update_toggle) {
					slow_o = alifException_getContext(slow_o);
					ALIF_DECREF(slow_o);
				}
				slow_update_toggle = !slow_update_toggle;
			}
			alifException_setContext(value, excValue);
		}
		else {
			ALIF_DECREF(excValue);
		}
	}
	if (alifExceptionInstance_check(value))
		tb = alifException_getTraceback(value);
	_alifErr_restore(tstate, ALIF_NEWREF(ALIF_TYPE(value)), value, tb);
}







void alifErr_setObject(AlifObject* _exception, AlifObject* _value) { // 246
	AlifThread* thread = _alifThread_get();
	_alifErr_setObject(thread, _exception, _value);
}
