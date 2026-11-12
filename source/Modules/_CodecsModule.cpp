#include "alif.h"
#include "AlifCore_Codecs.h"


#ifdef _WINDOWS
#include <windows.h>
#endif








#include "clinic/_CodecsModule.cpp.h"








static AlifObject* _codecs_register(AlifObject* _module,
	AlifObject* _searchFunction) { // 62
	if (alifCodec_register(_searchFunction))
		return nullptr;

	return ALIF_NONE;
}














static AlifMethodDef _codecsFunctions_[] = { // 1026
	_CODECS_REGISTER_METHODDEF
	{nullptr, nullptr}                /* sentinel */
};


static AlifModuleDefSlot _codecsSlots_[] = { // 1076
	{ALIF_MOD_MULTIPLE_INTERPRETERS, ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED},
	{ALIF_MOD_GIL, ALIF_MOD_GIL_NOT_USED},
	{0, nullptr}
};

static AlifModuleDef _codecsModule_ = { // 1082
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "برمجيات_ترميز",
	.methods = _codecsFunctions_,
	.slots = _codecsSlots_,
};


ALIFMODINIT_FUNC alifInit__codecs(void) {
	return alifModuleDef_init(&_codecsModule_);
}
