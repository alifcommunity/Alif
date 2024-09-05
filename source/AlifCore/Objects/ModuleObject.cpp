#include "alif.h"

#include "AlifCore_Import.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"





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


AlifObject* alifModule_createInitialized(AlifModuleDef* _module) { // 209

	const char* name{};
	AlifModuleObject* m{};

	if (!alifModuleDef_init(_module)) return nullptr;

	name = _module->name;
	if (_module->slots) {
		// error
		return nullptr;
	}
	//name = alifImport_resolveNameWithPackageContext(name);
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














AlifTypeObject _alifModuleType_ = { // 1290
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "وحدة",
	.basicSize = sizeof(AlifModuleObject),
	.itemSize = 0,                                          
	//(Destructor)module_dealloc,                 
};
