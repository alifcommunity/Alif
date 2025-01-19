#include "alif.h"

#include "AlifCore_Import.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_ObjectDeferred.h" // هذا التضمين غير ضروري ويجب حذفه مع التقدم في البرنامج

#include "OSDefs.h"




AlifTypeObject _alifModuleDefType_ = { // 24
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	"تعريف-الوحدة",
	sizeof(AlifModuleDef),
	0,
};








AlifObject* alifModuleDef_init(AlifModuleDef* def) { // 45
	if (def->base.index == 0) {
		ALIF_SET_REFCNT(def, 1);
		ALIF_SET_TYPE(def, &_alifModuleDefType_);
		def->base.index = alifImport_getNextModuleIndex();
	}
	return (AlifObject*)def;
}


static AlifIntT module_initDict(AlifModuleObject* mod, AlifObject* md_dict,
	AlifObject* name, AlifObject* doc) { // 57
	if (doc == nullptr)
		doc = ALIF_NONE;

	if (alifDict_setItem(md_dict, &ALIF_ID(__name__), name) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_ID(__doc__), doc) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_ID(__package__), ALIF_NONE) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_ID(__loader__), ALIF_NONE) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_ID(__spec__), ALIF_NONE) != 0)
		return -1;
	if (ALIFUSTR_CHECKEXACT(name)) {
		ALIF_XSETREF(mod->name, ALIF_NEWREF(name));
	}

	return 0;
}


static AlifModuleObject* newModule_noTrack(AlifTypeObject* mt) { // 82
	AlifModuleObject* m{};
	m = (AlifModuleObject*)alifType_allocNoTrack(mt, 0);
	if (m == nullptr) return nullptr;

	m->def = nullptr;
	m->state = nullptr;
	m->weaklist = nullptr;
	m->name = nullptr;
	m->dict = alifDict_new();
	if (m->dict == nullptr) {
		ALIF_DECREF(m);
		return nullptr;
	}
	return m;
}


static void track_module(AlifModuleObject* _m) { // 101
	alifObject_setDeferredRefcount(_m->dict);
	alifObject_gcTrack(_m->dict);

	alifObject_setDeferredRefcount((AlifObject*)_m);
	alifObject_gcTrack(_m);
}

AlifObject* alifModule_newObject(AlifObject* _name) { // 121
	AlifModuleObject* m = newModule_noTrack(&_alifModuleType_);
	if (m == nullptr) return nullptr;

	if (module_initDict(m, m->dict, _name, nullptr) != 0)
		goto fail;
	track_module(m);
	return (AlifObject*)m;

fail:
	ALIF_DECREF(m);
	return nullptr;
}




AlifObject* alifModule_new(const char* name) { // 137
	AlifObject* nameObj{}, * module{};
	nameObj = alifUStr_fromString(name);
	if (nameObj == nullptr)
		return nullptr;
	module = alifModule_newObject(nameObj);
	ALIF_DECREF(nameObj);
	return module;
}



static AlifIntT addMethods_toObject(AlifObject* _module,
	AlifObject* _name, AlifMethodDef* _functions) { // 169
	AlifObject* func{};
	AlifMethodDef* fdef{};

	for (fdef = _functions; fdef->name != nullptr; fdef++) {
		if ((fdef->flags & METHOD_CLASS) or
			(fdef->flags & METHOD_STATIC)) {
			//alifErr_setString(_alifExcValueError_,
			//	"module functions cannot set"
			//	" METH_CLASS or METH_STATIC");
			return -1;
		}
		func = ALIFCPPFUNCTION_NEWEX(fdef, (AlifObject*)_module, _name);
		if (func == nullptr) {
			return -1;
		}
		alifObject_setDeferredRefcount(func);
		if (alifObject_setAttrString(_module, fdef->name, func) != 0) {
			ALIF_DECREF(func);
			return -1;
		}
		ALIF_DECREF(func);
	}

	return 0;
}


