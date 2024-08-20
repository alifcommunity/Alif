#include "alif.h"

//#include "AlifCore_AlifEval.h"
//#include "AlifCore_ModSupport.h"
#include "AlifCore_State.h"

#include "BuiltinsModule.h"






//static AlifMethodDef builtinMethods[] = { // 3141
//	BUILTIN_PRINT_METHODDEF,
//	{nullptr, nullptr},
//};
//
//
//static AlifModuleDef _alifBuiltinsModule_ = { // 3202
//	ALIFMODULEDEF_HEAD_INIT,
//	"builtins",
//	nullptr,
//	-1,
//	builtinMethods,
//	nullptr,
//	nullptr,
//	nullptr,
//	nullptr,
//};


AlifObject* alifBuiltin_init(AlifInterpreter* _interpreter) { // 3215
	AlifObject* mod{}, * dict{};

	const AlifConfig* config = alifInterpreter_getConfig(_interpreter);

	//mod = alifModule_createInitialized(&_alifBuiltinsModule_);
	//if (mod == nullptr) return nullptr;

	//dict = alifModule_getDict(mod);

	return mod;
}
