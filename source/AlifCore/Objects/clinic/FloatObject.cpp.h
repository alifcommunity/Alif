
#include "AlifCore_ModSupport.h"




// 87
#define FLOAT___ROUND___METHODDEF    \
    {"__قرب__", ALIF_CPPFUNCTION_CAST(float___round__), METHOD_FASTCALL},

static AlifObject* float___round__Impl(AlifObject*, AlifObject*);

static AlifObject* float___round__(AlifObject* self,
	AlifObject* const* args, AlifSizeT nargs) { // 93
	AlifObject* returnValue{};
	AlifObject* oNDigits = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("__قرب__", nargs, 0, 1)) {
		goto exit;
	}
	if (nargs < 1) {
		goto skip_optional;
	}
	oNDigits = args[0];
skip_optional:
	returnValue = float___round__Impl(self, oNDigits);

exit:
	return returnValue;
}










// 308
#define FLOAT___FORMAT___METHODDEF    \
    {"__تنسيق__", (AlifCPPFunction)float___format__, METHOD_O, /*float___format____doc__*/},

static AlifObject* float___format___impl(AlifObject*, AlifObject*); // 311

static AlifObject* float___format__(AlifObject* _self, AlifObject* _arg) { // 314
	AlifObject* returnValue = nullptr;
	AlifObject* formatSpec{};

	if (!ALIFUSTR_CHECK(_arg)) {
		//_alifArg_badArgument("__تنسيق__", "argument", "str", _arg);
		goto exit;
	}
	formatSpec = _arg;
	returnValue = float___format___impl(_self, formatSpec);

exit:
	return returnValue;
}