AlifObject* alifModule_createInitialized(AlifModuleDef* _module) { // 209

	const char* name{};
	AlifModuleObject* m{};

	if (!alifModuleDef_init(_module)) return nullptr;

	name = _module->name;
	if (_module->slots) {
		// error
		return nullptr;
	}
	name = alifImport_resolveNameWithPackageContext(name);
	if ((m = (AlifModuleObject*)alifModule_new(name)) == nullptr) return nullptr;

	if (_module->size > 0) {
		m->state = alifMem_dataAlloc(_module->size);
		if (!m->state) {
			//alifErr_noMemory();
			ALIF_DECREF(m);
			return nullptr;
		}
		memset(m->state, 0, _module->size);
	}

	if (_module->methods != nullptr) {
		if (alifModule_addFunctions((AlifObject*)m, _module->methods) != 0) {
			ALIF_DECREF(m);
			return nullptr;
		}
	}
	m->def = _module;
	m->gil = ALIF_MOD_GIL_USED;

	return (AlifObject*)m;
}




AlifIntT alifUnstable_moduleSetGIL(AlifObject* _module, void* _gil) { // 442
	if (!ALIFMODULE_CHECK(_module)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	((AlifModuleObject*)_module)->gil = _gil;
	return 0;
}





AlifIntT alifModule_addFunctions(AlifObject* _m,
	AlifMethodDef* _functions) { // 523
	AlifIntT res{};
	AlifObject* name = alifModule_getNameObject(_m);
	if (name == nullptr) {
		return -1;
	}

	res = addMethods_toObject(_m, name, _functions);
	ALIF_DECREF(name);
	return res;
}


AlifObject* alifModule_getDict(AlifObject* _m) { // 551
	if (!ALIFMODULE_CHECK(_m)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	return _alifModule_getDict(_m);  // borrowed reference
}


AlifObject* alifModule_getNameObject(AlifObject* _mod) { // 561
	AlifObject* name{};

	if (!ALIFMODULE_CHECK(_mod)) {
		//alifErr_badArgument();
		return nullptr;
	}
	AlifObject* dict = ((AlifModuleObject*)_mod)->dict;  // borrowed reference
	if (dict == nullptr or !ALIFDICT_CHECK(dict)) {
		goto error;
	}
	// name
	if (alifDict_getItemRef(dict, &ALIF_ID(__name__), &name) <= 0) {
		// error or not found
		goto error;
	}
	if (!ALIFUSTR_CHECK(name)) {
		ALIF_DECREF(name);
		goto error;
	}
	return name;

error:
	//if (!alifErr_occurred()) {
	//	alifErr_setString(_alifExcSystemError_, "nameless module");
	//}
	return nullptr;
}



AlifObject* alifModule_getFilenameObject(AlifObject* _mod) { // 602
	AlifObject* fileobj{};

	if (!ALIFMODULE_CHECK(_mod)) {
		//ALIFERR_BADARGUMENT();
		return nullptr;
	}
	AlifObject* dict = ((AlifModuleObject*)_mod)->dict;  // borrowed reference
	if (dict == nullptr) {
		goto error;
	}
	if (alifDict_getItemRef(dict, &ALIF_ID(__file__), &fileobj) <= 0) {
		// error or not found
		goto error;
	}
	if (!ALIFUSTR_CHECK(fileobj)) {
		ALIF_DECREF(fileobj);
		goto error;
	}
	return fileobj;

error:
	if (!alifErr_occurred()) {
		//alifErr_setString(_alifExcSystemError_, "module filename missing");
	}
	return nullptr;
}


AlifIntT _alifModuleSpec_isInitializing(AlifObject* _spec) { // 793
	if (_spec == nullptr) {
		return 0;
	}
	AlifObject* value{};
	AlifIntT rc = alifObject_getOptionalAttr(_spec, &ALIF_ID(_initializing), &value);
	if (rc > 0) {
		rc = alifObject_isTrue(value);
		ALIF_DECREF(value);
	}
	return rc;
}



static AlifIntT getFileOrigin_fromSpec(AlifObject* _spec, AlifObject** _pOrigin) { // 828
	AlifObject* hasLocation = nullptr;
	AlifIntT rc_ = alifObject_getOptionalAttr(_spec, &ALIF_ID(HasLocation), &hasLocation);
	if (rc_ <= 0) {
		return rc_;
	}
	rc_ = alifObject_isTrue(hasLocation);
	ALIF_DECREF(hasLocation);
	if (rc_ <= 0) {
		return rc_;
	}
	AlifObject* origin = nullptr;
	rc_ = alifObject_getOptionalAttr(_spec, &ALIF_ID(Origin), &origin);
	if (rc_ <= 0) {
		return rc_;
	}
	if (!ALIFUSTR_CHECK(origin)) {
		ALIF_DECREF(origin);
		return 0;
	}
	*_pOrigin = origin;
	return 1;
}

static AlifIntT isModule_possiblyShadowing(AlifObject* _origin) { // 859
	if (_origin == nullptr) {
		return 0;
	}

	const AlifConfig* config = alif_getConfig();
	if (config->safePath) {
		return 0;
	}

	wchar_t root[MAXPATHLEN + 1];
	AlifSizeT size = alifUStr_asWideChar(_origin, root, MAXPATHLEN);
	if (size < 0) {
		return -1;
	}
	root[size] = L'\0';

	wchar_t* sep_ = wcsrchr(root, SEP);
	if (sep_ == nullptr) {
		return 0;
	}

	if (wcscmp(sep_ + 1, L"__init__.alif") == 0) {
		*sep_ = L'\0';
		sep_ = wcsrchr(root, SEP);
		if (sep_ == nullptr) {
			return 0;
		}
	}
	*sep_ = L'\0';

	wchar_t* sysPath0 = config->sysPath0;
	if (!sysPath0) {
		return 0;
	}

	wchar_t sysPath0Buf[MAXPATHLEN];
	if (sysPath0[0] == L'\0') {
		if (!alif_wGetCWD(sysPath0Buf, MAXPATHLEN)) {
			return -1;
		}
		sysPath0 = sysPath0Buf;
	}

	AlifIntT result = wcscmp(sysPath0, root) == 0;
	return result;
}

AlifObject* alifModule_getAttroImpl(AlifModuleObject* _m,
	AlifObject* _name, AlifIntT _suppress) { // 921
	AlifObject* attr{}, * modName{}, * getAttr{};
	AlifIntT isPossiblyShadowing{};
	AlifIntT isPossiblyShadowingStdLib = 0;

	attr = alifObject_genericGetAttrWithDict((AlifObject*)_m, _name, nullptr, _suppress);
	if (attr) {
		return attr;
	}
	if (_suppress == 1) {
		//if (alifErr_occurred()) {
			// pass up non-AttributeError exception
			//return nullptr;
		//}
	}
	else {
		//if (!alifErr_exceptionMatches(_alfiExcAttributeError_)) {
			// pass up non-AttributeError exception
			//return nullptr;
		//}
		//alifErr_clear();
	}
	if (alifDict_getItemRef(_m->dict, &ALIF_ID(__getAttr__), &getAttr) < 0) {
		return nullptr;
	}
	if (getAttr) {
		//AlifObject* result = alifObject_callOneArg(getAttr, _name);
		//if (result == nullptr and _suppress == 1
		//	//and alifErr_exceptionMatches(exception)
		//	) {
		//	//alifErr_clear();
		//	return nullptr;
		//}
		//ALIF_DECREF(getAttr);
		//return result;
	}
	if (_suppress == 1) {
		return nullptr;
	}
	if (alifDict_getItemRef(_m->dict, &ALIF_ID(__name__), &modName) < 0) {
		return nullptr;
	}
	if (!modName or !ALIFUSTR_CHECK(modName)) {
		ALIF_XDECREF(modName);
		//alifErr_format(_alifExcAttributeError_,
			//"module has no attribute '%U'", _name);
		return nullptr;
	}
	AlifObject* spec{};
	if (alifDict_getItemRef(_m->dict, &ALIF_ID(__spec__), &spec) < 0) {
		ALIF_DECREF(modName);
		return nullptr;
	}
	if (spec == nullptr) {
		//alifErr_format(_alifExcAttributeError_,
			//"module '%U' has no attribute '%U'",
			//modName, _name);
		ALIF_DECREF(modName);
		return nullptr;
	}

	AlifObject* origin = nullptr;
	if (getFileOrigin_fromSpec(spec, &origin) < 0) {
		goto done;
	}

	isPossiblyShadowing = isModule_possiblyShadowing(origin);
	if (isPossiblyShadowing < 0) {
		goto done;
	}
	isPossiblyShadowingStdLib = 0;
	if (isPossiblyShadowing) {
		AlifObject* stdlibModules = alifSys_getObject("stdlib_module_names");
		if (stdlibModules and ALIFANYSET_CHECK(stdlibModules)) {
			//isPossiblyShadowingStdLib = alifSet_contains(stdlibModules, modName);
			if (isPossiblyShadowingStdLib < 0) {
				goto done;
			}
		}
	}

	if (isPossiblyShadowingStdLib) {
		//alifErr_format(_alifExcAttributeError_,
		//	"module '%U' has no attribute '%U' "
		//	"(consider renaming '%U' since it has the same "
		//	"name as the standard library module named '%U' "
		//	"and the import system gives it precedence)",
		//	modName, name, origin, modName);
	}
	else {
		//AlifIntT rc_ = alifModuleSpec_isInitializing(spec);
		//if (rc_ > 0) {
		//	if (isPossiblyShadowing) {
		//	//	alifErr_format(_alifExcAttributeError_,
		//				//"module '%U' has no attribute '%U' "
		//					//"(consider renaming '%U' if it has the same name "
		//					//"as a third-party module you intended to import)",
		//					//modName, name, origin);
		//	}
		//	else if (origin) {
		//	//	alifErr_format(_alifExcAttributeError_,
		//		//"partially initialized "
		//			//"module '%U' from '%U' has no attribute '%U' "
		//			//"(most likely due to a circular import)",
		//			//modName, origin, name);
		//	}
		//	else {
		//	//	alifErr_format(_alifExcAttributeError_,
		//		//"partially initialized "
		//			//"module '%U' has no attribute '%U' "
		//			//"(most likely due to a circular import)",
		//			//modName, name);
		//	}
		//}
		//else if (rc_ == 0) {
		//	rc_ = alifModuleSpec_isUninitializedSubmodule(spec, _name);
		//	if (rc_ > 0) {
		//	//	alifErr_format(_alifExcAttributeError_,
		//		//"cannot access submodule '%U' of module '%U' "
		//			//"(most likely due to a circular import)",
		//			//name, modName);
		//	}
		//	else if (rc_ == 0) {
		//	//	alifErr_format(_alifExcAttributeError_,
		//		//"module '%U' has no attribute '%U'",
		//			//modName, name);
		//	}
		//}
	}

done:
	ALIF_XDECREF(origin);
	ALIF_DECREF(spec);
	ALIF_DECREF(modName);
	return nullptr;
}


AlifObject* alifModule_getAttro(AlifModuleObject* _m, AlifObject* _name) { // 1065
	return alifModule_getAttroImpl(_m, _name, 0);
}


static AlifIntT module_traverse(AlifModuleObject* m,
	VisitProc visit, void* arg) { // 1070
	if (m->def and m->def->traverse
		and (m->def->size <= 0 or m->state != nullptr))
	{
		AlifIntT res = m->def->traverse((AlifObject*)m, visit, arg);
		if (res)
			return res;
	}
	ALIF_VISIT(m->dict);
	return 0;
}



AlifTypeObject _alifModuleType_ = { // 1290
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "وحدة",
	.basicSize = sizeof(AlifModuleObject),
	//(Destructor)module_dealloc,
	.getAttro = (GetAttroFunc)alifModule_getAttro,
	.setAttro = alifObject_genericSetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE,
	.traverse = (TraverseProc)module_traverse,
	.weakListOffset = offsetof(AlifModuleObject, weaklist),

	.dictOffset = offsetof(AlifModuleObject, dict),

	//.new_ = new_module,
	.free = alifObject_gcDel,
};
