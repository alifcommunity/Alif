#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Import.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"
#include "AlifCore_Errors.h"
#include "AlifCore_Namespace.h"
#include "AlifCore_Object.h"


#include "Marshal.h"
#include "AlifCore_ImportDL.h"








extern InitTable _alifImportInitTab_[]; // 55

InitTable* _alifImportInitTable_ = _alifImportInitTab_; // 59

#define INITTABLE _alifDureRun_.imports.initTable // 69
#define LAST_MODULE_INDEX _alifDureRun_.imports.lastModuleIndex // 70
#define EXTENSIONS _alifDureRun_.imports.extensions // 71

// 80
#define MODULES(_interp) \
    (_interp)->imports.modules
#define MODULES_BY_INDEX(interp) \
    (interp)->imports.modulesByIndex
#define IMPORTLIB(_interp) \
    (_interp)->imports.importLib
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
			//alif_fatalError("interpreter has no modules dictionary");
		}
		//alifErr_setString(tstate, _alifExcRuntimeError_, "unable to get sys.modules");
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


AlifObject* alifImport_getModule(AlifObject* _name) { // 240
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
	//AlifObject* exc = _alifErr_getRaisedException(_thread);

	AlifObject* modules = get_modulesDict(_thread, true);
	if (ALIFDICT_CHECKEXACT(modules)) {
		// Error is reported to the caller
		(void)alifDict_pop(modules, _name, nullptr);
	}
	else if (ALIFMAPPING_DELITEM(modules, _name) < 0) {
		//if (_alifErr_exceptionMatches(_thread, _alifExcKeyError_)) {
		//	_alifErr_clear(_thread);
		//}
	}

	//_alifErr_chainExceptions1(exc);
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
	ALIFMUTEX_LOCK(&_alifDureRun_.imports.extensions.mutex);
}

