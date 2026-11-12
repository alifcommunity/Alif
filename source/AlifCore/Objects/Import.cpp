#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Import.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"
#include "AlifCore_Errors.h"
#include "AlifCore_Namespace.h"
#include "AlifCore_Object.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_SysModule.h"


#include "Marshal.h"
#include "AlifCore_ImportDL.h"





#include "clinic/Import.cpp.h"


extern InitTable _alifImportInitTab_[]; // 55

InitTable* _alifImportInitTable_ = _alifImportInitTab_; // 59

#define INITTABLE _alifRuntime_.imports.initTable // 69
#define LAST_MODULE_INDEX _alifRuntime_.imports.lastModuleIndex // 70
#define EXTENSIONS _alifRuntime_.imports.extensions // 71

// 80
#define MODULES(_interp) \
    (_interp)->imports.modules
#define MODULES_BY_INDEX(interp) \
    (interp)->imports.modulesByIndex
#define IMPORTLIB(_interp) \
    (_interp)->imports.importLib
#define OVERRIDE_MULTI_INTERP_EXTENSIONS_CHECK(_interp) \
    (_interp)->imports.overrideMultiInterpExtensionsCheck
#ifdef HAVE_DLOPEN
#  define DLOPENFLAGS(interp) \
        (interp)->imports.dlopenflags
#endif
#define IMPORT_FUNC(_interp) \
    (_interp)->imports.importFunc
// 94




AlifObject* alifImport_initModules(AlifInterpreter* _interp) { // 129
	MODULES(_interp) = alifDict_new();
	if (MODULES(_interp) == nullptr) {
		return nullptr;
	}
	return MODULES(_interp);
}


AlifObject* _alifImport_getModules(AlifInterpreter* _interp) { // 140
	return MODULES(_interp);
}


static inline AlifObject* get_modulesDict(AlifThread* tstate, bool fatal) { // 152
	AlifObject* modules = MODULES(tstate->interpreter);
	if (modules == nullptr) {
		if (fatal) {
			alif_fatalError("المفسر لا يحتوي على فهرس مكتبات");
		}
		_alifErr_setString(tstate, _alifExcRuntimeError_, "لم يتمكن من الحصول على النظام.مكتبات");
		return nullptr;
	}
	return modules;
}


AlifIntT _alifImport_setModuleString(const char* name, AlifObject* m) { // 187
	AlifThread* thread = _alifThread_get();
	AlifObject* modules = get_modulesDict(thread, true);
	return alifMapping_setItemString(modules, name, m);
}

static AlifObject* import_getModule(AlifThread* _thread, AlifObject* _name) { // 195
	AlifObject* modules = get_modulesDict(_thread, false);
	if (modules == nullptr) {
		return nullptr;
	}

	AlifObject* m{};
	ALIF_INCREF(modules);
	(void)alifMapping_getOptionalItem(modules, _name, &m);
	ALIF_DECREF(modules);
	return m;
}


AlifObject* alifImport_getModule(AlifObject* _name) { // 248
	AlifThread* thread = _alifThread_get();
	AlifObject* mod{};

	mod = import_getModule(thread, _name);
	if (mod != nullptr and mod != ALIF_NONE) {
		//if (import_ensureInitialized(thread->interpreter, mod, _name) < 0) {
		//	ALIF_DECREF(mod);
		//	remove_importLibFrames(thread);
		//	return nullptr;
		//}
	}
	return mod;
}


static AlifObject* import_addModule(AlifThread* _thread,
	AlifObject* _name) { // 261

	AlifObject* modules_ = get_modulesDict(_thread, false);
	if (modules_ == nullptr) {
		return nullptr;
	}

	AlifObject* m_{};
	if (alifMapping_getOptionalItem(modules_, _name, &m_) < 0) {
		return nullptr;
	}
	if (m_ != nullptr and ALIFMODULE_CHECK(m_)) {
		return m_;
	}
	ALIF_XDECREF(m_);
	m_ = alifModule_newObject(_name);
	if (m_ == nullptr) return nullptr;

	if (alifObject_setItem(modules_, _name, m_) != 0) {
		ALIF_DECREF(m_);
		return nullptr;
	}

	return m_;
}


AlifObject* alifImport_addModuleRef(const char* _name) { // 288
	AlifObject* nameObj = alifUStr_fromString(_name);
	if (nameObj == nullptr) {
		return nullptr;
	}
	AlifThread* thread = _alifThread_get();
	AlifObject* module = import_addModule(thread, nameObj);
	ALIF_DECREF(nameObj);
	return module;
}

static void remove_module(AlifThread* _thread, AlifObject* _name) { // 357
	AlifObject* exc = _alifErr_getRaisedException(_thread);

	AlifObject* modules = get_modulesDict(_thread, true);
	if (ALIFDICT_CHECKEXACT(modules)) {
		// Error is reported to the caller
		(void)alifDict_pop(modules, _name, nullptr);
	}
	else if (ALIFMAPPING_DELITEM(modules, _name) < 0) {
		if (_alifErr_exceptionMatches(_thread, _alifExcKeyError_)) {
			_alifErr_clear(_thread);
		}
	}

	_alifErr_chainExceptions1(exc);
}


AlifSizeT alifImport_getNextModuleIndex() { // 381
	return alifAtomic_addSize(&LAST_MODULE_INDEX, 1) + 1;
}

static void _set_moduleIndex(AlifModuleDef* def, AlifSizeT index) { // 404
	if (index == def->base.index) {
		/* There's nothing to do. */
	}
	else if (def->base.index == 0) {
		def->base.index = index;
	}
	else {
		def->base.index = index;
	}
}

static AlifIntT _modulesByIndex_set(AlifInterpreter* interp,
	AlifSizeT index, AlifObject* module) { // 450
	if (MODULES_BY_INDEX(interp) == nullptr) {
		MODULES_BY_INDEX(interp) = alifList_new(0);
		if (MODULES_BY_INDEX(interp) == nullptr) {
			return -1;
		}
	}

	while (ALIFLIST_GET_SIZE(MODULES_BY_INDEX(interp)) <= index) {
		if (alifList_append(MODULES_BY_INDEX(interp), ALIF_NONE) < 0) {
			return -1;
		}
	}

	return alifList_setItem(MODULES_BY_INDEX(interp), index, ALIF_NEWREF(module));
}





#ifdef HAVE_LOCAL_THREAD
ALIF_LOCAL_THREAD const char* _pkgcontext_ = nullptr;
# undef PKGCONTEXT
# define PKGCONTEXT _pkgcontext_
#endif

const char* alifImport_resolveNameWithPackageContext(const char* name) { // 740
#ifndef HAVE_LOCAL_THREAD
	alifThread_acquireLock(EXTENSIONS.mutex, WAIT_LOCK);
#endif
	if (PKGCONTEXT != nullptr) {
		const char* p = strrchr(PKGCONTEXT, '.');
		if (p != nullptr and strcmp(name, p + 1) == 0) {
			name = PKGCONTEXT;
			PKGCONTEXT = nullptr;
		}
	}
#ifndef HAVE_LOCAL_THREAD
	alifThread_releaseLock(EXTENSIONS.mutex);
#endif
	return name;
}

const char* _alifImport_swapPackageContext(const char* newcontext) { // 759
#ifndef HAVE_LOCAL_THREAD
	alifThread_acquireLock(EXTENSIONS.mutex, WAIT_LOCK);
#endif
	const char* oldcontext = PKGCONTEXT;
	PKGCONTEXT = newcontext;
#ifndef HAVE_LOCAL_THREAD
	alifThread_releaseLock(EXTENSIONS.mutex);
#endif
	return oldcontext;
}

// 781
#ifdef HAVE_DLOPEN
AlifIntT _alifImport_getDLOpenFlags(AlifInterpreter* _interp) {
	return DLOPENFLAGS(_interp);
}

void _alifImport_setDLOpenFlags(AlifInterpreter* _interp,
	AlifIntT _newVal) {
	DLOPENFLAGS(_interp) = _newVal;
}
#endif  // HAVE_DLOPEN

static AlifIntT exec_builtinOrDynamic(AlifObject* mod) { // 790
	AlifModuleDef* def{};
	void* state{};

	if (!ALIFMODULE_CHECK(mod)) {
		return 0;
	}

	def = alifModule_getDef(mod);
	if (def == nullptr) {
		return 0;
	}

	state = alifModule_getState(mod);
	if (state) {
		/* Already initialized; skip reload */
		return 0;
	}

	return alifModule_execDef(mod, def);
}



static inline void extensions_lockAcquire(void) { // 914
	ALIFMUTEX_LOCK(&_alifRuntime_.imports.extensions.mutex);
}

static inline void extensions_lockRelease(void) { // 920
	ALIFMUTEX_UNLOCK(&_alifRuntime_.imports.extensions.mutex);
}



typedef class CachedMDict { // 935
public:
	AlifObject* copied{};
	int64_t interpid{};
} *CachedMDictT;

class ExtensionsCacheValue { // 942
public:
	AlifModuleDef* def{};
	AlifModInitFunction init{};
	AlifSizeT index{};
	CachedMDictT dict{};
	CachedMDict mDict{};
	AlifExtModuleOrigin origin{};
	void* gil{};
};



