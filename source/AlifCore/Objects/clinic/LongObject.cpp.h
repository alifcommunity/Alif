#include "AlifCore_Abstract.h" 
#include "AlifCore_ModSupport.h"













// 95
#define INT___FORMAT___METHODDEF    \
    {"__تنسيق__", (AlifCPPFunction)int___format__, METHOD_O},

static AlifObject* int___format__Impl(AlifObject*, AlifObject*);

static AlifObject* int___format__(AlifObject *self, AlifObject *arg) {
	AlifObject* returnValue{};
	AlifObject* formatSpec{};

	if (!ALIFUSTR_CHECK(arg)) {
		//_alifArg_badArgument("__تنسيق__", "argument", "str", arg);
		goto exit;
	}
	formatSpec = arg;
	returnValue = int___format__Impl(self, formatSpec);

exit:
	return returnValue;
}



// 192
#define INT_BIT_LENGTH_METHODDEF    \
    {"طول_الثنائي", (AlifCPPFunction)int_bitLength, METHOD_NOARGS},

static AlifObject* int_bitLengthImpl(AlifObject*);

static AlifObject* int_bitLength(AlifObject* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 198
	return int_bitLengthImpl(self);
}
