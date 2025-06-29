#include "AlifCore_ModSupport.h"






 // 87
#define TYPE___SUBCLASSES___METHODDEF    \
    {"__subclasses__", (AlifCPPFunction)type___subclasses__, METHOD_NOARGS/*, type___subclasses____doc__*/},

static AlifObject* type___subclasses___impl(AlifTypeObject* self); // 90

static AlifObject* type___subclasses__(AlifTypeObject* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 93
	return type___subclasses___impl(self);
}




 // 207
#define OBJECT___FORMAT___METHODDEF    \
    {"__format__", (AlifCPPFunction)object___format__, METHOD_O}

static AlifObject* object___format__Impl(AlifObject*, AlifObject*); // 210

static AlifObject* object___format__(AlifObject* self, AlifObject* arg) { // 213
	AlifObject* returnValue = nullptr;
	AlifObject* formatSpec{};

	if (!ALIFUSTR_CHECK(arg)) {
		//_alifArg_badArgument("__format__", "argument", "str", arg);
		goto exit;
	}
	formatSpec = arg;
	returnValue = object___format__Impl(self, formatSpec);

exit:
	return returnValue;
}
