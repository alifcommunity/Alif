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

//* alif
#include "AlifCore_Compile.h"
#include "OSDefs.h"
#ifndef _WINDOWS
#include <dirent.h>
#include <sys/types.h>
#endif

static AlifObject* load_sourceImpl(AlifObject*); // alif

//* alif








extern InitTable _alifImportInitTab_[]; // 55

InitTable* _alifImportInitTable_ = _alifImportInitTab_; // 59

#define INITTABLE _alifDureRun_.imports.initTable // 69
#define LAST_MODULE_INDEX _alifDureRun_.imports.lastModuleIndex // 70
//#define EXTENSIONS _alifDureRun_.imports.extensions // 71

// 80
#define MODULES(_interp) \
    (_interp)->imports.modules
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
#ifndef HAVE_THREAD_LOCAL
	//alifThread_acquireLock(EXTENSIONS.mutex, WAIT_LOCK);
#endif
	const char* oldcontext = PKGCONTEXT;
	PKGCONTEXT = newcontext;
#ifndef HAVE_THREAD_LOCAL
	//alifThread_releaseLock(EXTENSIONS.mutex);
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




static AlifObject* import_runExtension(AlifThread* tstate, AlifModInitFunction p0,
	AlifExtModuleLoaderInfo* info, AlifObject* spec, AlifObject* modules) { // 1919

	AlifObject* mod = nullptr;
	AlifModuleDef* def = nullptr;
	//ExtensionsCacheValue* cached = nullptr;
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

			//SinglephaseGlobalUpdate singlephase = {
			//	.index = def->base.index,
			//	.origin = info->origin,
			//	.gil = ((AlifModuleObject*)mod)->gil,
			//};
			//if (def->size == -1) {
			//	singlephase.dict = alifModule_getDict(mod);
			//}
			//else {
			//	singlephase.m_init = p0;
			//}
			//cached = updateGlobalState_forExtension(
			//	main_tstate, info->path, info->name, def, &singlephase);
			//if (cached == nullptr) {
			//	goto main_finally;
			//}
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
		//alifExtModuleLoader_resultApplyError(&res, name_buf);
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

		//if (switched) {
		//	mod = reload_singlephaseExtension(tstate, cached, info);
		//	if (mod == nullptr) {
		//		goto error;
		//	}
		//}
		//else {
		//	AlifObject* modules = get_modulesDict(tstate, true);
		//	if (finish_singlephaseExtension(
		//		tstate, mod, cached, info->name, modules) < 0)
		//	{
		//		goto error;
		//	}
		//}
	}

	_alifExtModule_loaderResultClear(&res);
	return mod;

error:
	ALIF_XDECREF(mod);
	_alifExtModule_loaderResultClear(&res);
	return nullptr;
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
	AlifObject* mod{}; //* alif
	InitTable* found = nullptr; //* alif
	AlifModInitFunction p0{}; //* alif


	if (_alifExtModule_loaderInfoInitForBuiltin(&info, _name) < 0) {
		return nullptr;
	}

	//ExtensionsCacheValue* cached = nullptr;
	//AlifObject* mod = import_findExtension(_thread, &info, &cached);
	//if (mod != nullptr) {
	//	goto finally;
	//}
	/*else */if (_alifErr_occurred(_thread)) {
		goto finally;
	}

	//if (cached != nullptr) {
	//	_extensions_cacheDelete(info.path, info.name);
	//}

	
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

	AlifObject* sysPath = alifSys_getObject("path");
	AlifObject* sysMetaPath = alifSys_getObject("meta_path");
	AlifObject* sysPathHooks = alifSys_getObject("path_hooks");
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
		//_alifErr_setString(tstate, _alifExcValueError_, "Empty module name");
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
		//absName = resolve_name(tstate, name, globals, level);
		if (absName == nullptr)
			goto error;
	}
	else {  /* level == 0 */
		if (ALIFUSTR_GET_LENGTH(name) == 0) {
			//_alifErr_setString(tstate, _alifExcValueError_, "Empty module name");
			goto error;
		}
		absName = ALIF_NEWREF(name);
	}

	mod = import_getModule(thread, absName);
	if (mod == nullptr and _alifErr_occurred(thread)) {
		goto error;
	}

	if (mod != nullptr and mod != ALIF_NONE) {
		//if (import_ensureInitialized(thread->interpreter, mod, abs_name) < 0) {
		//	goto error;
		//}
	}
	else {
		ALIF_XDECREF(mod);
		//mod = import_findAndLoad(thread, absName);

		//* alif
		mod = load_sourceImpl(absName);
		//* alif

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
					//if (!_alifErr_occurred(tstate)) {
					//	_alifErr_format(tstate, _alifExcKeyError_,
					//		"%R not in sys.modules as expected",
					//		to_return);
					//}
					goto error;
				}
			}
		}
		else {
			finalMod = ALIF_NEWREF(mod);
		}
	}
	else {
		AlifIntT hasPath = alifObject_hasAttrWithError(mod, &ALIF_ID(__path__));
		if (hasPath < 0) {
			goto error;
		}
		if (hasPath) {
			//final_mod = alifObject_callMethodObjArgs(
			//	IMPORTLIB(interp), &ALIF_ID(_handleFromList),
			//	mod, fromlist, IMPORT_FUNC(interp), nullptr);
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
		//remove_importLibFrames(tstate);
	}
	return finalMod;
}





