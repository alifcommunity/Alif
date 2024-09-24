#include "alif.h"

#include "AlifCore_State.h"



static AlifObject* _alifSys_getObject(AlifInterpreter* _interp, const char* _name) { // 89
	AlifObject* sysDict = _interp->sysDict;
	if (sysDict == nullptr) {
		return nullptr;
	}
	AlifObject* value;
	if (alifDict_getItemStringRef(sysDict, _name, &value) != 1) {
		return nullptr;
	}
	ALIF_DECREF(value);  // return a borrowed reference
	return value;
}


AlifObject* alifSys_getObject(const char* _name) { // 104
	AlifThread* threadState = alifThread_get();

	//AlifObject* exc_ = alifErr_getRaisedException(threadState);
	AlifObject* value = _alifSys_getObject(threadState->interpreter, _name);

	//if (alifErr_occurred(threadState)) {
		//alifErr_formatUnraisable("Exception ignored in alifSys_getObject()");
	//}
	//alifErr_setRaisedException(threadState, exc_);
	return value;
}
