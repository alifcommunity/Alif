#include "alif.h"

#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"






















AlifObject* alifModule_createInitialized(AlifModuleDef* _module) { // 209

	const wchar_t* name{};
	AlifModuleObject* m{};

	if (!alifModuleDef_init(_module)) return nullptr;

	name = _module->name;
	if (_module->slots) {
		// error
		return nullptr;
	}
	//name = alifImport_resolveNameWithPackageContext(name);
	if ((m = (AlifModuleObject*)alifNew_module(name)) == nullptr) return nullptr;

	if (_module->size > 0) {
		m->state = alifMem_dataAlloc(_module->size);
		if (!m->state) {
			// memory error
			ALIF_DECREF(m);
			return nullptr;
		}
		//memset(m->state, 0, _module->size);
	}

	if (_module->methods != nullptr) {
		if (alifModule_addFunctions((AlifObject*)m, _module->methods) != 0) {
			ALIF_DECREF(m);
			return nullptr;
		}
	}
	if (_module->doc != nullptr) {
		//if (alifModule_setDocString((AlifObject*)m, _module->doc) != 0) {
		//	ALIF_DECREF(m);
		//	return nullptr;
		//}
	}
	m->def = _module;

	return (AlifObject*)m;
}