static inline void extensions_lockRelease(void) { // 920
	ALIFMUTEX_UNLOCK(&_alifDureRun_.imports.extensions.mutex);
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

	alif_setImmortalUntracked((AlifObject*)def);

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
	EXTENSIONS.hashtable = alifHashTable_newFull(
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
		if (alifHashTable_set(EXTENSIONS.hashtable, key, newvalue) < 0) {
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



static AlifThread* switchTo_mainInterpreter(AlifThread* _thread) { // 1523
	if (alif_isMainInterpreter(_thread->interpreter)) {
		return _thread;
	}

	//* todo
	//AlifThread* main_tstate = _alifThread_newBound(
	//	alifInterpreter_main(), _ALIFTHREADSTATE_WHENCE_EXEC);
	//if (main_tstate == nullptr) {
	//	return nullptr;
	//}
	//(void)alifThread_swap(main_tstate);
	//return main_tstate;
	return _thread; //* alif //* delete
}


static AlifObject* getCore_moduleDict(AlifInterpreter* _interp,
	AlifObject* _name, AlifObject* _path) { // 1580
	/* Only builtin modules are core. */
	if (_path == _name) {
		if (alifUStr_compareWithASCIIString(_name, "sys") == 0) {
			return ALIF_NEWREF(_interp->sysdictCopy);
		}
		if (alifUStr_compareWithASCIIString(_name, "builtins") == 0) {
			return ALIF_NEWREF(_interp->builtinsCopy);
		}
	}
	return nullptr;
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
	//if (_alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
	//	return nullptr;
	//}

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
			if (alifModule_addObjectRef(mod, "__file__", info->filename) < 0) {
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
	//if (_alifImport_checkSubinterpIncompatibleExtensionAllowed(name_buf) < 0) {
	//	return nullptr;
	//}

	AlifObject* mod = reload_singlephaseExtension(tstate, cached, info);
	if (mod == nullptr) {
		return nullptr;
	}

	//AlifIntT verbose = alifInterpreter_getConfig(tstate->interpreter)->verbose;
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
				alifUStr_internImmortal(interp, &filename);

				if (alifModule_addObjectRef(mod, "__file__", filename) < 0) {
					//alifErr_clear(); /* Not important enough to report */
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
		//switchBackFrom_mainInterpreter(tstate, main_tstate, mod);
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
				tstate, mod, cached, info->name, modules) < 0)
			{
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
		//if (alifUStr_equalToASCIIString(_name, inittab[i].name)) {
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
		//if (alifUStr_equalToASCIIString(info.name, p->name)) {
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
		//_alifErr_format(_thread, _alifExcImportError_,
		//	"Loaded module %R not found in sys.modules",
		//	_name);
	}

	return m;
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
		//alifErr_clear();
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
		//info->isAlias = resolve_moduleAlias(name, _alifImportFrozenAliases_,
		//	&info->origname); //* todo
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
		//alifErr_clear();
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
		err = alifDict_setItemString(d, "__path__", l);
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
	err = alifDict_setItemString(d, "__origname__", origname);
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

	AlifObject* name = alifUStr_fromString("_imp");
	if (name == nullptr) {
		return nullptr;
	}

	AlifObject* attrs = alif_buildValue("{sO}", "name", name);
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
	//AlifIntT verbose = alifInterpreter_getConfig(interp)->verbose;

	// Import _importlib through its frozen version, _frozen_importlib.
	//if (verbose) {
	//	alifSys_formatStderr("import _frozen_importlib # frozen\n");
	//}
	if (alifImport_importFrozenModule("_frozen_importlib") <= 0) {
		return -1;
	}

	AlifObject* importlib = alifImport_addModuleRef("_frozen_importlib");
	if (importlib == nullptr) {
		return -1;
	}
	IMPORTLIB(interp) = importlib;

	// Import the _imp module

	AlifObject* impMod = bootstrap_imp(tstate);
	if (impMod == nullptr) {
		return -1;
	}
	if (_alifImport_setModuleString("_imp", impMod) < 0) {
		ALIF_DECREF(impMod);
		return -1;
	}

	// Install importlib as the implementation of import
	//AlifObject* value = alifObject_callMethod(importlib, "_install",
	//	"OO", sysmod, imp_mod); //* todo
	//ALIF_DECREF(imp_mod);
	//if (value == nullptr) {
	//	return -1;
	//}
	//ALIF_DECREF(value);

	return 0;
}



AlifIntT _alifImport_initDefaultImportFunc(AlifInterpreter* _interp) { // 3338
	// Get the __import__ function
	AlifObject* importFunc{};
	if (alifDict_getItemStringRef(_interp->builtins, "_استورد_", &importFunc) <= 0) {
		return -1;
	}
	IMPORT_FUNC(_interp) = importFunc;
	return 0;
}

AlifIntT _alifImport_isDefaultImportFunc(AlifInterpreter* _interp, AlifObject* _func) { // 3350
	return _func == IMPORT_FUNC(_interp);
}



static AlifObject* import_findAndLoad(AlifThread* tstate, AlifObject* abs_name) { // 3617
	AlifObject* mod = nullptr;
	AlifInterpreter* interp = tstate->interpreter;
	//AlifIntT import_time = alifInterpreter_getConfig(interp)->importTime;
//#define import_level FIND_AND_LOAD(interp).importLevel
//#define accumulated FIND_AND_LOAD(interp).accumulated

	//AlifTimeT t1 = 0, accumulated_copy = accumulated;

	AlifObject* sysPath = alifSys_getObject("Path");
	//AlifObject* sysMetaPath = alifSys_getObject("meta_path");
	//AlifObject* sysPathHooks = alifSys_getObject("path_hooks");
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

	mod = alifObject_callMethodObjArgs(IMPORTLIB(interp), &ALIF_ID(_findAndLoad),
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



//AlifObject* alifImport_importModuleLevelObject(AlifObject* name, AlifObject* globals,
//	AlifObject* locals, AlifObject* fromlist, AlifIntT level) { // 3688
//	AlifThread* thread = _alifThread_get();
//	AlifObject* absName = nullptr;
//	AlifObject* finalMod = nullptr;
//	AlifObject* mod = nullptr;
//	AlifObject* package = nullptr;
//	AlifInterpreter* interp = thread->interpreter;
//	AlifIntT hasFrom{};
//
//	if (name == nullptr) {
//		//_alifErr_setString(tstate, _alifExcValueError_, "Empty module name");
//		goto error;
//	}
//
//	if (!ALIFUSTR_CHECK(name)) {
//		//_alifErr_setString(tstate, _alifExcTypeError_,
//		//	"module name must be a string");
//		goto error;
//	}
//	if (level < 0) {
//		//_alifErr_setString(tstate, _alifExcValueError_, "level must be >= 0");
//		goto error;
//	}
//
//	if (level > 0) {
//		//absName = resolve_name(tstate, name, globals, level);
//		if (absName == nullptr)
//			goto error;
//	}
//	else {  /* level == 0 */
//		if (ALIFUSTR_GET_LENGTH(name) == 0) {
//			//_alifErr_setString(tstate, _alifExcValueError_, "Empty module name");
//			goto error;
//		}
//		absName = ALIF_NEWREF(name);
//	}
//
//	mod = import_getModule(thread, absName);
//	if (mod == nullptr and _alifErr_occurred(thread)) {
//		goto error;
//	}
//
//	if (mod != nullptr and mod != ALIF_NONE) {
//		//if (import_ensureInitialized(thread->interpreter, mod, abs_name) < 0) {
//		//	goto error;
//		//}
//	}
//	else {
//		ALIF_XDECREF(mod);
//		mod = import_findAndLoad(thread, absName);
//
//		if (mod == nullptr) {
//			goto error;
//		}
//	}
//
//	hasFrom = 0;
//	if (fromlist != nullptr and fromlist != ALIF_NONE) {
//		hasFrom = alifObject_isTrue(fromlist);
//		if (hasFrom < 0)
//			goto error;
//	}
//	if (!hasFrom) {
//		AlifSizeT len = ALIFUSTR_GET_LENGTH(name);
//		if (level == 0 or len > 0) {
//			AlifSizeT dot{};
//
//			dot = alifUStr_findChar(name, '.', 0, len, 1);
//			if (dot == -2) {
//				goto error;
//			}
//
//			if (dot == -1) {
//				/* No dot in module name, simple exit */
//				finalMod = ALIF_NEWREF(mod);
//				goto error;
//			}
//
//			if (level == 0) {
//				AlifObject* front = alifUStr_subString(name, 0, dot);
//				if (front == nullptr) {
//					goto error;
//				}
//
//				finalMod = alifImport_importModuleLevelObject(front, nullptr, nullptr, nullptr, 0);
//				ALIF_DECREF(front);
//			}
//			else {
//				AlifSizeT cut_off = len - dot;
//				AlifSizeT abs_name_len = ALIFUSTR_GET_LENGTH(absName);
//				AlifObject* to_return = alifUStr_subString(absName, 0,
//					abs_name_len - cut_off);
//				if (to_return == nullptr) {
//					goto error;
//				}
//
//				finalMod = import_getModule(thread, to_return);
//				ALIF_DECREF(to_return);
//				if (finalMod == nullptr) {
//					//if (!_alifErr_occurred(tstate)) {
//					//	_alifErr_format(tstate, _alifExcKeyError_,
//					//		"%R not in sys.modules as expected",
//					//		to_return);
//					//}
//					goto error;
//				}
//			}
//		}
//		else {
//			finalMod = ALIF_NEWREF(mod);
//		}
//	}
//	else {
//		AlifIntT hasPath = alifObject_hasAttrWithError(mod, &ALIF_ID(__path__));
//		if (hasPath < 0) {
//			goto error;
//		}
//		if (hasPath) {
//			//final_mod = alifObject_callMethodObjArgs(
//			//	IMPORTLIB(interp), &ALIF_ID(_handleFromList),
//			//	mod, fromlist, IMPORT_FUNC(interp), nullptr);
//		}
//		else {
//			finalMod = ALIF_NEWREF(mod);
//		}
//	}
//
//error:
//	ALIF_XDECREF(absName);
//	ALIF_XDECREF(mod);
//	ALIF_XDECREF(package);
//	if (finalMod == nullptr) {
//		//remove_importLibFrames(tstate);
//	}
//	return finalMod;
//}



AlifObject* alifImport_importModuleLevel(const char* name, AlifObject* globals,
	AlifObject* locals, AlifObject* fromlist, AlifIntT level) { // 3839
	AlifObject* nameobj{}, * mod{};
	nameobj = alifUStr_fromString(name);
	if (nameobj == nullptr)
		return nullptr;
	mod = alifImport_importModuleLevelObject(nameobj, globals, locals,
		fromlist, level);
	ALIF_DECREF(nameobj);
	return mod;
}



AlifObject* alifImport_import(AlifObject* module_name) { // 3888
	AlifThread* tstate = _alifThread_get();
	AlifObject* globals = nullptr;
	AlifObject* import = nullptr;
	AlifObject* builtins = nullptr;
	AlifObject* r = nullptr;

	AlifObject* from_list = alifList_new(0);
	if (from_list == nullptr) {
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
		builtins = alifImport_importModuleLevel("builtins",
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
			//_alifErr_setObject(tstate, _alifExcKeyError_, &ALIF_STR(__import__));
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
	r = alifObject_callFunction(import, "OOOOi", module_name, globals,
		globals, from_list, 0, nullptr);
	if (r == nullptr)
		goto err;
	ALIF_DECREF(r);

	r = import_getModule(tstate, module_name);
	if (r == nullptr and !_alifErr_occurred(tstate)) {
		//_alifErr_setObject(tstate, _alifExcKeyError_, module_name);
	}

err:
	ALIF_XDECREF(globals);
	ALIF_XDECREF(builtins);
	ALIF_XDECREF(import);
	ALIF_XDECREF(from_list);

	return r;
}



// alif
static void init_importFileTab(); //* delete
AlifIntT alifImport_init() { // 3954

	if (INITTABLE != nullptr) {
		// error
		return -1; //* alif
	}

	if (initBuildin_modulesTable() != 0) {
		return -1;
	}

	init_importFileTab(); //* alif //* delete

	return 1;
}









AlifIntT _alifImport_initCore(AlifThread* _thread,
	AlifObject* _sysmod, AlifIntT _importLib) { // 4026
	// XXX Initialize here: interp->modules and interp->importFunc.
	// XXX Initialize here: sys.modules and sys.meta_path.

	if (_importLib) {
		//if (init_importLib(_thread, _sysmod) < 0) { //* todo
		//	return -1;
		//}
	}

	return 1;
}




static AlifMethodDef _impMethods_[] = { // 4788
	//_IMP_EXTENSION_SUFFIXES_METHODDEF
	//_IMP_LOCK_HELD_METHODDEF
	//_IMP_ACQUIRE_LOCK_METHODDEF
	//_IMP_RELEASE_LOCK_METHODDEF
	//_IMP_FIND_FROZEN_METHODDEF
	//_IMP_GET_FROZEN_OBJECT_METHODDEF
	//_IMP_IS_FROZEN_PACKAGE_METHODDEF
	//_IMP_CREATE_BUILTIN_METHODDEF
	//_IMP_INIT_FROZEN_METHODDEF
	//_IMP_IS_BUILTIN_METHODDEF
	//_IMP_IS_FROZEN_METHODDEF
	//_IMP__FROZEN_MODULE_NAMES_METHODDEF
	//_IMP__OVERRIDE_FROZEN_MODULES_FOR_TESTS_METHODDEF
	//_IMP__OVERRIDE_MULTI_INTERP_EXTENSIONS_CHECK_METHODDEF
	//_IMP_CREATE_DYNAMIC_METHODDEF
	//_IMP_EXEC_DYNAMIC_METHODDEF
	//_IMP_EXEC_BUILTIN_METHODDEF
	//_IMP__FIX_CO_FILENAME_METHODDEF
	//_IMP_SOURCE_HASH_METHODDEF
	{nullptr, nullptr}  /* sentinel */
};



static AlifModuleDef _impModule_ = { // 4838
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "_imp",
	.size = 0,
	.methods = _impMethods_,
	//.slots = _impSlots_,
};


//* alif
AlifObject* alifInit__imp(void) { // 4847
	return alifModuleDef_init(&_impModule_);
}











/* --------------------------------------------------- alif import impl --------------------------------------------------- */

// IMPORTANT!: ALIFUSTR_GET_LENGTH return num of letters but we need num of bytes in this sys

//* alif
#include "AlifCore_Compile.h"
#include "AlifCore_UStrObject.h"
#include "OSDefs.h"
#ifndef _WINDOWS
#include <sys/stat.h>
#include <sys/types.h>
#endif

static AlifIntT case_ok(char*, AlifSizeT, AlifSizeT, char*);
static AlifIntT init_builtin(const char*);
static AlifIntT mark_miss(char*);
static AlifIntT find_initModule(char*);
static AlifIntT ensure_fromList(AlifObject*, AlifObject*, char*, AlifSizeT, AlifIntT);
static AlifObject* load_next(AlifObject*, AlifObject*, const char**, char*, AlifSizeT*);
static AlifObject* get_parent(AlifObject*, char*, AlifSizeT*, AlifIntT);
static AlifObject* import_submodule(AlifObject*, const char*, char*);
static class FileDescr* find_module(const char*, const char*, AlifObject*, char*, AlifUSizeT, FILE**, AlifObject**);
static AlifObject* load_module(const char*, FILE*, char*, AlifIntT, AlifObject*);

#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)

//* alif

enum FileType { // 10
	SEARCH_ERROR,
	ALIF_SOURCE,
	ALIF_COMPILED,
	C_EXTENSION,
	ALIF_RESOURCE, /* Mac only */
	PKG_DIRECTORY,
	CPP_BUILTIN,
	ALIF_CODERESOURCE, /* Mac only */
};

class FileDescr { // 23
public:
	const char* suffix{};
	const char* mode{};
	FileType type{};
};
extern FileDescr* _alifImportFileTab_;


static AlifObject* extensions = nullptr; // 90

FileDescr* _alifImportFileTab_ = nullptr; // 98


static const FileDescr _alifImportStandardFileTab_[] = { // 107
	{".aliflib", "U", ALIF_SOURCE},
#ifdef _WINDOWS
	{".alifw", "U", ALIF_SOURCE},
#endif
	{".alifc", "rb", ALIF_COMPILED},
	{0, 0}
};


static void init_importFileTab() { //* alif
	const FileDescr* scan{};
	FileDescr* filetab{};
	AlifIntT countD = 0;
	AlifIntT countS = 0;

	for (scan = _alifImportStandardFileTab_; scan->suffix != nullptr; ++scan)
		++countS;
	filetab = (((AlifUSizeT)(countD + countS + 1) > ALIF_SIZET_MAX / sizeof(FileDescr)) ? nullptr : \
		((FileDescr*)alifMem_dataAlloc((countD + countS + 1) * sizeof(FileDescr))));

	if (filetab == nullptr) {
		//alif_fatalError("Can't initialize import file table.");
	}

	memcpy(filetab + countD, _alifImportStandardFileTab_,
		countS * sizeof(FileDescr));
	filetab[countD + countS].suffix = nullptr;

	_alifImportFileTab_ = filetab;

}


AlifObject* alifImport_getModuleDict(void) { // 364
	AlifInterpreter* interp = alifInterpreter_get();
	if (interp->imports.modules == nullptr) {
		//alif_fatalError("alifImport_getModuleDict: no module dictionary!");
	}
	return interp->imports.modules;
}


AlifObject* _alifImport_fixupExtension(const char* _name) { // 546
	AlifObject* modules{}, * mod{}, * dict{}, * copy{};
	if (extensions == nullptr) {
		extensions = alifDict_new();
		if (extensions == nullptr)
			return nullptr;
	}

	modules = alifImport_getModuleDict();
	AlifObject* name = alifUStr_fromString(_name); //* alif
	mod = alifDict_getItemWithError(modules, name);
	if (mod == nullptr or !ALIFMODULE_CHECK(mod)) {
		//alifErr_format(_alifExcSystemError_,
		//	"_alifImport_fixupExtension: module %.200s not loaded", name);
		return nullptr;
	}
	dict = alifModule_getDict(mod);
	if (dict == nullptr)
		return nullptr;
	copy = alifDict_copy(dict);
	if (copy == nullptr)
		return nullptr;
	alifDict_setItemString(extensions, _name, copy);
	ALIF_DECREF(copy);
	return copy;
}

AlifObject* _alifImport_findExtension(const char* _name) { // 573
	AlifObject* dict{}, * mod{}, * mdict{};
	if (extensions == nullptr)
		return nullptr;

	AlifObject* name = alifUStr_fromString(_name); //* alif
	dict = alifDict_getItemWithError(extensions, name);
	if (dict == nullptr)
		return nullptr;
	mod = alifImport_addModuleRef(_name);
	if (mod == nullptr)
		return nullptr;
	mdict = alifModule_getDict(mod);
	if (mdict == nullptr)
		return nullptr;
	if (alifDict_update(mdict, dict))
		return nullptr;
	return mod;
}


AlifObject* alifImport_execCodeModuleEx(const char* name, AlifObject* co, const char* pathname) { // 649
	AlifObject* modules = alifImport_getModuleDict();
	AlifObject* m{}, * d{}, * v{};

	m = alifImport_addModuleRef(name);
	if (m == nullptr)
		return nullptr;

	d = alifModule_getDict(m);
	if (alifDict_getItemWithError(d, &ALIF_ID(__builtins__)) == nullptr) {
		if (alifDict_setItem(d, &ALIF_ID(__builtins__),
			alifEval_getBuiltins()) != 0)
			goto error;
	}

	if (pathname != nullptr) {
		v = alifUStr_fromString(pathname);
		if (v == nullptr) {
			alifErr_clear();
		}
	}
	if (v == nullptr) {
		v = ((AlifCodeObject*)co)->filename;
		ALIF_INCREF(v);
	}
	if (alifDict_setItem(d, &ALIF_ID(__file__), v) != 0) {
		alifErr_clear();
	}
	ALIF_DECREF(v);

	v = alifEval_evalCode(co, d, d);
	if (v == nullptr)
		goto error;
	ALIF_DECREF(v);

	if ((alifDict_getItemStringRef(modules, name, &m)) < 0) {
		//alifErr_format(_alifExcImportError_,
		//	"Loaded module %.200s not found in sys.modules",
		//	name);
		return nullptr;
	}

	ALIF_INCREF(m);

	return m;

error:
	//remove_module(name);
	return nullptr;
}


static AlifCodeObject* parse_sourceModule(const char* pathname, FILE* fp) { // 817
	AlifCodeObject* co = nullptr;
	ModuleTy mod{};
	AlifCompilerFlags flags{};
	AlifObject* fn{};
	AlifASTMem* astMem = alifASTMem_new();
	if (astMem == nullptr)
		return nullptr;

	flags.flags = 0;

	fn = alifUStr_fromString(pathname);

	mod = alifParser_astFromFile(fp, ALIF_FILE_INPUT, fn, nullptr, nullptr, nullptr, &flags,
		nullptr, nullptr, astMem);
	if (mod) {
		co = _alifAST_compile(mod, fn, &flags, 0, astMem);
	}
	alifASTMem_free(astMem);
	return co;
}


static AlifObject* load_sourceModule(const char* _name, const char* _pathname, FILE* _fp) { // 966
	FILE* fpc{};
	char buf[MAXPATHLEN + 1]{};
	AlifCodeObject* co{};
	AlifObject* m{};

	co = parse_sourceModule(_pathname, _fp);
	if (co == nullptr)
		return nullptr;

	m = alifImport_execCodeModuleEx(_name, (AlifObject*)co, _pathname);
	ALIF_DECREF(co);

	return m;
}


static AlifObject* load_package(const char* name, char* pathname) { // 1035
	AlifObject* m{}, * d{};
	AlifObject* file = nullptr;
	AlifObject* path = nullptr;
	AlifIntT err{};
	char buf[MAXPATHLEN + 1];
	FILE* fp = nullptr;
	FileDescr* fdp{};

	m = alifImport_addModuleRef(name);
	if (m == nullptr)
		return nullptr;

	d = alifModule_getDict(m);
	file = alifUStr_fromString(pathname);
	if (file == nullptr)
		goto error;
	path = alif_buildValue("[O]", file);
	if (path == nullptr)
		goto error;
	err = alifDict_setItemString(d, "__file__", file);
	if (err == 0)
		err = alifDict_setItemString(d, "__path__", path);
	if (err != 0)
		goto error;
	buf[0] = '\0';
	fdp = find_module(name, "_تهيئة_", path, buf, sizeof(buf), &fp, nullptr);
	if (fdp == nullptr) {
		//if (alifErr_exceptionMatches(_alifExcImportError_)) {
		//	alifErr_clear();
		//	ALIF_INCREF(m);
		//}
		//else {
			m = nullptr;
		//}
		goto cleanup;
	}
	m = load_module(name, fp, buf, fdp->type, nullptr);
	if (fp != nullptr)
		fclose(fp);
	goto cleanup;

error:
	m = nullptr;
cleanup:
	ALIF_XDECREF(path);
	ALIF_XDECREF(file);
	return m;
}



static FileDescr* find_module(const char* _fullname, const char* _subname, AlifObject* _path, char* _buf,
	AlifUSizeT _buflen, FILE** _pFP, AlifObject** _pLoader) { // 1198
	AlifSizeT i{}, npath{};
	AlifUSizeT len{}, namelen{};
	FileDescr* fdp = nullptr;
	const char* filemode{};
	FILE* fp = nullptr;

	struct stat statbuf {};

	static FileDescr fd_builtin = { "", "", CPP_BUILTIN };
	static FileDescr fd_package = { "", "", PKG_DIRECTORY };
	char name[MAXPATHLEN + 1]{};

	if (strlen(_subname) > MAXPATHLEN) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"module name is too long");
		return nullptr;
	}
	strcpy(name, _subname);

	//if (_path != nullptr and ALIFUSTR_CHECK(_path)) {
	//	/* The only type of submodule allowed inside a "frozen"
	//	   package are other frozen modules or packages. */
	//	if (ALIFUSTR_GET_LENGTH(_path) + 1 + strlen(name) >= (AlifUSizeT)_buflen) {
	//		//alifErr_setString(_alifExcImportError_,
	//		//	"full frozen module name too long");
	//		return nullptr;
	//	}
	//	strcpy(_buf, alifUStr_asUTF8(_path));
	//	strcat(_buf, ".");
	//	strcat(_buf, name);
	//	strcpy(name, _buf);
	//	if (find_frozen(name) != nullptr) {
	//		strcpy(_buf, name);
	//		return &fd_frozen;
	//	}
	//	alifErr_format(_alifExcImportError_,
	//		"No frozen submodule named %.200s", name);
	//	return nullptr;
	//}
	if (_path == nullptr) {
		AlifObject* biltinName = alifUStr_fromString(name);
		if (is_builtin(biltinName)) {
			strcpy(_buf, name);
			return &fd_builtin;
		}

		_path = alifSys_getObject("Path");
	}

	if (_path == nullptr or !ALIFLIST_CHECK(_path)) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"sys.path must be a list of directory names");
		return nullptr;
	}

	npath = ALIFLIST_GET_SIZE(_path);
	namelen = strlen(name);
	for (i = 0; i < npath; i++) {
		AlifObject* copy = nullptr;
		AlifObject* v = alifList_getItem(_path, i);
		if (!v)
			return nullptr;

		if (!ALIFUSTR_CHECK(v))
			continue;

		len = strlen(alifUStr_asUTF8(v));
		if (len + 7 + namelen + 12 >= _buflen) {
			ALIF_XDECREF(copy);
			continue; /* Too long */
		}
		strcpy(_buf, alifUStr_asUTF8(v));
		if (strlen(_buf) != len) {
			ALIF_XDECREF(copy);
			continue; /* v contains '\0' */
		}

		/* no hook was found, use builtin import */

		if (len > 0 and _buf[len - 1] != SEP
#ifdef ALTSEP
			and _buf[len - 1] != ALTSEP
#endif
			)
			_buf[len++] = SEP;
		strcpy(_buf + len, name);
		len += namelen;

		if (stat(_buf, &statbuf) == 0 and         /* it exists */
			S_ISDIR(statbuf.st_mode) and         /* it's a directory */
			case_ok(_buf, len, namelen, name)) { /* case matches */
			if (find_initModule(_buf)) { /* and has _تهيئة_.aliflib */
				ALIF_XDECREF(copy);
				return &fd_package;
			}
			//else {
			//	char warnstr[MAXPATHLEN + 80];
			//	sprintf(warnstr, "Not importing directory "
			//		"'%.*s': missing __تهيئة__.aliflib",
			//		MAXPATHLEN, buf);
			//	if (alifErr_warn(_alifExcImportWarning_,
			//		warnstr)) {
			//		ALIF_XDECREF(copy);
			//		return nullptr;
			//	}
			//}
		}

		for (fdp = _alifImportFileTab_; fdp->suffix != nullptr; fdp++) {
			strcpy(_buf + len, fdp->suffix);
			filemode = fdp->mode;
			if (filemode[0] == 'U')
				filemode = "r"; // "b";
			fp = fopen(_buf, filemode);
			if (fp != nullptr) {
					break;
			}
		}
		ALIF_XDECREF(copy);
		if (fp != nullptr)
			break;
	}
	if (fp == nullptr) {
		//alifErr_format(_alifExcImportError_,
		//	"No module named %.200s", name);
		return nullptr;
	}
	*_pFP = fp;
	return fdp;
}





static AlifIntT case_ok(char* _buf, AlifSizeT _len, AlifSizeT _namelen, char* _name) { // 1577
	/* Pick a platform-specific implementation; the sequence of #if's here should
	 * match the sequence just above.
	 */

	 /* _WINDOWS */
#if defined(_WINDOWS)
	WIN32_FIND_DATA data;
	HANDLE h;

	//* alif
	wchar_t str[MAXPATHLEN]{};
	AlifSizeT len = strlen(_buf);
	mbstowcs(str, _buf, len);
	//* alif

	h = FindFirstFile(str, &data);
	if (h == INVALID_HANDLE_VALUE) {
		//alifErr_format(_alifExcNameError_,
		//	"Can't find file for module %.100s\n(filename %.300s)",
		//	name, buf);
		return 0;
	}
	FindClose(h);

	//* alif
	wchar_t name[MAXPATHLEN]{};
	AlifSizeT namelen = strlen(_name);
	mbstowcs(name, _name, namelen);
	//* alif

	return wcsncmp(data.cFileName, name, namelen) == 0;



	/* new-fangled macintosh (macosx) or Cygwin */
#elif (defined(__MACH__) && defined(__APPLE__) || defined(__CYGWIN__)) && defined(HAVE_DIRENT_H)
	DIR* dirp;
	struct dirent* dp;
	char dirname[MAXPATHLEN + 1];
	const int dirlen = len - namelen - 1; /* don't want trailing SEP */

	/* Copy the dir component into dirname; substitute "." if empty */
	if (dirlen <= 0) {
		dirname[0] = '.';
		dirname[1] = '\0';
	}
	else {
		memcpy(dirname, buf, dirlen);
		dirname[dirlen] = '\0';
	}
	/* Open the directory and search the entries for an exact match. */
	dirp = opendir(dirname);
	if (dirp) {
		char* nameWithExt = buf + len - namelen;
		while ((dp = readdir(dirp)) != nullptr) {
			const int thislen =
#ifdef _DIRENT_HAVE_D_NAMELEN
				dp->d_namlen;
#else
				strlen(dp->d_name);
#endif
			if (thislen >= namelen &&
				strcmp(dp->d_name, nameWithExt) == 0) {
				(void)closedir(dirp);
				return 1; /* Found */
			}
		}
		(void)closedir(dirp);
	}
	return 0; /* Not found */

#else
	return 1;
#endif
}


static AlifIntT find_initModule(char* buf) { // 1715
	const AlifUSizeT save_len = strlen(buf);
	AlifUSizeT i = save_len;
	char* pname{};  /* pointer to start of _تهيئة_ */
	struct stat statbuf;

	/*      For calling case_ok(buf, len, namelen, name):
	 *      /a/b/c/d/e/f/g/h/i/j/k/some_long_module_name.aliflib\0
	 *      ^                      ^                   ^    ^
	 *      |--------------------- buf ---------------------|
	 *      |------------------- len ------------------|
	 *                             |------ name -------|
	 *                             |----- namelen -----|
	 */
	if (save_len + 15 >= MAXPATHLEN)
		return 0;
	buf[i++] = SEP;
	pname = buf + i;
	strcpy(pname, "_تهيئة_.aliflib");
	if (stat(buf, &statbuf) == 0) {
		if (case_ok(buf,
			save_len + 9,               /* len("/_تهيئة_") */
			8,                              /* len("_تهيئة_") */
			pname)) {
			buf[save_len] = '\0';
			return 1;
		}
	}
	return 0;
}


static AlifObject* load_module(const char* _name, FILE* fp,
	char* pathname, AlifIntT type, AlifObject* loader) { // 1800
	AlifObject* modules{};
	AlifObject* m{};
	AlifIntT err{};

	/* First check that there's an open file (if we need one)  */
	switch (type) {
	case ALIF_SOURCE:
	case ALIF_COMPILED:
		if (fp == nullptr) {
			//alifErr_format(_alifExcValueError_,
			//	"file object required for import (type code %d)",
			//	type);
			return nullptr;
		}
	}

	switch (type) {

	case ALIF_SOURCE:
		m = load_sourceModule(_name, pathname, fp);
		break;
	//case ALIF_COMPILED:
	//	m = load_compiled_module(name, pathname, fp);
	//	break;
	case PKG_DIRECTORY:
		m = load_package(_name, pathname);
		break;
	case CPP_BUILTIN:
		if (pathname != nullptr and pathname[0] != '\0')
			_name = pathname;
		err = init_builtin(_name);
		if (err < 0) {
			return nullptr;
		}

		modules = alifImport_getModuleDict();
		alifDict_getItemStringRef(modules, _name, &m);
		if (m == nullptr) {
			alifErr_format(_alifExcImportError_,
				"%s module %.200s not properly initialized",
				"builtin", _name);
			return nullptr;
		}

		ALIF_INCREF(m);
		break;
	default:
		alifErr_format(_alifExcImportError_,
			"Don't know how to import %.200s (type code %d)",
			_name, type);
		m = nullptr;
	}

	return m;
}




static AlifIntT init_builtin(const char* _name) { // 1897
	InitTable* p{};

	if (_alifImport_findExtension(_name) != nullptr)
		return 1;

	for (p = _alifImportInitTab_; p->name != nullptr; p++) {
		if (strcmp(_name, p->name) == 0) {
			if (p->initFunc == nullptr) {
				//alifErr_format(_alifExcImportError_,
				//	"Cannot re-init internal module %.200s",
				//	name);
				return -1;
			}
			//(*p->initFunc)();

			//* alif
			AlifObject* modules{};
			AlifObject* m{};
			AlifThread* thread = _alifThread_get();
			AlifObject* name = alifUStr_fromString(_name);

			AlifObject* attrs = alif_buildValue("{sO}", "name", name);
			if (attrs == nullptr) {
				return -1;
			}
			AlifObject* spec = alifNamespace_new(attrs);
			ALIF_DECREF(attrs);
			if (spec == nullptr) {
				return -1;
			}

			m = create_builtin(thread, name, spec);
			ALIF_DECREF(spec);
			if (m == nullptr) {
				return -1;
			}

			if (exec_builtinOrDynamic(m) < 0) {
				ALIF_DECREF(m);
				return -1;
			}

			// to add the lib to main import in thread
			modules = alifImport_getModuleDict();
			alifDict_setItemString(modules, _name, m);
			//* alif


			if (alifErr_occurred())
				return -1;
			if (_alifImport_fixupExtension(_name) == nullptr)
				return -1;
			return 1;
		}
	}
	return 0;
}




AlifObject* alifImport_importModule(const char* _name) { // 2036
	AlifObject* pname{};
	AlifObject* result{};

	pname = alifUStr_fromString(_name);
	if (pname == nullptr) return nullptr;
	result = alifImport_import(pname);
	ALIF_DECREF(pname);
	return result;
}


static AlifObject* import_moduleLevel(const char* _name, AlifObject* _globals, AlifObject* _locals,
	AlifObject* _fromlist, AlifIntT _level) { // 2114
	char buf[MAXPATHLEN + 1]{};
	AlifSizeT buflen = 0;
	AlifObject* parent{}, * head{}, * next{}, * tail{};

	if (strchr(_name, '/') != nullptr
#ifdef _WINDOWS
		or strchr(_name, '\\') != nullptr
#endif
		) {
		//alifErr_setString(_alifExcImportError_,
		//	"Import by filename is not supported.");
		return nullptr;
	}

	parent = get_parent(_globals, buf, &buflen, _level);
	if (parent == nullptr)
		return nullptr;

	head = load_next(parent, _level < 0 ? ALIF_NONE : parent, &_name, buf,
		&buflen);
	if (head == nullptr)
		return nullptr;

	tail = head;
	ALIF_INCREF(tail);
	while (_name) {
		next = load_next(tail, tail, &_name, buf, &buflen);
		ALIF_DECREF(tail);
		if (next == nullptr) {
			ALIF_DECREF(head);
			return nullptr;
		}
		tail = next;
	}
	if (tail == ALIF_NONE) {
		ALIF_DECREF(tail);
		ALIF_DECREF(head);
		//alifErr_setString(_alifExcValueError_,
		//	"Empty module name");
		return nullptr;
	}

	if (_fromlist != nullptr) {
		if (_fromlist == ALIF_NONE or !alifObject_isTrue(_fromlist))
			_fromlist = nullptr;
	}

	if (_fromlist == nullptr) {
		ALIF_DECREF(tail);
		return head;
	}

	ALIF_DECREF(head);
	if (!ensure_fromList(tail, _fromlist, buf, buflen, 0)) {
		ALIF_DECREF(tail);
		return nullptr;
	}

	return tail;
}


AlifObject* alifImport_importModuleLevelObject(AlifObject* name, AlifObject* globals,
	AlifObject* locals, AlifObject* fromlist, AlifIntT level) { // 2182
	AlifObject* result{};
	//_alifImport_acquireLock();
	level = -1; //* alif
	result = import_moduleLevel(alifUStr_asUTF8(name), globals, locals, fromlist, level);
	//if (_alifImport_releaseLock() < 0) {
	//	ALIF_XDECREF(result);
	//	alifErr_setString(_alifExcRuntimeError_,
	//		"not holding the import lock");
	//	return nullptr;
	//}
	return result;
}




static AlifObject* get_parent(AlifObject* globals, char* buf,
	AlifSizeT* p_buflen, AlifIntT level) { // 2209
	static AlifObject* namestr = nullptr;
	static AlifObject* pathstr = nullptr;
	static AlifObject* pkgstr = nullptr;
	AlifObject* pkgname{}, * modname{}, * modpath{}, * modules{}, * parent{};
	AlifIntT orig_level = level;

	if (globals == nullptr or !ALIFDICT_CHECK(globals) or !level)
		return ALIF_NONE;

	if (namestr == nullptr) {
		namestr = alifUStr_internFromString("__name__");
		if (namestr == nullptr)
			return nullptr;
	}
	if (pathstr == nullptr) {
		pathstr = alifUStr_internFromString("__path__");
		if (pathstr == nullptr)
			return nullptr;
	}
	if (pkgstr == nullptr) {
		pkgstr = alifUStr_internFromString("__package__");
		if (pkgstr == nullptr)
			return nullptr;
	}

	*buf = '\0';
	*p_buflen = 0;
	pkgname = alifDict_getItemWithError(globals, pkgstr);

	if ((pkgname != nullptr) and (pkgname != ALIF_NONE)) {
		/* __package__ is set, so use it */
		AlifSizeT len{};
		if (!ALIFUSTR_CHECK(pkgname)) {
			//alifErr_setString(_alifExcValueError_,
			//	"__package__ set to non-string");
			return nullptr;
		}
		//len = ALIFUSTR_GET_LENGTH(pkgname);
		len = strlen(alifUStr_asUTF8(pkgname)); //* alif
		if (len == 0) {
			if (level > 0) {
				//alifErr_setString(_alifExcValueError_,
				//	"Attempted relative import in non-package");
				return nullptr;
			}
			return ALIF_NONE;
		}
		if (len > MAXPATHLEN) {
			//alifErr_setString(_alifExcValueError_,
			//	"Package name too long");
			return nullptr;
		}
		strcpy(buf, alifUStr_asUTF8(pkgname));
	}
	else {
		/* __package__ not set, so figure it out and set it */
		modname = alifDict_getItemWithError(globals, namestr);
		if (modname == nullptr or !ALIFUSTR_CHECK(modname))
			return ALIF_NONE;

		modpath = alifDict_getItemWithError(globals, pathstr);
		if (modpath != nullptr) {
			/* __path__ is set, so modname is already the package name */
			//AlifSizeT len = ALIFUSTR_GET_LENGTH(modname); // ALIFUSTR_GET_LENGTH return num of letters and we need num of bytes in this sys
			AlifSizeT len = strlen(alifUStr_asUTF8(modname)); //* alif
			int error{};
			if (len > MAXPATHLEN) {
				//alifErr_setString(_alifExcValueError_,
				//	"Module name too long");
				return nullptr;
			}
			strcpy(buf, alifUStr_asUTF8(modname));
			error = alifDict_setItem(globals, pkgstr, modname);
			if (error) {
				//alifErr_setString(_alifExcValueError_,
				//	"Could not set __package__");
				return nullptr;
			}
		}
		else {
			/* Normal module, so work out the package name if any */
			const char* start = alifUStr_asUTF8(modname);
			const char* lastdot = strrchr(start, '.');
			AlifUSizeT len{};
			AlifIntT error{};
			if (lastdot == nullptr and level > 0) {
				//alifErr_setString(_alifExcValueError_,
				//	"Attempted relative import in non-package");
				return nullptr;
			}
			if (lastdot == nullptr) {
				error = alifDict_setItem(globals, pkgstr, ALIF_NONE);
				if (error) {
					//alifErr_setString(_alifExcValueError_,
					//	"Could not set __package__");
					return nullptr;
				}
				return ALIF_NONE;
			}
			len = lastdot - start;
			if (len >= MAXPATHLEN) {
				//alifErr_setString(_alifExcValueError_,
				//	"Module name too long");
				return nullptr;
			}
			strncpy(buf, start, len);
			buf[len] = '\0';
			pkgname = alifUStr_fromString(buf);
			if (pkgname == nullptr) {
				return nullptr;
			}
			error = alifDict_setItem(globals, pkgstr, pkgname);
			ALIF_DECREF(pkgname);
			if (error) {
				//alifErr_setString(_alifExcValueError_,
				//	"Could not set __package__");
				return nullptr;
			}
		}
	}
	while (--level > 0) {
		char* dot = strrchr(buf, '.');
		if (dot == nullptr) {
			//alifErr_setString(_alifExcValueError_,
			//	"Attempted relative import beyond "
			//	"toplevel package");
			return nullptr;
		}
		*dot = '\0';
	}
	*p_buflen = strlen(buf);

	modules = alifImport_getModuleDict();
	parent = alifDict_getItemWithError(modules, alifUStr_fromString(buf));
	if (parent == nullptr) {
		if (orig_level < 1) {
			//AlifObject* err_msg = alifString_fromFormat(
			//	"Parent module '%.200s' not found "
			//	"while handling absolute import", buf);
			//if (err_msg == nullptr) {
			//	return nullptr;
			//}
			//if (!alifErr_warnEx(_alifExcRuntimeWarning_,
			//	alifUStr_asUTF8(err_msg), 1)) {
			//	*buf = '\0';
			//	*p_buflen = 0;
			//	parent = ALIF_NONE;
			//}
			//ALIF_DECREF(err_msg);
		}
		else {
			//alifErr_format(_alifExcSystemError_,
			//	"Parent module '%.200s' not loaded, "
			//	"cannot perform relative import", buf);
		}
	}
	return parent;
}


static AlifObject* load_next(AlifObject* mod, AlifObject* altmod,
	const char** p_name, char* buf, AlifSizeT* p_buflen) { // 2371
	const char* name = *p_name;
	const char* dot = strchr(name, '.');
	AlifUSizeT len{};
	char* p{};
	AlifObject* result{};

	if (strlen(name) == 0) {
		/* completely empty module name should only happen in
		   'من . استورد' (or '_استورد_("")')*/
		ALIF_INCREF(mod);
		*p_name = nullptr;
		return mod;
	}

	if (dot == nullptr) {
		*p_name = nullptr;
		len = strlen(name);
	}
	else {
		*p_name = dot + 1;
		len = dot - name;
	}
	if (len == 0) {
		//alifErr_setString(_alifExcValueError_,
		//	"Empty module name");
		return nullptr;
	}

	p = buf + *p_buflen;
	if (p != buf) {
		*p++ = '.';
	}
	if (p + len - buf >= MAXPATHLEN) {
		//alifErr_setString(_alifExcValueError_,
		//	"Module name too long");
		return nullptr;
	}
	strncpy(p, name, len);
	p[len] = '\0';
	*p_buflen = p + len - buf;

	result = import_submodule(mod, p, buf);
	if (result == ALIF_NONE and altmod != mod) {
		ALIF_DECREF(result);
		/* Here, altmod must be None and mod must not be None */
		result = import_submodule(altmod, p, p);
		if (result != nullptr and result != ALIF_NONE) {
			if (mark_miss(buf) != 0) {
				ALIF_DECREF(result);
				return nullptr;
			}
			strncpy(buf, name, len);
			buf[len] = '\0';
			*p_buflen = len;
		}
	}
	if (result == nullptr)
		return nullptr;

	if (result == ALIF_NONE) {
		ALIF_DECREF(result);
		//alifErr_format(_alifExcImportError_,
		//	"No module named %.200s", name);
		return nullptr;
	}

	return result;
}


static AlifIntT mark_miss(char* name) { // 2443
	AlifObject* modules = alifImport_getModuleDict();
	return alifDict_setItemString(modules, name, ALIF_NONE);
}

static AlifIntT ensure_fromList(AlifObject* mod, AlifObject* fromlist,
	char* buf, AlifSizeT buflen, AlifIntT recursive) { // 2450
	AlifIntT i{};

	if (!alifObject_hasAttrWithError(mod, &ALIF_ID(__path__)))
		return 1;

	for (i = 0; ; i++) {
		AlifObject* item = alifSequence_getItem(fromlist, i);
		AlifIntT hasit{};
		if (item == nullptr) {
			if (alifErr_exceptionMatches(_alifExcIndexError_)) {
				alifErr_clear();
				return 1;
			}
			return 0;
		}
		if (!ALIFUSTR_CHECK(item)) {
			//alifErr_setString(_alifExcTypeError_,
			//	"Item in ``from list'' not a string");
			ALIF_DECREF(item);
			return 0;
		}
		if (alifUStr_asUTF8(item)[0] == '*') {
			AlifObject* all{};
			ALIF_DECREF(item);
			/* See if the package defines __all__ */
			if (recursive)
				continue; /* Avoid endless recursion */
			all = alifObject_getAttrString(mod, "__all__");
			if (all == nullptr) {
				//alifErr_clear();
			}
			else {
				int ret = ensure_fromList(mod, all, buf, buflen, 1);
				ALIF_DECREF(all);
				if (!ret)
					return 0;
			}
			continue;
		}
		hasit = alifObject_hasAttrWithError(mod, item);
		if (!hasit) {
			const char* subname = alifUStr_asUTF8(item);
			AlifObject* submod{};
			char* p;
			if (buflen + strlen(subname) >= MAXPATHLEN) {
				//alifErr_setString(_alifExcValueError_,
				//	"Module name too long");
				ALIF_DECREF(item);
				return 0;
			}
			p = buf + buflen;
			*p++ = '.';
			strcpy(p, subname);
			submod = import_submodule(mod, subname, buf);
			ALIF_XDECREF(submod);
			if (submod == nullptr) {
				ALIF_DECREF(item);
				return 0;
			}
		}
		ALIF_DECREF(item);
	}

	/* NOTREACHED */
}


static AlifIntT add_submodule(AlifObject* mod, AlifObject* submod,
	char* fullname, const char* subname, AlifObject* modules) { // 2519
	if (mod == ALIF_NONE)
		return 1;
	if (submod == nullptr) {
		submod = alifDict_getItemWithError(modules, alifUStr_fromString(fullname));
		if (submod == nullptr)
			return 1;
	}

	if (ALIFMODULE_CHECK(mod)) {
		/* We can't use setattr here since it can give a
		 * spurious warning if the submodule name shadows a
		 * builtin name */
		AlifObject* dict = alifModule_getDict(mod);
		if (!dict)
			return 0;
		if (alifDict_setItemString(dict, subname, submod) < 0)
			return 0;
	}
	else {
		if (alifObject_setAttrString(mod, subname, submod) < 0)
			return 0;
	}
	return 1;
}

static AlifObject* import_submodule(AlifObject* mod,
	const char* subname, char* fullname) { // 2553
	AlifObject* modules = alifImport_getModuleDict();
	AlifObject* m = nullptr;

	/* Require:
	   if mod == None: subname == fullname
	   else: mod.__name__ + "." + subname == fullname
	*/

	if ((m = alifDict_getItemWithError(modules, alifUStr_fromString(fullname))) != nullptr) {
		ALIF_INCREF(m);
	}
	else {
		AlifObject* path, * loader = nullptr;
		char buf[MAXPATHLEN + 1]{};
		FileDescr* fdp{};
		FILE* fp = nullptr;

		if (mod == ALIF_NONE)
			path = nullptr;
		else {
			path = alifObject_getAttrString(mod, "__path__");
			if (path == nullptr) {
				//alifErr_clear();
				ALIF_INCREF(ALIF_NONE);
				return ALIF_NONE;
			}
		}

		buf[0] = '\0';
		fdp = find_module(fullname, subname, path, buf, MAXPATHLEN + 1,
			&fp, &loader);
		ALIF_XDECREF(path);
		if (fdp == nullptr) {
			if (!alifErr_exceptionMatches(_alifExcImportError_))
				return nullptr;
			//alifErr_clear();
			ALIF_INCREF(ALIF_NONE);
			return ALIF_NONE;
		}
		m = load_module(fullname, fp, buf, fdp->type, loader);
		ALIF_XDECREF(loader);
		if (fp)
			fclose(fp);
		if (!add_submodule(mod, m, fullname, subname, modules)) {
			ALIF_XDECREF(m);
			m = nullptr;
		}
	}

	return m;
}






//AlifObject* alifImport_import(AlifObject* _moduleName) { // 2717
//	static AlifObject* silly_list = nullptr;
//	static AlifObject* builtins_str = nullptr;
//	static AlifObject* import_str = nullptr;
//	AlifObject* globals = nullptr;
//	AlifObject* import = nullptr;
//	AlifObject* builtins = nullptr;
//	AlifObject* r = nullptr;
//
//	/* Initialize constant string objects */
//	if (silly_list == nullptr) {
//		import_str = alifUStr_internFromString("__import__");
//		if (import_str == nullptr)
//			return nullptr;
//		builtins_str = alifUStr_internFromString("__builtins__");
//		if (builtins_str == nullptr)
//			return nullptr;
//		silly_list = alif_buildValue("[s]", "__doc__");
//		if (silly_list == nullptr)
//			return nullptr;
//	}
//
//	/* Get the builtins from current globals */
//	globals = alifEval_getGlobals();
//	if (globals != nullptr) {
//		ALIF_INCREF(globals);
//		builtins = alifObject_getItem(globals, builtins_str);
//		if (builtins == nullptr)
//			goto err;
//	}
//	else {
//		/* No globals -- use standard builtins, and fake globals */
//		builtins = alifImport_importModuleLevel("__builtin__",
//			nullptr, nullptr, nullptr, 0);
//		if (builtins == nullptr)
//			return nullptr;
//		globals = alif_buildValue("{OO}", builtins_str, builtins);
//		if (globals == nullptr)
//			goto err;
//	}
//
//	/* Get the __import__ function from the builtins */
//	if (ALIFDICT_CHECK(builtins)) {
//		import = alifObject_getItem(builtins, import_str);
//		if (import == nullptr) {
//			//alifErr_setObject(_alifExcKeyError_, import_str);
//		}
//	}
//	else {
//		import = alifObject_getAttr(builtins, import_str);
//	}
//		if (import == nullptr) goto err;
//
//	/* Call the __import__ function with the proper argument list
//	 * Always use absolute import here. */
//	r = alifObject_callFunction(import, "OOOOi", _moduleName, globals,
//		globals, silly_list, 0, nullptr);
//
//err:
//	ALIF_XDECREF(globals);
//	ALIF_XDECREF(builtins);
//	ALIF_XDECREF(import);
//
//	return r;
//}
