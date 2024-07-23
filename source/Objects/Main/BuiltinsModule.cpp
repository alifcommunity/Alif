#include "alif.h"

#include "AlifCore_AlifEval.h"
#include "AlifCore_ModSupport.h"

#include "BuiltinsModule.h"






static AlifMethodDef builtinMethods[] = {
	BUILTIN_PRINT_METHODDEF
};


static class AlifModuleDef _alifBuiltinsModule_ = {
	ALIFMODULEDEF_HEAD_INIT,
	L"builtins",
	nullptr,
	-1,
	builtinMethods,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
};


AlifObject* alifBuiltin_init(AlifInterpreter* _interp) { // 3130
	AlifObject* mod{}, * dict{};

	const AlifConfig* config = &_interp->config;

	mod = alifModule_createInitialized(&_alifBuiltinsModule_);
	if (mod == nullptr) return nullptr;

	dict = alifModule_getDict(mod);

	return mod;
}
