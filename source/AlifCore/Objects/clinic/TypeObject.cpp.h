#include "AlifCore_ModSupport.h"






 // 87
#define TYPE___SUBCLASSES___METHODDEF    \
    {"__subclasses__", (AlifCPPFunction)type___subclasses__, METHOD_NOARGS/*, type___subclasses____doc__*/},

static AlifObject* type___subclasses___impl(AlifTypeObject* self); // 90

static AlifObject* type___subclasses__(AlifTypeObject* self, AlifObject* ALIF_UNUSED(ignored))
{ // 93
	return type___subclasses___impl(self);
}
