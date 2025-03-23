#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_InitConfig.h"

#include "AlifCore_State.h"


#include "_IOModule.h"













#include "clinic/_IOModule.cpp.h" // 627





static AlifMethodDef _moduleMethods_[] = { // 630
	_IO_OPEN_METHODDEF,
	//_IO_TEXT_ENCODING_METHODDEF
	//_IO_OPEN_CODE_METHODDEF
	{nullptr, nullptr}
};










AlifModuleDef _alifIOModule_ = { // 724
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "io",
	//.doc = module_doc,
	.size = sizeof(AlifIOState),
	.methods = _moduleMethods_,
	//.traverse = iomodule_traverse,
	//.clear = iomodule_clear,
	//.free = iomodule_free,
	//.slots = iomodule_slots,
};

AlifObject* alifInit__io(void) { // 736
	return alifModuleDef_init(&_alifIOModule_);
}
