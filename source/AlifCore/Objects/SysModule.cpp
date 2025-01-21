#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Dict.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_SysModule.h"





AlifObject* _alifSys_getAttr(AlifThread* _thread, AlifObject* _name) { // 73
	AlifObject* sd = _thread->interpreter->sysDict;
	if (sd == nullptr) {
		return nullptr;
	}
	//AlifObject* exc = _alifErr_getRaisedException(_thread);
	AlifObject* value = _alifDict_getItemWithError(sd, _name);
	//_alifErr_setRaisedException(_thread, exc);
	return value;
}


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




static AlifObject* listBuiltin_moduleNames(void) { // 2628
	AlifObject* tuple{};

	AlifObject* list = _alifImport_getBuiltinModuleNames();
	if (list == nullptr) {
		return nullptr;
	}
	if (alifList_sort(list) != 0) {
		goto error;
	}
	tuple = alifList_asTuple(list);
	ALIF_DECREF(list);
	return tuple;

error:
	ALIF_DECREF(list);
	return nullptr;
}





static AlifModuleDef _sysModule_ = { // 3447
	ALIFMODULEDEF_HEAD_INIT,
	"sys",
	0, //_sysDoc_
	-1, /* multiple "initialization" just copies the module dict. */
	0, //_sysMethods_,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};


 // 3460
#define SET_SYS(key, value)                                \
    do {                                                   \
        AlifObject *v = (value);                             \
        if (v == nullptr) {                                   \
            goto err_occurred;                             \
        }                                                  \
        res = alifDict_setItemString(sysdict, key, v);       \
        ALIF_DECREF(v);                                      \
        if (res < 0) {                                     \
            goto err_occurred;                             \
        }                                                  \
    } while (0)

#define SET_SYS_FROM_STRING(_key, _value) \
        SET_SYS(_key, alifUStr_fromString(_value))

static AlifIntT _alifSys_initCore(AlifThread* tstate, AlifObject* sysdict) { // 3476
	AlifObject* version_info{};
	AlifIntT res{};
	AlifInterpreter* interp = tstate->interpreter;

#undef COPY_SYS_ATTR

	SET_SYS_FROM_STRING("version", alif_getVersion());
	/* initialize hash_info */

	SET_SYS("builtin_module_names", listBuiltin_moduleNames()); //* review
	//SET_SYS("stdlib_module_names", listStdlib_moduleNames());
#if ALIF_BIG_ENDIAN
	SET_SYS_FROM_STRING("byteorder", "big");
#else
	SET_SYS_FROM_STRING("byteorder", "little");
#endif

	/* adding sys.path_hooks and sys.path_importer_cache */
	SET_SYS("meta_path", alifList_new(0));
	SET_SYS("path_importer_cache", alifDict_new());
	SET_SYS("path_hooks", alifList_new(0));

	if (_alifErr_occurred(tstate)) {
		goto err_occurred;
	}
	return 1;

type_init_failed:
	//return _ALIFSTATUS_ERR("failed to initialize a type");
	return -1; //* alif

err_occurred:
	//return _ALIFSTATUS_ERR("can't initialize sys module");
	return -1; //* alif
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

	sysmod = alifModule_createInitialized(&_sysModule_/*, ALIF_API_VERSION*/); //* alif
	if (sysmod == nullptr) {
		//return ALIFSTATUS_ERR("failed to create a module object");
		return -1; // temp
	}
	//alifUnstableModule_setGIL(sysmod, ALIF_MOD_GIL_NOT_USED);

	sysdict = alifModule_getDict(sysmod);
	if (sysdict == nullptr) {
		goto error;
	}
	interp->sysDict = ALIF_NEWREF(sysdict);

	//interp->sysDictCopy = alifDict_copy(sysdict);
	//if (interp->sysDictCopy == nullptr) {
	//	goto error;
	//}

	if (alifDict_setItemString(sysdict, "modules", modules) < 0) {
		goto error;
	}

	//status = _alifSys_setPreliminaryStderr(sysdict);
	//if (status < 1) {
	//	return status;
	//}

	status = _alifSys_initCore(_thread, sysdict);
	if (status < 1) {
		return status;
	}

	//if (_alifImport_fixupBuiltin(_thread, sysmod, "sys", modules) < 0) {
	//	goto error;
	//}

//	monitoring = _alif_createMonitoringObject();
//	if (monitoring == nullptr) {
//		goto error;
//	}
//	err = alifDict_setItemString(sysdict, "monitoring", monitoring);
//	ALIF_DECREF(monitoring);
//	if (err < 0) {
//		goto error;
//	}


	*_sysModP = sysmod;
	return 1;

error:
	//return ALIFSTATUS_ERR("can't initialize sys module");
	return -1; //* alif
}
