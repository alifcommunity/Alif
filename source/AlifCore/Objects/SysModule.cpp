#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_LifeCycle.h"
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
	AlifThread* threadState = _alifThread_get();

	//AlifObject* exc_ = alifErr_getRaisedException(threadState);
	AlifObject* value = _alifSys_getObject(threadState->interpreter, _name);

	//if (alifErr_occurred(threadState)) {
		//alifErr_formatUnraisable("Exception ignored in alifSys_getObject()");
	//}
	//alifErr_setRaisedException(threadState, exc_);
	return value;
}















AlifIntT alifSys_create(AlifThread* _thread, AlifObject** _sysModP) { // 3779

	AlifIntT status{};
	AlifIntT err{};
	AlifObject* sysmod{};
	AlifObject* sysdict{};
	AlifObject* monitoring{};
	AlifInterpreter* interp = _thread->interpreter;

	AlifObject* modules = alifImport_initModules(interp);
	if (modules == nullptr) {
		goto error;
	}

//	sysmod = alifModule_createInitialized(&_sysModule_, ALIF_API_VERSION);
//	if (sysmod == nullptr) {
//		//return ALIFSTATUS_ERR("failed to create a module object");
//		return -1; // temp
//	}
//#ifdef ALIF_GIL_DISABLED
//	alifUnstableModule_setGIL(sysmod, ALIF_MOD_GIL_NOT_USED);
//#endif
//
//	sysdict = alifModule_getDict(sysmod);
//	if (sysdict == nullptr) {
//		goto error;
//	}
//	interp->sysDict = ALIF_NEWREF(sysdict);
//
//	interp->sysDictCopy = alifDict_copy(sysdict);
//	if (interp->sysDictCopy == nullptr) {
//		goto error;
//	}
//
//	if (alifDict_setItemString(sysdict, "modules", modules) < 0) {
//		goto error;
//	}
//
//	status = _alifSys_setPreliminaryStderr(sysdict);
//	if (status < 1) {
//		return status;
//	}
//
//	status = _alifSys_initCore(_thread, sysdict);
//	if (status < 1) {
//		return status;
//	}
//
//	if (_alifImport_fixupBuiltin(_thread, sysmod, "sys", modules) < 0) {
//		goto error;
//	}
//
//	monitoring = _alif_createMonitoringObject();
//	if (monitoring == nullptr) {
//		goto error;
//	}
//	err = alifDict_setItemString(sysdict, "monitoring", monitoring);
//	ALIF_DECREF(monitoring);
//	if (err < 0) {
//		goto error;
//	}
//
//
//	*_sysModP = sysmod;
	return 1;

error:
	//return ALIFSTATUS_ERR("can't initialize sys module");
	return -1; // temp
}
