#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"






static inline AlifBaseExceptionObject* _alifBaseExceptionObject_cast(AlifObject* _exc) { // 228
	return (AlifBaseExceptionObject*)_exc;
}


static AlifObject* baseException_addNote(AlifObject* self, AlifObject* note) { // 235
	if (!ALIFUSTR_CHECK(note)) {
		alifErr_format(_alifExcTypeError_,
			"note must be a str, not '%s'",
			ALIF_TYPE(note)->name);
		return nullptr;
	}

	AlifObject* notes{};
	if (alifObject_getOptionalAttr(self, &ALIF_ID(__notes__), &notes) < 0) {
		return nullptr;
	}
	if (notes == nullptr) {
		notes = alifList_new(0);
		if (notes == nullptr) {
			return nullptr;
		}
		if (alifObject_setAttr(self, &ALIF_ID(__notes__), notes) < 0) {
			ALIF_DECREF(notes);
			return nullptr;
		}
	}
	else if (!ALIFLIST_CHECK(notes)) {
		ALIF_DECREF(notes);
		alifErr_setString(_alifExcTypeError_, "Cannot add note: __notes__ is not a list");
		return nullptr;
	}
	if (alifList_append(notes, note) < 0) {
		ALIF_DECREF(notes);
		return nullptr;
	}
	ALIF_DECREF(notes);
	return ALIF_NONE;
}





AlifObject* alifException_getTraceback(AlifObject* _self) { // 411
	AlifBaseExceptionObject* baseSelf = _alifBaseExceptionObject_cast(_self);
	return ALIF_XNEWREF(baseSelf->traceback);
}



AlifObject* alifException_getContext(AlifObject* _self) { // 441
	AlifObject* context = _alifBaseExceptionObject_cast(_self)->context;
	return ALIF_XNEWREF(context);
}


void alifException_setContext(AlifObject* _self, AlifObject* _context) { // 449
	ALIF_XSETREF(_alifBaseExceptionObject_cast(_self)->context, _context);
}
















AlifIntT _alifException_addNote(AlifObject* _exc, AlifObject* _note) { // 3866
	if (!ALIFEXCEPTIONINSTANCE_CHECK(_exc)) {
		alifErr_format(_alifExcTypeError_,
			"exc must be an exception, not '%s'",
			ALIF_TYPE(_exc)->name);
		return -1;
	}
	AlifObject* r = baseException_addNote(_exc, _note);
	AlifIntT res{};
	r == nullptr ? res = -1 : res = 0;
	ALIF_XDECREF(r);
	return res;
}
