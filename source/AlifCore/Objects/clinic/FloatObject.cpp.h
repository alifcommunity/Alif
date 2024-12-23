
#include "AlifCore_ModSupport.h"
















// 308
#define FLOAT___FORMAT___METHODDEF    \
    {"__format__", (AlifCPPFunction)float___format__, METHOD_O, /*float___format____doc__*/},

static AlifObject* float___format___impl(AlifObject*, AlifObject*); // 311

static AlifObject* float___format__(AlifObject* _self, AlifObject* _arg) { // 314
	AlifObject* returnValue = nullptr;
	AlifObject* formatSpec{};

	if (!ALIFUSTR_CHECK(_arg)) {
		//_alifArg_badArgument("__format__", "argument", "str", _arg);
		goto exit;
	}
	formatSpec = _arg;
	returnValue = float___format___impl(_self, formatSpec);

exit:
	return returnValue;
}
