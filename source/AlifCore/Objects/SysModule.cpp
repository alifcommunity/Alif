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
#include "AlifCore_StructSeq.h"
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
	AlifThread* thread = _alifThread_get();

	//AlifObject* exc_ = alifErr_getRaisedException(threadState);
	AlifObject* value = _alifSys_getObject(thread->interpreter, _name);

	if (_alifErr_occurred(thread)) {
		//alifErr_formatUnraisable("Exception ignored in alifSys_getObject()");
	}
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



static AlifTypeObject _flagsType_; // 3094

static AlifStructSequenceField _flagsFields_[] = {
	{"interactive",             "-i"},
	{"optimize",                "-O or -OO"},
	{"safePath",					 "-P"},
	{"intMaxStrDigits",      "-X int_max_str_digits"},
	{0}
};

#define SYS_FLAGS_INT_MAX_STR_DIGITS 17

static AlifStructSequenceDesc _flagsDesc_ = {
	.name = "sys.flags",        /* name */
	.fields = _flagsFields_,       /* fields */
	.nInSequence = 4,
};


static void sys_setFlag(AlifObject* _flags, AlifSizeT _pos, AlifObject* _value) { // 3128
	AlifObject* oldValue = ALIFTUPLE_GET_ITEM(_flags, _pos); //* alif
	ALIFSTRUCTSEQUENCE_SET_ITEM(_flags, _pos, ALIF_NEWREF(_value));
	ALIF_XDECREF(oldValue);
}



static AlifIntT setFlags_fromConfig(AlifInterpreter* _interp, AlifObject* _flags) { // 3170
	//const AlifPreConfig* preconfig = &_interp->dureRun->preConfig;
	const AlifConfig* config = alifInterpreter_getConfig(_interp);

	AlifSizeT pos = 0;
#define SetFlagObj(expr) \
    do { \
        AlifObject *value = (expr); \
        if (value == nullptr) { \
            return -1; \
        } \
        sys_setFlag(_flags, pos, value); \
        ALIF_DECREF(value); \
        pos++; \
    } while (0)
#define SetFlag(expr) SetFlagObj(alifLong_fromLong(expr))


	SetFlag(config->interactive);
	SetFlag(config->optimizationLevel);
	//SetFlag(!config->writeBytecode);
	//SetFlag(!config->userSiteDirectory);
	//SetFlag(!config->siteImport);
	//SetFlag(!config->useEnvironment);
	//SetFlag(config->useHashSeed == 0 or config->hashSeed != 0);
	//SetFlag(config->isolated);
	//SetFlag(config->utf8Mode);
	SetFlagObj(alifBool_fromLong(config->safePath));
	SetFlag(config->intMaxStrDigits);

#undef SetFlagObj
#undef SetFlag
	return 0;
}

static AlifObject* make_flags(AlifInterpreter* interp) { // 3225
	AlifObject* flags = alifStructSequence_new(&_flagsType_);
	if (flags == nullptr) {
		return nullptr;
	}

	if (setFlags_fromConfig(interp, flags) < 0) {
		ALIF_DECREF(flags);
		return nullptr;
	}
	return flags;
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


#define ENSURE_INFO_TYPE(_type, _desc) \
    do { \
        if (_alifStructSequence_initBuiltinWithFlags( \
                interp, &_type, &_desc, ALIF_TPFLAGS_DISALLOW_INSTANTIATION) < 0) { \
            goto type_init_failed; \
        } \
    } while (0)

	ENSURE_INFO_TYPE(_flagsType_, _flagsDesc_);
	SET_SYS("flags", make_flags(tstate->interpreter));


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



AlifIntT _alifSys_updateConfig(AlifThread* _thread) { // 3645
	AlifInterpreter* interp = _thread->interpreter;
	AlifObject* sysdict = interp->sysDict;

	AlifObject* flags{}; //* alif
	const wchar_t* stdlibdir{}; //* alif

	const AlifConfig* config = alifInterpreter_getConfig(interp);
	AlifIntT res{};

#define COPY_LIST(_key, _value) \
        SET_SYS(_key, _alifWStringList_asList(&(_value)));

#define SET_SYS_FROM_WSTR(_key, _value) \
        SET_SYS(_key, alifUStr_fromWideChar(_value, -1));

#define COPY_WSTR(_sysAttr, _wstr) \
    if (_wstr != nullptr) { \
        SET_SYS_FROM_WSTR(_sysAttr, _wstr); \
    }

	if (config->moduleSearchPathsSet) {
		COPY_LIST("path", config->moduleSearchPaths);
	}

	//COPY_WSTR("executable", config->executable);
	//COPY_WSTR("_base_executable", config->baseExecutable);
	//COPY_WSTR("prefix", config->prefix);
	//COPY_WSTR("base_prefix", config->basePrefix);
	//COPY_WSTR("exec_prefix", config->execPrefix);
	//COPY_WSTR("base_exec_prefix", config->baseExecPrefix);
	//COPY_WSTR("platlibdir", config->platlibdir);

	//if (config->alifCachePrefix != nullptr) {
	//	SET_SYS_FROM_WSTR("alifCachePrefix", config->alifCachePrefix);
	//}
	//else {
	//	if (alifDict_setItemString(sysdict, "alifCachePrefix", ALIF_NONE) < 0) {
	//		return -1;
	//	}
	//}

	COPY_LIST("argv", config->argv);
	COPY_LIST("origArgv", config->origArgv);
	//COPY_LIST("warnoptions", config->warnOptions);


	//stdlibdir = _alif_getStdLibDir();
	if (stdlibdir != nullptr) {
		SET_SYS_FROM_WSTR("_stdlibDir", stdlibdir);
	}
	else {
		if (alifDict_setItemString(sysdict, "_stdlibDir", ALIF_NONE) < 0) {
			return -1;
		}
	}

#undef SET_SYS_FROM_WSTR
#undef COPY_LIST
#undef COPY_WSTR

	// sys.flags
	flags = _alifSys_getObject(interp, "flags"); // borrowed ref
	if (flags == nullptr) {
		if (!_alifErr_occurred(_thread)) {
			//_alifErr_setString(_thread, _alifExcRuntimeError_, "lost sys.flags");
		}
		return -1;
	}
	if (setFlags_fromConfig(interp, flags) < 0) {
		return -1;
	}

	if (_alifErr_occurred(_thread)) {
		goto err_occurred;
	}

	return 0;

err_occurred:
	return -1;
}

#undef SET_SYS
#undef SET_SYS_FROM_STRING


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