// alif
AlifIntT alifImport_init() { // 3954

	if (INITTABLE != nullptr) {
		// error
		return -1; //* alif
	}

	if (initBuildin_modulesTable() != 0) {
		return -1;
	}

	return 1;
}









AlifIntT _alifImport_initCore(AlifThread* _thread,
	AlifObject* _sysmod, AlifIntT _importLib) { // 4026
	// XXX Initialize here: interp->modules and interp->importFunc.
	// XXX Initialize here: sys.modules and sys.meta_path.

	if (_importLib) {
		if (init_importLib(_thread, _sysmod) < 0) {
			return -1;
		}
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


AlifObject* alifImport_getModuleDict(void) { // 364
	AlifInterpreter* interp = alifInterpreter_get();
	if (interp->imports.modules == nullptr) {
		//alif_fatalError("alifImport_getModuleDict: no module dictionary!");
	}
	return interp->imports.modules;
}

AlifObject* alifImport_execCodeModuleEx(const char* name, AlifObject* co, char* pathname) { // 649
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
	/* Remember the filename as the __file__ attribute */
	if (pathname != nullptr) {
		v = alifUStr_fromString(pathname);
		if (v == nullptr) {
			//alifErr_clear();
		}
	}
	if (v == nullptr) {
		v = ((AlifCodeObject*)co)->filename;
		ALIF_INCREF(v);
	}
	if (alifDict_setItem(d, &ALIF_ID(__file__), v) != 0) {
		//alifErr_clear();
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


static AlifCodeObject* parse_sourceModule(const char* pathname, FILE* fp) { // 817 // import27.c
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
		AlifSizeT len = strlen(pathname);
		AlifObject* pathObj = alifUStr_fromStringAndSize(pathname, len);
		co = _alifAST_compile(mod, pathObj, &flags, 0, astMem);
	}
	alifASTMem_free(astMem);
	return co;
}


static AlifObject* load_sourceModule(char* _name, char* _pathname, FILE* _fp) { // 966
	FILE* fpc{};
	AlifCodeObject* co{};
	AlifObject* m{};

	co = parse_sourceModule(_pathname, _fp);
	if (co == nullptr)
		return nullptr;

	m = alifImport_execCodeModuleEx(_name, (AlifObject*)co, _pathname);
	ALIF_DECREF(co);

	return m;
}




static FILE* open_file(char* pathname, const char* mode) { // 2938
	FILE* fp{};
	fp = fopen(pathname, mode);
	if (fp == nullptr) {
		//alifErr_setFromErrno(_alifExcIOError_);
	}
	return fp;
}

static void path_addFile(char* path, const char* extension) {
	AlifUSizeT pathLen = strlen(path);

	// Check if the path already has an extension
	const char* dot = strrchr(path, '.');
	if (dot and dot > strrchr(path, '/') and dot > strrchr(path, '\\')) {
		// Replace the existing extension with the new one
		strcpy((char*)dot + 1, extension);
	}
	else {
		// If there is no extension, append it
		strcat(path, ".");
		strcat(path, extension);
	}
}

static AlifIntT get_absPath(const char* _name, char* _pathname, AlifUSizeT _pathNameSize) {
	if (_name == nullptr) return -1;

#ifdef _WINDOWS
	if (_fullpath(_pathname, _name, _pathNameSize) != nullptr) {
		return 1;
	}
#else
	if (getcwd(_pathname, _pathNameSize) == nullptr) {
		printf("لم يستطع جلب مسار الملف خلال الاستيراد");
		return -1;
	}

	strcat(_pathname, "/");
	strcat(_pathname, _name);

	if (_pathname != nullptr) {
		return 1;
	}

#endif
	return -1;
}

static const char* get_fileExtension(const char* filename) {
	const char* dot = strrchr(filename, '.');
	if (!dot or dot == filename) return "";
	return dot + 1;
}

static char* get_baseName(const char* filename) {
	const char* dot = strrchr(filename, '.');

	if (!dot or dot == filename) return strdup(filename);
	AlifUSizeT len = dot - filename;
	char* baseName = (char*)alifMem_dataAlloc(len + 1);
	if (baseName == nullptr) {
		return nullptr;
	}

	strncpy(baseName, filename, len);
	baseName[len] = '\0';

	return baseName;
}

static AlifIntT find_file(const char* name, char* pathname) {

#ifdef _WINDOWS
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = FindFirstFileA("*", &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "FindFirstFile failed (%lu)\n", GetLastError());
		return -1;
	}

	BOOL bFound = true;
	do {
		const char* filename = findFileData.cFileName;
		char* baseName = get_baseName(filename);
		if (baseName == nullptr) {
			return -1;
		}
		if (strcmp(baseName, name) == 0) {
			const char* extension = get_fileExtension(filename);
			if (strcmp(extension, "aliflib") == 0
				/*or strcmp(extension, "alif") == 0
				or strcmp(extension, "alifm") == 0*/) {
				char newFilename[MAXPATHLEN]{};
				snprintf(newFilename, sizeof(newFilename), "%s.%s", baseName, extension);
				path_addFile(pathname, extension);
			}
			alifMem_dataFree(baseName);
			FindClose(hFind);
			return 1;
		}
		bFound = FindNextFileA(hFind, &findFileData);
	} while (bFound);

	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		fprintf(stderr, "FindNextFile failed (%lu)\n", dwError);
		FindClose(hFind);
		return -1;
	}

	FindClose(hFind);
	fprintf(stderr, "File not found: %s\n", name);
	return -1;

#else
	DIR* dir = opendir(".");
	if (!dir) {
		perror("opendir");
		return -1;
	}

	struct dirent* entry{};
	while ((entry = readdir(dir)) != nullptr) {
		if (entry->d_type == DT_REG) { // Check if it's a regular file
			char* baseName = get_baseName(entry->d_name);
			if (baseName == nullptr) {
				return -1;
			}
			if (strcmp(baseName, name) == 0) {
				const char* extension = get_fileExtension(entry->d_name);
				if (strcmp(extension, "aliflib") == 0
					/*or strcmp(extension, "alif") == 0
					or strcmp(extension, "alifm") == 0*/) {
					char newFilename[MAXPATHLEN]{};
					snprintf(newFilename, sizeof(newFilename), "%s.%s", baseName, extension);
					path_addFile(pathname, extension);
				}
				alifMem_dataFree(baseName);
				closedir(dir);
				return 1;
			}
		}
	}

	closedir(dir);
	fprintf(stderr, "File not found: %s\n", name);
	return -1;

#endif
}



static AlifObject* load_sourceImpl(AlifObject* _absName) { // 3002 // import27.c
	char* name{};
	char pathname[MAXPATHLEN]{};
	AlifObject* m{};
	FILE* fp{};

	name = (char*)alifUStr_asUTF8(_absName);
	if (name == nullptr) {
		return nullptr;
	}

	if (is_builtin(_absName)) {
		AlifThread* thread = _alifThread_get();

		AlifObject* attrs = alif_buildValue("{sO}", "name", _absName);
		if (attrs == nullptr) {
			return nullptr;
		}
		AlifObject* spec = alifNamespace_new(attrs);
		ALIF_DECREF(attrs);
		if (spec == nullptr) {
			return nullptr;
		}

		AlifObject* mod = create_builtin(thread, _absName, spec);
		ALIF_DECREF(spec);
		if (mod == nullptr) {
			return nullptr;
		}

		if (exec_builtinOrDynamic(mod) < 0) {
			ALIF_DECREF(mod);
			return nullptr;
		}

		return mod;
	}


	if (get_absPath(name, pathname, MAXPATHLEN) < 0) {
		printf("لم يستطع جلب المسار اثناء الاستيراد");
		return nullptr;
	}

	// البحث عن الملف ضمن الملفات في المسار الحالي وإضافته لمسار
	if (find_file(name, pathname) < 0) {
		return nullptr;
	}

	fp = open_file(pathname, "r");
	if (fp == nullptr) return nullptr;

	m = load_sourceModule(name, pathname, fp);

	if (fp != nullptr) fclose(fp);

	return m;
}