static ExtensionsCacheValue* allocExtensions_cacheValue(void) { // 973
	ExtensionsCacheValue* value
		= (ExtensionsCacheValue*)alifMem_dataAlloc(sizeof(ExtensionsCacheValue));
	if (value == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	*value = { 0 };
	return value;
}


static void freeExtensions_cacheValue(ExtensionsCacheValue* _value) { // 986
	alifMem_dataFree(_value);
}

static AlifSizeT _getCached_moduleIndex(ExtensionsCacheValue* _cached) { // 992
	return _cached->index;
}


static void fixup_cachedDef(ExtensionsCacheValue* value) { // 999
	AlifModuleDef* def = value->def;

	_alif_setImmortalUntracked((AlifObject*)def);

	def->base.init = value->init;

	_set_moduleIndex(def, value->index);

	if (value->dict != nullptr) {
		def->base.copy = ALIF_NEWREF(value->dict->copied);
	}
}


static void restore_oldCachedDef(AlifModuleDef* def, AlifModuleDefBase* oldbase) { // 1035
	def->base = *oldbase;
}

static void cleanup_oldCachedDef(AlifModuleDefBase* oldbase) { // 1041
	ALIF_XDECREF(oldbase->copy);
}

static void del_cachedDef(ExtensionsCacheValue* _value) { // 1047
	ALIF_XDECREF(_value->def->base.copy);
	_value->def->base.copy = nullptr;
}

static AlifIntT init_cachedMDict(ExtensionsCacheValue* value, AlifObject* m_dict) { // 1060
	if (m_dict == nullptr) {
		return 0;
	}
	AlifInterpreter* interp = _alifInterpreter_get();

	AlifObject* copied = alifDict_copy(m_dict);
	if (copied == nullptr) {
		return -1;
	}

	value->mDict = {
		.copied = copied,
		.interpid = alifInterpreter_getID(interp),
	};

	value->dict = &value->mDict;
	return 0;
}

static void del_cachedMDict(ExtensionsCacheValue* _value) { // 1105
	if (_value->dict != nullptr) {
		ALIF_XDECREF(_value->dict->copied);
		_value->dict = nullptr;
	}
}

static AlifObject* getCore_moduleDict(AlifInterpreter*, AlifObject*, AlifObject*); // 1126

static AlifObject* get_cachedMDict(ExtensionsCacheValue* value,
	AlifObject* name, AlifObject* path) { // 1129
	AlifInterpreter* interp = _alifInterpreter_get();
	if (value->origin == AlifExtModuleOrigin::Alif_Ext_Module_Origin_CORE) {
		return getCore_moduleDict(interp, name, path);
	}
	AlifObject* dict = value->def->base.copy;
	ALIF_XINCREF(dict);
	return dict;
}

static void delExtensions_cacheValue(ExtensionsCacheValue* _value) { // 1139
	if (_value != nullptr) {
		del_cachedMDict(_value);
		del_cachedDef(_value);
		freeExtensions_cacheValue(_value);
	}
}

static void* hashtableKey_from2Strings(AlifObject* _str1, AlifObject* _str2, const char _sep) { // 1149
	AlifSizeT str1_len{}, str2_len{};
	const char* str1_data = alifUStr_asUTF8AndSize(_str1, &str1_len);
	const char* str2_data = alifUStr_asUTF8AndSize(_str2, &str2_len);
	if (str1_data == nullptr or str2_data == nullptr) {
		return nullptr;
	}
	/* Make sure sep and the nullptr byte won't cause an overflow. */
	AlifUSizeT size = str1_len + 1 + str2_len + 1;

	// XXX Use a buffer if it's a temp value (every case but "set").
	char* key = (char*)alifMem_dataAlloc(size);
	if (key == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}

	strncpy(key, str1_data, str1_len);
	key[str1_len] = _sep;
	strncpy(key + str1_len + 1, str2_data, str2_len + 1);
	return key;
}


static AlifUHashT hashtable_hashStr(const void* _key) { // 1176
	return alif_hashBuffer(_key, strlen((const char*)_key));
}

static AlifIntT hashtable_compareStr(const void* _key1, const void* _key2) { // 1182
	return strcmp((const char*)_key1, (const char*)_key2) == 0;
}

static void hashtable_destroyStr(void* _ptr) { // 1188
	alifMem_dataFree(_ptr);
}


#define HTSEP ':' // 1229

static AlifIntT _extensions_cacheInit(void) { // 1231
	AlifHashTableAllocatorT alloc = { alifMem_dataAlloc, alifMem_dataFree };
	EXTENSIONS.hashtable = _alifHashTable_newFull(
		hashtable_hashStr,
		hashtable_compareStr,
		hashtable_destroyStr,  // key
		(AlifHashTableDestroyFunc)delExtensions_cacheValue,  // value
		&alloc
	);
	if (EXTENSIONS.hashtable == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	return 0;
}

static AlifHashTableEntryT* _extensionsCache_findUnlocked(AlifObject* _path,
	AlifObject* _name, void** _pKey) { // 1249
	if (EXTENSIONS.hashtable == nullptr) {
		return nullptr;
	}
	void* key = hashtableKey_from2Strings(_path, _name, HTSEP);
	if (key == nullptr) {
		return nullptr;
	}
	AlifHashTableEntryT* entry = _alifHashTable_getEntry(EXTENSIONS.hashtable, key);
	if (_pKey != nullptr) {
		*_pKey = key;
	}
	else {
		hashtable_destroyStr(key);
	}
	return entry;
}

static ExtensionsCacheValue* _extensions_cacheGet(AlifObject* _path, AlifObject* _name) { // 1272
	ExtensionsCacheValue* value = nullptr;
	extensions_lockAcquire();

	AlifHashTableEntryT* entry =
		_extensionsCache_findUnlocked(_path, _name, nullptr);
	if (entry == nullptr) {
		/* It was never added. */
		goto finally;
	}
	value = (ExtensionsCacheValue*)entry->value;

	finally:
	extensions_lockRelease();
	return value;
}

static ExtensionsCacheValue* _extensions_cacheSet(AlifObject* path,
	AlifObject* name, AlifModuleDef* def, AlifModInitFunction m_init, AlifSizeT m_index,
	AlifObject* m_dict, AlifExtModuleOrigin origin, void* md_gil) { // 1292
	ExtensionsCacheValue* value = nullptr;
	void* key = nullptr;
	ExtensionsCacheValue* newvalue = nullptr;
	AlifModuleDefBase olddefbase = def->base;

	extensions_lockAcquire();

	if (EXTENSIONS.hashtable == nullptr) {
		if (_extensions_cacheInit() < 0) {
			goto finally;
		}
	}

	/* Create a cached value to populate for the module. */
	AlifHashTableEntryT* entry;
	entry = _extensionsCache_findUnlocked(path, name, &key);
	value = entry == nullptr
		? nullptr
		: (ExtensionsCacheValue*)entry->value;
	if (value != nullptr) {
		goto finally_oldvalue;
	}
	newvalue = allocExtensions_cacheValue();
	if (newvalue == nullptr) {
		goto finally;
	}

	/* Populate the new cache value data. */
	*newvalue = {
		.def = def,
		.init = m_init,
		.index = m_index,
		.origin = origin,
		.gil = md_gil,
	};

	if (init_cachedMDict(newvalue, m_dict) < 0) {
		goto finally;
	}
	fixup_cachedDef(newvalue);

	if (entry == nullptr) {
		/* It was never added. */
		if (_alifHashTable_set(EXTENSIONS.hashtable, key, newvalue) < 0) {
			//alifErr_noMemory();
			goto finally;
		}
		/* The hashtable owns the key now. */
		key = nullptr;
	}
	else if (value == nullptr) {
		/* It was previously deleted. */
		entry->value = newvalue;
	}
	else {
		/* This shouldn't ever happen. */
		ALIF_UNREACHABLE();
	}

	value = newvalue;

	finally:
	if (value == nullptr) {
		restore_oldCachedDef(def, &olddefbase);
		if (newvalue != nullptr) {
			delExtensions_cacheValue(newvalue);
		}
	}
	else {
		cleanup_oldCachedDef(&olddefbase);
	}

finally_oldvalue:
	extensions_lockRelease();
	if (key != nullptr) {
		hashtable_destroyStr(key);
	}

	return value;
}



static void _extensions_cacheDelete(AlifObject* path, AlifObject* name) { // 1418
	extensions_lockAcquire();

	if (EXTENSIONS.hashtable == nullptr) {
		goto finally;
	}

	AlifHashTableEntryT* entry;
	entry = _extensionsCache_findUnlocked(path, name, nullptr);
	if (entry == nullptr) {
		/* It was never added. */
		goto finally;
	}
	if (entry->value == nullptr) {
		/* It was already removed. */
		goto finally;
	}
	ExtensionsCacheValue* value;
	value = (ExtensionsCacheValue*)entry->value;
	entry->value = nullptr;

	delExtensions_cacheValue(value);

	finally:
	extensions_lockRelease();
}


static bool check_multiInterpExtensions(AlifInterpreter* interp) { // 1459
	AlifIntT override = OVERRIDE_MULTI_INTERP_EXTENSIONS_CHECK(interp);
	if (override < 0) {
		return false;
	}
	else if (override > 0) {
		return true;
	}
	else if (_alifInterpreterState_hasFeature(
		interp, ALIF_RTFLAGS_MULTI_INTERP_EXTENSIONS)) {
		return true;
	}
	return false;
}

AlifIntT _alifImport_checkSubinterpIncompatibleExtensionAllowed(const char* _name) { // 1476
	AlifInterpreter* interp = _alifInterpreter_get();
	if (check_multiInterpExtensions(interp)) {
		alifErr_format(_alifExcImportError_,
			"الوحدة %s لا تدعم التحميل ضمن مفسرات فرعية",
			_name);
		return -1;
	}
	return 0;
}



static AlifThread* switchTo_mainInterpreter(AlifThread* _thread) { // 1523
	if (alif_isMainInterpreter(_thread->interpreter)) {
		return _thread;
	}

	AlifThread* mainThread = _alifThreadState_newBound(
		_alifInterpreterState_main()/*, _ALIFTHREADSTATE_WHENCE_EXEC*/);
	if (mainThread == nullptr) {
		return nullptr;
	}
	(void)alifThreadState_swap(mainThread);
	return mainThread;
}


static AlifObject* getCore_moduleDict(AlifInterpreter* _interp,
	AlifObject* _name, AlifObject* _path) { // 1580
	/* Only builtin modules are core. */
	if (_path == _name) {
		if (alifUStr_compareWithASCIIString(_name, "النظام") == 0) {
			return ALIF_NEWREF(_interp->sysdictCopy);
		}
		if (alifUStr_compareWithASCIIString(_name, "الضمنيات") == 0) {
			return ALIF_NEWREF(_interp->builtinsCopy);
		}
	}
	return nullptr;
}

static void switchBackFrom_mainInterpreter(AlifThread* _thread,
	AlifThread* _mainThread, AlifObject* _tempObj) { // 1551
	if (alifErr_occurred()) {
		//if (alifErr_exceptionMatches(_alifExcMemoryError_)) {
		//	alifErr_clear();
		//}
		//else {
		//	alifErr_printEx(0);
		//}
		printf("حالة غير مكتملة");
	}

	ALIF_XDECREF(_tempObj);

	alifThreadState_clear(_mainThread);
	(void)alifThreadState_swap(_thread);
	alifThreadState_delete(_mainThread);
}

class SinglephaseGlobalUpdate { // 1666
public:
	AlifModInitFunction init{};
	AlifSizeT index{};
	AlifObject* dict{};
	AlifExtModuleOrigin origin{};
	void* gil{};
};

static ExtensionsCacheValue* updateGlobalState_forExtension(AlifThread* tstate,
	AlifObject* path, AlifObject* name, AlifModuleDef* def,
	SinglephaseGlobalUpdate* singlephase) { // 1674
	ExtensionsCacheValue* cached = nullptr;
	AlifModInitFunction m_init = nullptr;
	AlifObject* m_dict = nullptr;

	/* Set up for _extensions_cacheSet(). */
	if (singlephase == nullptr) {
		// nothing for now
	}
	else {
		if (singlephase->init != nullptr) {
			m_init = singlephase->init;
		}
		else if (singlephase->dict == nullptr) {
			/* It must be a core builtin module. */
		}
		else {
			m_dict = singlephase->dict;
		}
	}

	if (alif_isMainInterpreter(tstate->interpreter) or def->size == -1) {
		cached = _extensions_cacheSet(
			path, name, def, m_init, singlephase->index, m_dict,
			singlephase->origin, singlephase->gil);
		if (cached == nullptr) {
			return nullptr;
		}
	}

	return cached;
}



static AlifIntT finish_singlephaseExtension(AlifThread* tstate,
	AlifObject* mod, ExtensionsCacheValue* cached,
	AlifObject* name, AlifObject* modules) { // 1743
	AlifSizeT index = _getCached_moduleIndex(cached);
	if (_modulesByIndex_set(tstate->interpreter, index, mod) < 0) {
		return -1;
	}

	if (modules != nullptr) {
		if (alifObject_setItem(modules, name, mod) < 0) {
			return -1;
		}
	}

	return 0;
}



static AlifObject* reload_singlephaseExtension(AlifThread* _thread,
	ExtensionsCacheValue* cached, AlifExtModuleLoaderInfo* info) { // 1774
	AlifModuleDef* def = cached->def;
	AlifObject* mod = nullptr;

	const char* name_buf = alifUStr_asUTF8(info->name);
	if (_alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
		return nullptr;
	}

	AlifObject* modules = get_modulesDict(_thread, true);
	if (def->size == -1) {
		AlifObject* m_copy = get_cachedMDict(cached, info->name, info->path);
		if (m_copy == nullptr) {
			return nullptr;
		}
		mod = import_addModule(_thread, info->name);
		if (mod == nullptr) {
			ALIF_DECREF(m_copy);
			return nullptr;
		}
		AlifObject* mdict = alifModule_getDict(mod);
		if (mdict == nullptr) {
			ALIF_DECREF(m_copy);
			ALIF_DECREF(mod);
			return nullptr;
		}
		AlifIntT rc = alifDict_update(mdict, m_copy);
		ALIF_DECREF(m_copy);
		if (rc < 0) {
			ALIF_DECREF(mod);
			return nullptr;
		}
		if (def->base.copy != nullptr) {
			((AlifModuleObject*)mod)->gil = cached->gil;
		}
	}
	else {
		AlifModInitFunction p0 = def->base.init;
		if (p0 == nullptr) {
			return nullptr;
		}
		AlifExtModuleLoaderResult res{};
		if (_alifImport_runModInitFunc(p0, info, &res) < 0) {
			//_alifExtModule_loaderResultApplyError(&res, name_buf);
			return nullptr;
		}
		mod = res.module;
		_alifExtModule_loaderResultClear(&res);

		if (info->filename != nullptr) {
			if (alifModule_addObjectRef(mod, "__ملف__", info->filename) < 0) {
				alifErr_clear(); /* Not important enough to report */
			}
		}

		if (alifObject_setItem(modules, info->name, mod) == -1) {
			ALIF_DECREF(mod);
			return nullptr;
		}
	}

	AlifSizeT index = _getCached_moduleIndex(cached);
	if (_modulesByIndex_set(_thread->interpreter, index, mod) < 0) {
		ALIFMAPPING_DELITEM(modules, info->name);
		ALIF_DECREF(mod);
		return nullptr;
	}

	return mod;
}



static AlifObject* import_findExtension(AlifThread* tstate,
	AlifExtModuleLoaderInfo* info, ExtensionsCacheValue** p_cached) { // 1885
	/* Only single-phase init modules will be in the cache. */
	ExtensionsCacheValue* cached
		= _extensions_cacheGet(info->path, info->name);
	if (cached == nullptr) {
		return nullptr;
	}

	*p_cached = cached;

	const char* name_buf = alifUStr_asUTF8(info->name);
	if (_alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
		return nullptr;
	}

	AlifObject* mod = reload_singlephaseExtension(tstate, cached, info);
	if (mod == nullptr) {
		return nullptr;
	}

	//AlifIntT verbose = _alifInterpreterState_getConfig(tstate->interpreter)->verbose;
	//if (verbose) {
	//	alifSys_formatStderr("import %U # previously loaded (%R)\n",
	//		info->name, info->path);
	//}

	return mod;
}


static AlifObject* import_runExtension(AlifThread* tstate, AlifModInitFunction p0,
	AlifExtModuleLoaderInfo* info, AlifObject* spec, AlifObject* modules) { // 1919

	AlifObject* mod = nullptr;
	AlifModuleDef* def = nullptr;
	ExtensionsCacheValue* cached = nullptr;
	const char* name_buf = ALIFBYTES_AS_STRING(info->nameEncoded);

	bool switched = false;

	AlifThread* main_tstate = switchTo_mainInterpreter(tstate);
	if (main_tstate == nullptr) {
		return nullptr;
	}
	else if (main_tstate != tstate) {
		switched = true;
	}

	AlifExtModuleLoaderResult res{};
	AlifIntT rc = _alifImport_runModInitFunc(p0, info, &res);
	if (rc < 0) {
		/* We discard res.def. */
	}
	else {
		mod = res.module;
		res.module = nullptr;
		def = res.def;

		if (res.kind == AlifExtModuleKind::Alif_Ext_Module_Kind_SINGLEPHASE) {
			if (info->filename != nullptr) {
				AlifObject* filename = nullptr;
				if (switched) {
					filename = _alifUStr_copy(info->filename);
					if (filename == nullptr) {
						return nullptr;
					}
				}
				else {
					filename = ALIF_NEWREF(info->filename);
				}
				AlifInterpreter* interp = _alifInterpreter_get();
				_alifUStr_internImmortal(interp, &filename);

				if (alifModule_addObjectRef(mod, "__file__", filename) < 0) {
					alifErr_clear(); /* Not important enough to report */
				}
			}

			SinglephaseGlobalUpdate singlephase = {
				.index = def->base.index,
				.origin = info->origin,
				.gil = ((AlifModuleObject*)mod)->gil,
			};
			if (def->size == -1) {
				singlephase.dict = alifModule_getDict(mod);
			}
			else {
				singlephase.init = p0;
			}
			cached = updateGlobalState_forExtension(
				main_tstate, info->path, info->name, def, &singlephase);
			if (cached == nullptr) {
				goto main_finally;
			}
		}
	}

main_finally:
	if (switched) {
		switchBackFrom_mainInterpreter(tstate, main_tstate, mod);
		mod = nullptr;
	}

	/*****************************************************************/
	/* At this point we are back to the interpreter we started with. */
	/*****************************************************************/

	if (rc < 0) {
		//alifExtModule_loaderResultApplyError(&res, name_buf);
		goto error;
	}

	if (res.kind == AlifExtModuleKind::Alif_Ext_Module_Kind_MULTIPHASE) {
		mod = ALIFMODULE_FROMDEFANDSPEC(def, spec);
		if (mod == nullptr) {
			goto error;
		}
	}
	else {
		//if (_alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
		//	goto error;
		//}

		if (switched) {
			mod = reload_singlephaseExtension(tstate, cached, info);
			if (mod == nullptr) {
				goto error;
			}
		}
		else {
			AlifObject* modules = get_modulesDict(tstate, true);
			if (finish_singlephaseExtension(
				tstate, mod, cached, info->name, modules) < 0) {
				goto error;
			}
		}
	}

	_alifExtModule_loaderResultClear(&res);
	return mod;

error:
	ALIF_XDECREF(mod);
	_alifExtModule_loaderResultClear(&res);
	return nullptr;
}



AlifIntT _alifImport_fixupBuiltin(AlifThread* _thread, AlifObject* _mod,
	const char* _name, AlifObject* _modules) { // 2188
	AlifIntT res = -1;

	AlifObject* nameobj{};
	nameobj = alifUStr_internFromString(_name);
	if (nameobj == nullptr) {
		return -1;
	}

	AlifModuleDef* def = alifModule_getDef(_mod);
	if (def == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		goto finally;
	}

	ExtensionsCacheValue* cached;
	cached = _extensions_cacheGet(nameobj, nameobj);
	if (cached == nullptr) {
		SinglephaseGlobalUpdate singlephase = {
			.index = def->base.index,
			.dict = nullptr,
			.origin = AlifExtModuleOrigin::Alif_Ext_Module_Origin_CORE,
			.gil = nullptr,
		};
		cached = updateGlobalState_forExtension(
			_thread, nameobj, nameobj, def, &singlephase);
		if (cached == nullptr) {
			goto finally;
		}
	}

	if (finish_singlephaseExtension(_thread, _mod, cached, nameobj, _modules) < 0) {
		goto finally;
	}

	res = 0;

	finally:
	ALIF_DECREF(nameobj);
	return res;
}


static AlifIntT is_builtin(AlifObject* _name) { // 2254
	AlifIntT i{};
	InitTable* inittab = INITTABLE;
	for (i = 0; inittab[i].name != nullptr; i++) {
		//if (_alifUStr_equalToASCIIString(_name, inittab[i].name)) {
		if (alifUStr_equalToUTF8(_name, inittab[i].name)) { //* alif
			if (inittab[i].initFunc == nullptr)
				return -1;
			else
				return 1;
		}
	}
	return 0;
}


static AlifObject* create_builtin(AlifThread* _thread,
	AlifObject* _name, AlifObject* _spec) { // 2270
	AlifExtModuleLoaderInfo info{};
	InitTable* found = nullptr; //* alif
	AlifModInitFunction p0{}; //* alif


	if (_alifExtModule_loaderInfoInitForBuiltin(&info, _name) < 0) {
		return nullptr;
	}

	ExtensionsCacheValue* cached = nullptr;
	AlifObject* mod = import_findExtension(_thread, &info, &cached);
	if (mod != nullptr) {
		goto finally;
	}
	else if (_alifErr_occurred(_thread)) {
		goto finally;
	}

	if (cached != nullptr) {
		_extensions_cacheDelete(info.path, info.name);
	}


	for (InitTable* p = INITTABLE; p->name != nullptr; p++) {
		//if (_alifUStr_equalToASCIIString(info.name, p->name)) {
		if (alifUStr_equalToUTF8(info.name, p->name)) { //* alif
			found = p;
		}
	}
	if (found == nullptr) {
		// not found
		mod = ALIF_NEWREF(ALIF_NONE);
		goto finally;
	}

	p0 = (AlifModInitFunction)found->initFunc;
	if (p0 == nullptr) {
		mod = import_addModule(_thread, info.name);
		goto finally;
	}

	//_alifEval_enableGILTransient(_thread);

	/* Now load it. */
	mod = import_runExtension(
		_thread, p0, &info, _spec, get_modulesDict(_thread, true));

	//if (_alifImport_checkGILForModule(mod, info.name) < 0) {
	//	ALIF_CLEAR(mod);
	//	goto finally;
	//}

	finally:
	_alifExtModule_loaderInfoClear(&info);
	return mod;
}



static AlifIntT initBuildin_modulesTable() { // 2419

	AlifUSizeT size_{};
	for (size_ = 0; _alifImportInitTable_[size_].name != nullptr; size_++)
		;
	size_++;

	InitTable* tableCopy = (InitTable*)alifMem_dataAlloc(size_ * sizeof(InitTable));
	if (tableCopy == nullptr) return -1;

	memcpy(tableCopy, _alifImportInitTable_, size_ * sizeof(InitTable));
	INITTABLE = tableCopy;
	return 0;
}

AlifObject* _alifImport_getBuiltinModuleNames(void) { // 2445
	AlifObject* list = alifList_new(0);
	if (list == nullptr) {
		return nullptr;
	}
	InitTable* inittab = INITTABLE;
	for (AlifSizeT i = 0; inittab[i].name != nullptr; i++) {
		AlifObject* name = alifUStr_fromString(inittab[i].name);
		if (name == nullptr) {
			ALIF_DECREF(list);
			return nullptr;
		}
		if (alifList_append(list, name) < 0) {
			ALIF_DECREF(name);
			ALIF_DECREF(list);
			return nullptr;
		}
		ALIF_DECREF(name);
	}
	return list;
}


static AlifObject* moduleDict_forExec(AlifThread* _thread, AlifObject* _name) { // 2580
	AlifObject* m{}, * d{};

	m = import_addModule(_thread, _name);
	if (m == nullptr)
		return nullptr;
	/* If the module is being reloaded, we get the old module back
	   and re-use its dict to exec the new code. */
	d = alifModule_getDict(m);
	AlifIntT r = alifDict_contains(d, &ALIF_ID(__builtins__));
	if (r == 0) {
		r = alifDict_setItem(d, &ALIF_ID(__builtins__), alifEval_getBuiltins());
	}
	if (r < 0) {
		remove_module(_thread, _name);
		ALIF_DECREF(m);
		return nullptr;
	}

	ALIF_INCREF(d);
	ALIF_DECREF(m);
	return d;
}

static AlifObject* execCode_inModule(AlifThread* _thread, AlifObject* _name,
	AlifObject* _moduleDict, AlifObject* _codeObject) { // 2606
	AlifObject* v{}, * m{};

	v = alifEval_evalCode(_codeObject, _moduleDict, _moduleDict);
	if (v == nullptr) {
		remove_module(_thread, _name);
		return nullptr;
	}
	ALIF_DECREF(v);

	m = import_getModule(_thread, _name);
	if (m == nullptr and !_alifErr_occurred(_thread)) {
		_alifErr_format(_thread, _alifExcImportError_,
			"تحميل مكتبة %R غير موجودة في النظام.الوحدات",
			_name);
	}

	return m;
}


static bool resolve_moduleAlias(const char* _name,
	const ModuleAlias* _aliases, const char** _alias) { // 2713
	const ModuleAlias* entry{};
	for (entry = _aliases; ; entry++) {
		if (entry->name == nullptr) {
			/* It isn't an alias. */
			return false;
		}
		if (strcmp(_name, entry->name) == 0) {
			if (_alias != nullptr) {
				*_alias = entry->orig;
			}
			return true;
		}
	}
}



enum FrozenStatus { // 2820
	Frozen_Okay,
	Frozen_Bad_Name,
	Frozen_Not_Found,
	Frozen_Disabled,
	Frozen_Excluded,

	Frozen_Invalid,
};


static const Frozen* lookup_frozen(const char* name) { // 2865
	const Frozen* p{};
	for (p = _alifImportFrozenBootstrap_; ; p++) {
		if (p->name == nullptr) {
			// We hit the end-of-list sentinel value.
			break;
		}
		if (strcmp(name, p->name) == 0) {
			return p;
		}
	}

	// Prefer custom modules, if any.  Frozen stdlib modules can be
	// disabled here by setting "code" to nullptr in the array entry.
	//if (_alifImportFrozenModules_ != nullptr) { 
	//	for (p = _alifImportFrozenModules_; ; p++) {
	//		if (p->name == nullptr) {
	//			break;
	//		}
	//		if (strcmp(name, p->name) == 0) {
	//			return p;
	//		}
	//	}
	//}
	//// Frozen stdlib modules may be disabled.
	//if (use_frozen()) {
	//	for (p = _alifImportFrozenStdlib_; ; p++) {
	//		if (p->name == nullptr) {
	//			break;
	//		}
	//		if (strcmp(name, p->name) == 0) {
	//			return p;
	//		}
	//	}
	//	for (p = _alifImportFrozenTest_; ; p++) {
	//		if (p->name == nullptr) {
	//			break;
	//		}
	//		if (strcmp(name, p->name) == 0) {
	//			return p;
	//		}
	//	}
	//}
	return nullptr;
}


class FrozenInfo { // 2913
public:
	AlifObject* nameobj{};
	const char* data{};
	AlifSizeT size{};
	bool isPackage{};
	bool isAlias{};
	const char* origname{};
};


static FrozenStatus find_frozen(AlifObject* nameobj, FrozenInfo* info) { // 2922
	if (info != nullptr) {
		memset(info, 0, sizeof(*info));
	}

	if (nameobj == nullptr or nameobj == ALIF_NONE) {
		return FrozenStatus::Frozen_Bad_Name;
	}
	const char* name = alifUStr_asUTF8(nameobj);
	if (name == nullptr) {
		alifErr_clear();
		return FrozenStatus::Frozen_Bad_Name;
	}

	const Frozen* p = lookup_frozen(name);
	if (p == nullptr) {
		return FrozenStatus::Frozen_Not_Found;
	}
	if (info != nullptr) {
		info->nameobj = nameobj;  // borrowed
		info->data = (const char*)p->code;
		info->size = p->size;
		info->isPackage = p->isPackage;
		if (p->size < 0) {
			// backward compatibility with negative size values
			info->size = -(p->size);
			info->isPackage = true;
		}
		info->origname = name;
		info->isAlias = resolve_moduleAlias(name, _alifImportFrozenAliases_,
			&info->origname);
	}
	if (p->code == nullptr) {
		/* It is frozen but marked as un-importable. */
		return FrozenStatus::Frozen_Excluded;
	}
	if (p->code[0] == '\0' or p->size == 0) {
		/* Does not contain executable code. */
		return FrozenStatus::Frozen_Invalid;
	}
	return FrozenStatus::Frozen_Okay;
}

static AlifObject* unmarshal_frozenCode(AlifInterpreter* _interp,
	FrozenInfo* _info) { // 2971
	AlifObject* co = alifMarshal_readObjectFromString(_info->data, _info->size);
	if (co == nullptr) {
		/* Does not contain executable code. */
		alifErr_clear();
		//set_frozenError(FROZEN_INVALID, _info->nameobj);
		return nullptr;
	}
	if (!ALIFCODE_CHECK(co)) {
		// We stick with TypeError for backward compatibility.
		//alifErr_format(_alifExcTypeError_,
		//	"frozen object %R is not a code object",
		//	_info->nameobj);
		ALIF_DECREF(co);
		return nullptr;
	}
	return co;
}

AlifIntT alifImport_importFrozenModuleObject(AlifObject* name) { // 2998
	AlifThread* tstate = _alifThread_get();
	AlifObject* co{}, * m{}, * d = nullptr;
	AlifIntT err{};

	FrozenInfo info{};
	FrozenStatus status = find_frozen(name, &info);
	if (status == FrozenStatus::Frozen_Not_Found or status == FrozenStatus::Frozen_Disabled) {
		return 0;
	}
	else if (status == FrozenStatus::Frozen_Bad_Name) {
		return 0;
	}
	else if (status != FrozenStatus::Frozen_Okay) {
		//set_frozenError(status, name);
		return -1;
	}
	co = unmarshal_frozenCode(tstate->interpreter, &info);
	if (co == nullptr) {
		return -1;
	}
	if (info.isPackage) {
		/* Set __path__ to the empty list */
		AlifObject* l{};
		m = import_addModule(tstate, name);
		if (m == nullptr)
			goto err_return;
		d = alifModule_getDict(m);
		l = alifList_new(0);
		if (l == nullptr) {
			ALIF_DECREF(m);
			goto err_return;
		}
		err = alifDict_setItemString(d, "__مسار__", l);
		ALIF_DECREF(l);
		ALIF_DECREF(m);
		if (err != 0)
			goto err_return;
	}
	d = moduleDict_forExec(tstate, name);
	if (d == nullptr) {
		goto err_return;
	}
	m = execCode_inModule(tstate, name, d, co);
	if (m == nullptr) {
		goto err_return;
	}
	ALIF_DECREF(m);
	/* Set __origname__ . */
	AlifObject* origname;
	if (info.origname) {
		origname = alifUStr_fromString(info.origname);
		if (origname == nullptr) {
			goto err_return;
		}
	}
	else {
		origname = ALIF_NEWREF(ALIF_NONE);
	}
	err = alifDict_setItemString(d, "__الاسم_الاصل__", origname); // __origname__
	ALIF_DECREF(origname);
	if (err != 0) {
		goto err_return;
	}
	ALIF_DECREF(d);
	ALIF_DECREF(co);
	return 1;

err_return:
	ALIF_XDECREF(d);
	ALIF_DECREF(co);
	return -1;
}


AlifIntT alifImport_importFrozenModule(const char* _name) { // 3074
	AlifObject* nameobj{};
	AlifIntT ret{};
	nameobj = alifUStr_internFromString(_name);
	if (nameobj == nullptr)
		return -1;
	ret = alifImport_importFrozenModuleObject(nameobj);
	ALIF_DECREF(nameobj);
	return ret;
}


static AlifObject* bootstrap_imp(AlifThread* _thread) { // 3096
	AlifObject* spec{}; //* alif
	AlifObject* mod{}; //* alif

	AlifObject* name = alifUStr_fromString("_ورد");
	if (name == nullptr) {
		return nullptr;
	}

	AlifObject* attrs = alif_buildValue("{sO}", "اسم", name);
	if (attrs == nullptr) {
		goto error;
	}
	spec = alifNamespace_new(attrs);
	ALIF_DECREF(attrs);
	if (spec == nullptr) {
		goto error;
	}

	// Create the _imp module from its definition.
	mod = create_builtin(_thread, name, spec);
	ALIF_CLEAR(name);
	ALIF_DECREF(spec);
	if (mod == nullptr) {
		goto error;
	}

	// Execute the _imp module: call imp_module_exec().
	if (exec_builtinOrDynamic(mod) < 0) {
		ALIF_DECREF(mod);
		goto error;
	}
	return mod;

error:
	ALIF_XDECREF(name);
	return nullptr;
}


static AlifIntT init_importLib(AlifThread* tstate, AlifObject* sysmod) { // 3150

	AlifInterpreter* interp = tstate->interpreter;
	AlifIntT verbose = _alifInterpreterState_getConfig(interp)->verbose;

	// استورد مكتبة_الاستيراد بنسختها المصرفة, _مكتبة_استيراد_مصرفة.
	//if (verbose) {
	//	alifSys_formatStderr("استورد _مكتبة_استيراد_مصرفة # مصرفة\n");
	//}
	if (alifImport_importFrozenModule("_مكتبة_استيراد_مصرفة") <= 0) { // _frozen_importlib
		return -1;
	}

	AlifObject* importlib = alifImport_addModuleRef("_مكتبة_استيراد_مصرفة");
	if (importlib == nullptr) {
		return -1;
	}
	IMPORTLIB(interp) = importlib;

	// Import the _imp module

	AlifObject* impMod = bootstrap_imp(tstate);
	if (impMod == nullptr) {
		return -1;
	}
	if (_alifImport_setModuleString("_ورد", impMod) < 0) {
		ALIF_DECREF(impMod);
		return -1;
	}

	// Install importlib as the implementation of import
	AlifObject* value = alifObject_callMethod(importlib, "_ثبت",
		"OO", sysmod, impMod);
	ALIF_DECREF(impMod);
	if (value == nullptr) {
		return -1;
	}
	ALIF_DECREF(value);

	return 0;
}

static AlifIntT init_importlibExternal(AlifInterpreter* _interp) { // 3207
	AlifObject* value{};
	value = alifObject_callMethod(IMPORTLIB(_interp),
		"_ثبت_مستوردات_الخارج", "");
	if (value == nullptr) {
		return -1;
	}
	ALIF_DECREF(value);
	return 0;
}

AlifIntT _alifImport_initDefaultImportFunc(AlifInterpreter* _interp) { // 3338
	// Get the __import__ function
	AlifObject* importFunc{};
	if (alifDict_getItemStringRef(_interp->builtins, "__استورد__", &importFunc) <= 0) {
		return -1;
	}
	IMPORT_FUNC(_interp) = importFunc;
	return 0;
}

AlifIntT _alifImport_isDefaultImportFunc(AlifInterpreter* _interp,
	AlifObject* _func) { // 3350
	return _func == IMPORT_FUNC(_interp);
}

/* Import a module, either built-in, frozen, or external, and return
its module object WITH INCREMENTED REFERENCE COUNT */

AlifObject* alifImport_importModule(const char* _name) { // 3376
	AlifObject* pname{};
	AlifObject* result{};

	pname = alifUStr_fromString(_name);
	if (pname == nullptr)
		return nullptr;
	result = alifImport_import(pname);
	ALIF_DECREF(pname);
	return result;
}


static void remove_importLibFrames(AlifThread* _thread) { // 3415
	const char *importlibFilename = "<مصرف Importlib._bootstrap>";
	const char *externalFilename = "<مصرف Importlib._bootstrap_external>";
	const char *removeFrames = "_استدعي_بدون_إطار";
	AlifIntT alwaysTrim = 0;
	AlifIntT inImportlib = 0;
	AlifObject** prevLink, **outerLink = nullptr;
	AlifObject* baseTB = nullptr;

	/* Synopsis: if it's an ImportError, we trim all importlib chunks
	from the traceback. We always trim chunks
	which end with a call to "_استدعي_بدون_إطار". */

	AlifObject *exc = _alifErr_getRaisedException(_thread);
	if (exc == nullptr or _alifInterpreterState_getConfig(_thread->interpreter)->verbose) {
		goto done;
	}

	if (alifType_isSubType(ALIF_TYPE(exc), (AlifTypeObject*)_alifExcImportError_)) {
		alwaysTrim = 1;
	}

	baseTB = alifException_getTraceback(exc);
	prevLink = &baseTB;
	AlifObject* tb; tb = baseTB;
	while (tb != nullptr) {
		AlifTracebackObject *traceback = (AlifTracebackObject*)tb;
		AlifObject *next = (AlifObject*)traceback->next;
		AlifFrameObject *frame = traceback->frame;
		AlifCodeObject *code = alifFrame_getCode(frame);
		AlifIntT nowInImportlib{};

		//* alif
		// here we use alifUStr_equalToUTF8 instade of _alifUStr_equalToASCIIString
		// because the name contains unicode "Arabic" letters
		nowInImportlib = alifUStr_equalToUTF8(code->filename, importlibFilename)
			or
			alifUStr_equalToUTF8(code->filename, externalFilename);
		if (nowInImportlib and !inImportlib) {
			/* This is the link to this chunk of importlib tracebacks */
			outerLink = prevLink;
		}
		inImportlib = nowInImportlib;

		if (inImportlib and
			(alwaysTrim or
				alifUStr_equalToUTF8(code->name, removeFrames))) {
			ALIF_XSETREF(*outerLink, ALIF_XNEWREF(next));
			prevLink = outerLink;
		}
		else {
			prevLink = (AlifObject**) &traceback->next;
		}
		ALIF_DECREF(code);
		tb = next;
	}
	if (baseTB == nullptr) {
		baseTB = ALIF_NONE;
		ALIF_INCREF(ALIF_NONE);
	}
	alifException_setTraceback(exc, baseTB);
done:
	ALIF_XDECREF(baseTB);
	_alifErr_setRaisedException(_thread, exc);
}


static AlifObject* resolve_name(AlifThread* _thread,
	AlifObject* _name, AlifObject* _globals, AlifIntT _level) { // 3482
	AlifObject* absName{};
	AlifObject* package{};
	AlifObject* spec{};
	AlifSizeT last_dot{};
	AlifObject* base{};
	AlifIntT level_up{};

	if (_globals == nullptr) {
		_alifErr_setString(_thread, _alifExcKeyError_, "'__اسم__' ليس في اسماء_عامة");
		goto error;
	}
	if (!ALIFDICT_CHECK(_globals)) {
		_alifErr_setString(_thread, _alifExcTypeError_, "اسماء_عامة يجب أن تكون فهرس");
		goto error;
	}
	if (alifDict_getItemRef(_globals, &ALIF_STR(__package__), &package) < 0) {
		goto error;
	}
	if (package == ALIF_NONE) {
		ALIF_DECREF(package);
		package = nullptr;
	}
	if (alifDict_getItemRef(_globals, &ALIF_STR(__spec__), &spec) < 0) {
		goto error;
	}

	if (package != nullptr) {
		if (!ALIFUSTR_CHECK(package)) {
			_alifErr_setString(_thread, _alifExcTypeError_,
				"متغير الحزمة يجب أن يكون نص");
			goto error;
		}
		else if (spec != nullptr and spec != ALIF_NONE) {
			AlifIntT equal{};
			AlifObject* parent = alifObject_getAttr(spec, &ALIF_STR(Parent));
			if (parent == nullptr) {
				goto error;
			}

			equal = alifObject_richCompareBool(package, parent, ALIF_EQ);
			ALIF_DECREF(parent);
			if (equal < 0) {
				goto error;
			}
			else if (equal == 0) {
				//if (alifErr_warnEx(_alifExcDeprecationWarning_,
				//	"__package__ != __spec__.parent", 1) < 0) {
				//	goto error;
				//}
			}
		}
	}
	else if (spec != nullptr and spec != ALIF_NONE) {
		package = alifObject_getAttr(spec, &ALIF_STR(Parent));
		if (package == nullptr) {
			goto error;
		}
		else if (!ALIFUSTR_CHECK(package)) {
			_alifErr_setString(_thread, _alifExcTypeError_,
				"__خصائص__.الوالد يجب أن تكون نص");
			goto error;
		}
	}
	else {
		//if (alifErr_warnEx(_alifExcImportWarning_,
		//	"can't resolve package from __spec__ or __package__, "
		//	"falling back on __name__ and __path__", 1) < 0) {
		//	goto error;
		//}

		if (alifDict_getItemRef(_globals, &ALIF_STR(__name__), &package) < 0) {
			goto error;
		}
		if (package == nullptr) {
			_alifErr_setString(_thread, _alifExcKeyError_,
				"'__اسم__' ليس في الاسماء العامة");
			goto error;
		}

		if (!ALIFUSTR_CHECK(package)) {
			_alifErr_setString(_thread, _alifExcTypeError_,
				"__اسم__ يجب أن يكون نص");
			goto error;
		}

		AlifIntT haspath = alifDict_contains(_globals, &ALIF_STR(__path__));
		if (haspath < 0) {
			goto error;
		}
		if (!haspath) {
			AlifSizeT dot{};

			dot = alifUStr_findChar(package, '.',
				0, ALIFUSTR_GET_LENGTH(package), -1);
			if (dot == -2) {
				goto error;
			}
			else if (dot == -1) {
				goto no_parent_error;
			}
			AlifObject* substr = alifUStr_subString(package, 0, dot);
			if (substr == nullptr) {
				goto error;
			}
			ALIF_SETREF(package, substr);
		}
	}

	last_dot = ALIFUSTR_GET_LENGTH(package);
	if (last_dot == 0) {
		goto no_parent_error;
	}

	for (level_up = 1; level_up < _level; level_up += 1) {
		last_dot = alifUStr_findChar(package, '.', 0, last_dot, -1);
		if (last_dot == -2) {
			goto error;
		}
		else if (last_dot == -1) {
			_alifErr_setString(_thread, _alifExcImportError_,
				"تمت محاولة استيراد نسبي يتجاوز أعلى مستوى "
				"للحزمة");
			goto error;
		}
	}

	ALIF_XDECREF(spec);
	base = alifUStr_subString(package, 0, last_dot);
	ALIF_DECREF(package);
	if (base == nullptr or ALIFUSTR_GET_LENGTH(_name) == 0) {
		return base;
	}

	absName = alifUStr_fromFormat("%U.%U", base, _name);
	ALIF_DECREF(base);
	return absName;

no_parent_error:
	_alifErr_setString(_thread, _alifExcImportError_,
		"محاولة استيراد نسبي "
		"ضمن والد حزمة غير معروف");

error:
	ALIF_XDECREF(spec);
	ALIF_XDECREF(package);
	return nullptr;
}


static AlifObject* import_findAndLoad(AlifThread* tstate, AlifObject* abs_name) { // 3633
	AlifObject* mod = nullptr;
	AlifInterpreter* interp = tstate->interpreter;
	//AlifIntT import_time = _alifInterpreterState_getConfig(interp)->importTime;
//#define import_level FIND_AND_LOAD(interp).importLevel
//#define accumulated FIND_AND_LOAD(interp).accumulated

	//AlifTimeT t1 = 0, accumulated_copy = accumulated;

	AlifObject* sysPath = alifSys_getObject("مسار"); // Path
	AlifObject* sysMetaPath = alifSys_getObject("مسار_التعريف"); // meta_path
	AlifObject* sysPathHooks = alifSys_getObject("خطافات_المسار"); // path_hooks
	//if (_alifSys_audit(tstate, "import", "OOOOO",
	//	abs_name, ALIF_NONE, sysPath ? sysPath : ALIF_NONE,
	//	sysMetaPath ? sysMetaPath : ALIF_NONE,
	//	sysPathHooks ? sysPathHooks : ALIF_NONE) < 0) {
	//	return nullptr;
	//}


//	if (import_time) {
//#define header FIND_AND_LOAD(interp).header
//		if (header) {
//			fputs("import time: self [us] | cumulative | imported package\n",
//				stderr);
//			header = 0;
//		}
//#undef header
//
//		import_level++;
//		// ignore error: don't block import if reading the clock fails
//		(void)alifTime_perfCounterRaw(&t1);
//		accumulated = 0;
//	}

	mod = alifObject_callMethodObjArgs(IMPORTLIB(interp), &ALIF_STR(_findAndLoad),
		abs_name, IMPORT_FUNC(interp), nullptr);

	//if (import_time) {
	//	AlifTimeT t2;
	//	(void)alifTime_perfCounterRaw(&t2);
	//	AlifTimeT cum = t2 - t1;

	//	import_level--;
	//	fprintf(stderr, "import time: %9ld | %10ld | %*s%s\n",
	//		(long)_alifTime_asMicroseconds(cum - accumulated, AlifTime_Round_Ceiling),
	//		(long)_alifTime_asMicroseconds(cum, AlifTime_Round_Ceiling),
	//		import_level * 2, "", alifUStr_asUTF8(abs_name));

	//	accumulated = accumulated_copy + cum;
	//}

	return mod;
#undef import_level
#undef accumulated
}


AlifObject* alifImport_importModuleLevelObject(AlifObject* name, AlifObject* globals,
	AlifObject* locals, AlifObject* fromlist, AlifIntT level) { // 3688
 
	AlifThread* thread = _alifThread_get();
	AlifObject* absName = nullptr;
	AlifObject* finalMod = nullptr;
	AlifObject* mod = nullptr;
	AlifObject* package = nullptr;
	AlifInterpreter* interp = thread->interpreter;
	AlifIntT hasFrom{};

	if (name == nullptr) {
		_alifErr_setString(thread, _alifExcValueError_, "اسم الوحدة فارغ");
		goto error;
	}

	if (!ALIFUSTR_CHECK(name)) {
		//_alifErr_setString(tstate, _alifExcTypeError_,
		//	"module name must be a string");
		goto error;
	}
	if (level < 0) {
		//_alifErr_setString(tstate, _alifExcValueError_, "level must be >= 0");
		goto error;
	}

	if (level > 0) {
		absName = resolve_name(thread, name, globals, level);
		if (absName == nullptr)
			goto error;
	}
	else {  /* level == 0 */
		if (ALIFUSTR_GET_LENGTH(name) == 0) {
			_alifErr_setString(thread, _alifExcValueError_, "اسم وحدة فارغ");
			goto error;
		}
		absName = ALIF_NEWREF(name);
	}

	mod = import_getModule(thread, absName);
	if (mod == nullptr and _alifErr_occurred(thread)) {
		goto error;
	}

	if (mod != nullptr and mod != ALIF_NONE) {
		//if (import_ensureInitialized(thread->interpreter, mod, absName) < 0) {
		//	goto error;
		//}
	}
	else {
		ALIF_XDECREF(mod);
		mod = import_findAndLoad(thread, absName);

		if (mod == nullptr) {
			goto error;
		}
	}

	hasFrom = 0;
	if (fromlist != nullptr and fromlist != ALIF_NONE) {
		hasFrom = alifObject_isTrue(fromlist);
		if (hasFrom < 0)
			goto error;
	}
	if (!hasFrom) {
		AlifSizeT len = ALIFUSTR_GET_LENGTH(name);
		if (level == 0 or len > 0) {
			AlifSizeT dot{};

			dot = alifUStr_findChar(name, '.', 0, len, 1);
			if (dot == -2) {
				goto error;
			}

			if (dot == -1) {
				/* No dot in module name, simple exit */
				finalMod = ALIF_NEWREF(mod);
				goto error;
			}

			if (level == 0) {
				AlifObject* front = alifUStr_subString(name, 0, dot);
				if (front == nullptr) {
					goto error;
				}

				finalMod = alifImport_importModuleLevelObject(front, nullptr, nullptr, nullptr, 0);
				ALIF_DECREF(front);
			}
			else {
				AlifSizeT cut_off = len - dot;
				AlifSizeT abs_name_len = ALIFUSTR_GET_LENGTH(absName);
				AlifObject* to_return = alifUStr_subString(absName, 0,
					abs_name_len - cut_off);
				if (to_return == nullptr) {
					goto error;
				}

				finalMod = import_getModule(thread, to_return);
				ALIF_DECREF(to_return);
				if (finalMod == nullptr) {
					if (!_alifErr_occurred(thread)) {
						_alifErr_format(thread, _alifExcKeyError_,
							"%R ليس في النظام.وحدات كما يجب أن يكون",
							to_return);
					}
					goto error;
				}
			}
		}
		else {
			finalMod = ALIF_NEWREF(mod);
		}
	}
	else {
		AlifIntT hasPath = alifObject_hasAttrWithError(mod, &ALIF_STR(__path__));
		if (hasPath < 0) {
			goto error;
		}
		if (hasPath) {
			finalMod = alifObject_callMethodObjArgs(
				IMPORTLIB(interp), &ALIF_STR(_handleFromList),
				mod, fromlist, IMPORT_FUNC(interp), nullptr);
		}
		else {
			finalMod = ALIF_NEWREF(mod);
		}
	}

error:
	ALIF_XDECREF(absName);
	ALIF_XDECREF(mod);
	ALIF_XDECREF(package);
	if (finalMod == nullptr) {
		remove_importLibFrames(thread);
	}
	return finalMod;
}



AlifObject* alifImport_importModuleLevel(const char* name, AlifObject* globals,
	AlifObject* locals, AlifObject* fromlist, AlifIntT level) { // 3847
	AlifObject* nameobj{}, * mod{};
	nameobj = alifUStr_fromString(name);
	if (nameobj == nullptr)
		return nullptr;
	mod = alifImport_importModuleLevelObject(nameobj, globals, locals,
		fromlist, level);
	ALIF_DECREF(nameobj);
	return mod;
}



AlifObject* alifImport_import(AlifObject* _moduleName) { // 3888
	AlifThread* thread = _alifThread_get();
	AlifObject* globals = nullptr;
	AlifObject* import = nullptr;
	AlifObject* builtins = nullptr;
	AlifObject* r = nullptr;

	AlifObject* fromList = alifList_new(0);
	if (fromList == nullptr) {
		goto err;
	}

	/* Get the builtins from current globals */
	globals = alifEval_getGlobals();
	if (globals != nullptr) {
		ALIF_INCREF(globals);
		builtins = alifObject_getItem(globals, &ALIF_ID(__builtins__));
		if (builtins == nullptr)
			goto err;
	}
	else {
		/* No globals -- use standard builtins, and fake globals */
		builtins = alifImport_importModuleLevel("الضمنيات",
			nullptr, nullptr, nullptr, 0);
		if (builtins == nullptr) {
			goto err;
		}
		globals = alif_buildValue("{OO}", &ALIF_ID(__builtins__), builtins);
		if (globals == nullptr)
			goto err;
	}

	/* Get the __import__ function from the builtins */
	if (ALIFDICT_CHECK(builtins)) {
	import = alifObject_getItem(builtins, &ALIF_STR(__import__));
		if (import == nullptr) {
			_alifErr_setObject(thread, _alifExcKeyError_, &ALIF_STR(__import__));
		}
	}
	else {
	import = alifObject_getAttr(builtins, &ALIF_STR(__import__));
	}
	if (import == nullptr)
		goto err;

	/* Call the __import__ function with the proper argument list
	   Always use absolute import here.
	   Calling for side-effect of import. */
	r = alifObject_callFunction(import, "OOOOi", _moduleName, globals,
		globals, fromList, 0, nullptr);
	if (r == nullptr)
		goto err;
	ALIF_DECREF(r);

	r = import_getModule(thread, _moduleName);
	if (r == nullptr and !_alifErr_occurred(thread)) {
		_alifErr_setObject(thread, _alifExcKeyError_, _moduleName);
	}

err:
	ALIF_XDECREF(globals);
	ALIF_XDECREF(builtins);
	ALIF_XDECREF(import);
	ALIF_XDECREF(fromList);

	return r;
}



// alif
AlifStatus _alifImport_init() { // 3954

	if (INITTABLE != nullptr) {
		return ALIFSTATUS_ERR("تمت تهيئة حالة الاستيراد العامة بالفعل"); //* alif
	}

	AlifStatus status = ALIFSTATUS_OK();

	if (initBuildin_modulesTable() != 0) {
		status = alifStatus_noMemory();
	}


	return status;
}









AlifStatus _alifImport_initCore(AlifThread* _thread,
	AlifObject* _sysmod, AlifIntT _importLib) { // 4026
	// XXX Initialize here: interp->modules and interp->importFunc.
	// XXX Initialize here: sys.modules and sys.مسار_التعريف.

	if (_importLib) {
		if (init_importLib(_thread, _sysmod) < 0) {
			return ALIFSTATUS_ERR("failed to initialize importlib");
		}
	}

	return ALIFSTATUS_OK();
}



AlifStatus _alifImport_initExternal(AlifThread* _thread) { // 4133
	AlifIntT verbose = _alifInterpreterState_getConfig(_thread->interpreter)->verbose;

	// XXX Initialize here: النظام.خطافات_المسار and النظام.مخبأ_مستورد_المسار.

	if (init_importlibExternal(_thread->interpreter) != 0) {
		_alifErr_print(_thread);
		return ALIFSTATUS_ERR("external importer setup failed");
	}

	//if (init_zipImport(_thread, verbose) != 0) {
	//	alifErr_print();
	//	return ALIFSTATUS_ERR("initializing zipimport failed");
	//}

	return ALIFSTATUS_OK();
}


AlifObject* _alifImport_getModuleAttr(AlifObject* _modName,
	AlifObject* _attrName) { // 4173
	AlifObject* mod = alifImport_import(_modName);
	if (mod == nullptr) {
		return nullptr;
	}
	AlifObject* result = alifObject_getAttr(mod, _attrName);
	ALIF_DECREF(mod);
	return result;
}

AlifObject* _alifImport_getModuleAttrString(const char* _modName,
	const char* _attrName) { // 4177
	AlifObject* pmodname = alifUStr_fromString(_modName);
	if (pmodname == nullptr) {
		return nullptr;
	}
	AlifObject* pattrname = alifUStr_fromString(_attrName);
	if (pattrname == nullptr) {
		ALIF_DECREF(pmodname);
		return nullptr;
	}
	AlifObject* result = _alifImport_getModuleAttr(pmodname, pattrname);
	ALIF_DECREF(pattrname);
	ALIF_DECREF(pmodname);
	return result;
}




static AlifObject* _imp_createBuiltin(AlifObject* _module,
	AlifObject* _spec) { // 4299
	AlifThread* thread = _alifThread_get();

	AlifObject* name = alifObject_getAttrString(_spec, "اسم");
	if (name == nullptr) {
		return nullptr;
	}

	if (!ALIFUSTR_CHECK(name)) {
		alifErr_format(_alifExcTypeError_,
			"الاسم يجب أن يكون من نوع نص, وليس %.200s",
			ALIF_TYPE(name)->name);
		ALIF_DECREF(name);
		return nullptr;
	}

	AlifObject* mod = create_builtin(thread, name, _spec);
	ALIF_DECREF(name);
	return mod;
}


static AlifObject* _imp_extensionSuffixesImpl(AlifObject* _module) { // 4330
	AlifObject* list{};

	list = alifList_new(0);
	if (list == nullptr)
		return nullptr;
#ifdef HAVE_DYNAMIC_LOADING
	const char* suffix;
	unsigned int index = 0;

	while ((suffix = _alifImportDynLoadFiletab_[index])) {
		AlifObject* item = alifUStr_fromString(suffix);
		if (item == nullptr) {
			ALIF_DECREF(list);
			return nullptr;
		}
		if (alifList_append(list, item) < 0) {
			ALIF_DECREF(list);
			ALIF_DECREF(item);
			return nullptr;
		}
		ALIF_DECREF(item);
		index += 1;
	}
#endif
	return list;
}


static AlifObject* _imp_findFrozenImpl(AlifObject* _module,
	AlifObject* _name, AlifIntT _withdata) { // 4405
	FrozenInfo info{};
	FrozenStatus status = find_frozen(_name, &info);
	if (status == FrozenStatus::Frozen_Not_Found or status == FrozenStatus::Frozen_Disabled) {
		return ALIF_NONE;
	}
	else if (status == FrozenStatus::Frozen_Bad_Name) {
		return ALIF_NONE;
	}
	else if (status != FrozenStatus::Frozen_Okay) {
		//set_frozenError(status, _name);
		return nullptr;
	}

	AlifObject* data = nullptr;
	if (_withdata) {
		data = alifMemoryView_fromMemory((char *)info.data, info.size, ALIFBUF_READ);
		if (data == nullptr) {
			return nullptr;
		}
	}

	AlifObject* origname = nullptr;
	if (info.origname != nullptr and info.origname[0] != '\0') {
		origname = alifUStr_fromString(info.origname);
		if (origname == nullptr) {
			ALIF_XDECREF(data);
			return nullptr;
		}
	}

	AlifObject* result = alifTuple_pack(3, data ? data : ALIF_NONE,
		info.isPackage ? ALIF_TRUE : ALIF_FALSE,
		origname ? origname : ALIF_NONE);
	ALIF_XDECREF(origname);
	ALIF_XDECREF(data);
	return result;
}

static AlifObject* _imp_getFrozenObjectImpl(AlifObject* _module,
	AlifObject* _name, AlifObject* _dataObj) { // 4457
	FrozenInfo info = { 0 };
	AlifBuffer buf = { 0 };
	if (alifObject_checkBuffer(_dataObj)) {
		if (alifObject_getBuffer(_dataObj, &buf, ALIFBUF_SIMPLE) != 0) {
			return nullptr;
		}
		info.data = (const char*)buf.buf;
		info.size = buf.len;
	}
	else if (_dataObj != ALIF_NONE) {
		//_alifArg_badArgument("اجلب_كائن_مصرف", "argument 2", "bytes", _dataObj);
		return nullptr;
	}
	else {
		FrozenStatus status = find_frozen(_name, &info);
		if (status != FrozenStatus::Frozen_Okay) {
			//set_frozenError(status, _name);
			return nullptr;
		}
	}

	if (info.nameobj == nullptr) {
		info.nameobj = _name;
	}
	if (info.size == 0) {
		/* Does not contain executable code. */
		//set_frozenError(FrozenStatus::Frozen_Invalid, _name);
		return nullptr;
	}

	AlifInterpreter* interp = _alifInterpreter_get();
	AlifObject* codeobj = unmarshal_frozenCode(interp, &info);
	if (_dataObj != ALIF_NONE) {
		alifBuffer_release(&buf);
	}
	return codeobj;
}

static AlifObject* _imp_isBuiltinImpl(AlifObject* _module,
	AlifObject* _name) { // 4531
	return alifLong_fromLong(is_builtin(_name));
}

static AlifObject* _imp_isFrozenImpl(AlifObject* _module,
	AlifObject* _name) { // 4547
	FrozenInfo info{};
	FrozenStatus status = find_frozen(_name, &info);
	if (status != FrozenStatus::Frozen_Okay) {
		ALIF_RETURN_FALSE;
	}
	ALIF_RETURN_TRUE;
}



#ifdef HAVE_DYNAMIC_LOADING // 4628

static AlifObject* _imp_createDynamicImpl(AlifObject* module,
	AlifObject* spec, AlifObject* file) { // 4640
	AlifObject* mod{};
	AlifThread* tstate = _alifThread_get();

	class AlifExtModuleLoaderInfo info{};
	if (_alifExtModule_loaderInfoInitFromSpec(&info, spec) < 0) {
		return nullptr;
	}

	class ExtensionsCacheValue* cached = nullptr;
	mod = import_findExtension(tstate, &info, &cached);
	if (mod != nullptr) {
		goto finally;
	}
	else if (_alifErr_occurred(tstate)) {
		goto finally;
	}
	/* Otherwise it must be multi-phase init or the first time it's loaded. */

	if (cached != nullptr) {
		/* For now we clear the cache and move on. */
		_extensions_cacheDelete(info.path, info.name);
	}

	//if (alifSys_audit("استورد", "OOOOO", info.name, info.filename,
	//	ALIF_NONE, ALIF_NONE, ALIF_NONE) < 0) {
	//	goto finally;
	//}

	FILE* fp;
	if (file != nullptr) {
		fp = _alif_fOpenObj(info.filename, "r");
		if (fp == nullptr) {
			goto finally;
		}
	}
	else {
		fp = nullptr;
	}

	AlifModInitFunction p0; p0 = _alifImport_getModInitFunc(&info, fp);
	if (p0 == nullptr) {
		goto finally;
	}

	//_alifEval_enableGILTransient(tstate);

	mod = import_runExtension(
		tstate, p0, &info, spec, get_modulesDict(tstate, true));

	//if (_alifImport_checkGILForModule(mod, info.name) < 0) {
	//	ALIF_CLEAR(mod);
	//	goto finally;
	//}

	// XXX Shouldn't this happen in the error cases too (i.e. in "finally")?
	if (fp) {
		fclose(fp);
	}

	finally:
	_alifExtModule_loaderInfoClear(&info);
	return mod;
}



static AlifIntT _imp_execDynamicImpl(AlifObject* _module,
	AlifObject* _mod) { // 4738
	return exec_builtinOrDynamic(_mod);
}

#endif // 4746

static AlifIntT _imp_execBuiltinImpl(AlifObject* _module,
	AlifObject* _mod) { // 4757
	return exec_builtinOrDynamic(_mod);
}


static AlifObject* _imp_printImpl(AlifObject* module, AlifObject* msg) { //* alif //* todo //* delete
	AlifThread* thread = _alifThread_get();
	AlifObject* file = _alifSys_getAttr(thread, &ALIF_ID(Stderr));
	if (file == nullptr) {
		alifErr_setString(_alifExcRuntimeError_, "مفقود النظام.الإخراج");
		return nullptr;
	}

	AlifIntT err = 0;

	err = alifFile_writeString(" ", file);
	if (err) {
		return nullptr;
	}
	err = alifFile_writeObject(msg, file, ALIF_PRINT_RAW);
	if (err) {
		return nullptr;
	}

	alifFile_writeString("\n", file);
	return ALIF_NONE;
}

static AlifMethodDef _impMethods_[] = { // 4788
	_IMP_EXTENSION_SUFFIXES_METHODDEF
	//_IMP_LOCK_HELD_METHODDEF
	//_IMP_ACQUIRE_LOCK_METHODDEF
	//_IMP_RELEASE_LOCK_METHODDEF
	_IMP_FIND_FROZEN_METHODDEF
	_IMP_GET_FROZEN_OBJECT_METHODDEF
	//_IMP_IS_FROZEN_PACKAGE_METHODDEF
	_IMP_CREATE_BUILTIN_METHODDEF
	//_IMP_INIT_FROZEN_METHODDEF
	_IMP_IS_BUILTIN_METHODDEF
	_IMP_IS_FROZEN_METHODDEF
	//_IMP__FROZEN_MODULE_NAMES_METHODDEF
	//_IMP__OVERRIDE_FROZEN_MODULES_FOR_TESTS_METHODDEF
	//_IMP__OVERRIDE_MULTI_INTERP_EXTENSIONS_CHECK_METHODDEF
	_IMP_CREATE_DYNAMIC_METHODDEF
	_IMP_EXEC_DYNAMIC_METHODDEF
	_IMP_EXEC_BUILTIN_METHODDEF
	//_IMP__FIX_CO_FILENAME_METHODDEF
	//_IMP_SOURCE_HASH_METHODDEF
	_IMP_PRINT //* alif //* delete //* todo
	{nullptr, nullptr}  /* sentinel */
};


/*
 _ورد
 تعني وررِد
 اي أصل كلمة استورد
*/
static AlifModuleDef _impModule_ = { // 4838
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "_ورد", // _imp
	.size = 0,
	.methods = _impMethods_,
	//.slots = _impSlots_,
};


//* alif
AlifObject* alifInit__imp(void) { // 4847
	return alifModuleDef_init(&_impModule_);
}


