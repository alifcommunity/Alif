#include "AlifCore_Abstract.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"













 // 110
#define LIST_APPEND_METHODDEF    \
    {"اضف", (AlifCPPFunction)list_append, METHOD_O /*, list_append__doc__*/},

static AlifObject*
list_appendImpl(AlifListObject*, AlifObject*); // 113

static AlifObject* list_append(AlifListObject* _self, AlifObject* _object) { // 116
	AlifObject* return_value = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(_self);
	return_value = list_appendImpl(_self, _object);
	ALIF_END_CRITICAL_SECTION();

	return return_value;
}
