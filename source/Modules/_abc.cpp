#include "alif.h"

#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_Runtime.h"
#include "AlifCore_SetObject.h"
#include "AlifCore_WeakRef.h"

#include "clinic/_abc.cpp.h"




class ABCModuleState { // 22
public:
	AlifTypeObject* abcDataType{};
	uint64_t abcInvalidationCounter{};
};












//static AlifMethodDef _abcModuleMethods_[] = {
//	_ABC__ABC_REGISTER_METHODDEF
//	{nullptr, nullptr}          /* sentinel */
//};
//
//
//
//static AlifModuleDef _abcModule_ = { // 977
//	.base = ALIFMODULEDEF_HEAD_INIT,
//	.name = "صنف_اساس_مجرد",
//	.doc = nullptr,
//	.size = sizeof(ABCModuleState),
//	.methods = _abcModuleMethods_,
//	//.slots = _abcModuleSlots_,
//	//.traverse = _abcmodule_traverse,
//	//.clear = _abcmodule_clear,
//	//.free = _abcmodule_free,
//};
//
//AlifObject* alifInit__abc(void) { // 990
//	return alifModuleDef_init(&_abcModule_);
//}
