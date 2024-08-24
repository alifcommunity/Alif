#include "alif.h"

#include "AlifCore_Import.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"





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




AlifObject* alifModule_new(const char* name) { // 137
	AlifObject* nameObj{}, * module{};
	nameObj = alifUStr_fromString(name);
	if (nameObj == nullptr)
		return nullptr;
	//module = alifModule_newObject(nameobj);
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
		//if (alifModule_addFunctions((AlifObject*)m, _module->methods) != 0) {
		//	ALIF_DECREF(m);
		//	return nullptr;
		//}
	}
	m->def = _module;

	return (AlifObject*)m;
}
