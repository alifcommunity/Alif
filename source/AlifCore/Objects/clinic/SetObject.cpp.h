
#include "AlifCore_ModSupport.h"





// 16
#define SET_POP_METHODDEF    \
    {"اسحب", (AlifCPPFunction)set_pop, METHOD_NOARGS},

static AlifObject* set_popImpl(AlifSetObject*); // 19


static AlifObject* set_pop(AlifSetObject* so, AlifObject* ALIF_UNUSED(ignored)) { // 22
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(so);
	returnValue = set_popImpl(so);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}
