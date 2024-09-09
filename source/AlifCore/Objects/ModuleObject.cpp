#include "alif.h"

#include "AlifCore_Import.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_ObjectDeferred.h" // هذا التضمين غير ضروري ويجب حذفه مع التقدم في البرنامج






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



static AlifIntT addMethods_toObject(AlifObject* module,
	AlifObject* name, AlifMethodDef* functions) { // 169
	AlifObject* func{};
	AlifMethodDef* fdef{};

	for (fdef = functions; fdef->name != nullptr; fdef++) {
		if ((fdef->flags & METH_CLASS) ||
			(fdef->flags & METH_STATIC)) {
			//alifErr_setString(_alifExcValueError_,
			//	"module functions cannot set"
			//	" METH_CLASS or METH_STATIC");
			return -1;
		}
		func = ALIFCFUNCTION_NEWEX(fdef, (AlifObject*)module, name);
		if (func == nullptr) {
			return -1;
		}
		alifObject_setDeferredRefcount(func);
		if (alifObject_setAttrString(module, fdef->name, func) != 0) {
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
			// memory error
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

	return (AlifObject*)m;
}





AlifIntT alifModule_addFunctions(AlifObject* _m, AlifMethodDef* _functions) { // 523
	AlifIntT res{};
	AlifObject* name = alifModule_getNameObject(_m);
	if (name == nullptr) {
		return -1;
	}

	res = addMethods_toObject(_m, name, _functions);
	ALIF_DECREF(name);
	return res;
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








AlifTypeObject _alifModuleType_ = { // 1290
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "وحدة",
	.basicSize = sizeof(AlifModuleObject),
	.itemSize = 0,                                          
	//(Destructor)module_dealloc,                 
};
