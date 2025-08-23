#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"

















#define DICT_KEYS_METHODDEF    \
    {"مفاتيح", (AlifCPPFunction)dict_keys, METHOD_NOARGS}, // 268

static AlifObject* dict_keysImpl(AlifDictObject*); // 271

static AlifObject* dict_keys(AlifDictObject* self, AlifObject* ALIF_UNUSED(ignored)) { // 274
	return dict_keysImpl(self);
}




#define DICT_VALUES_METHODDEF    \
    {"قيم", (AlifCPPFunction)dict_values, METHOD_NOARGS}, // 304

static AlifObject* dict_valuesImpl(AlifDictObject*); // 307

static AlifObject* dict_values(AlifDictObject* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 310
	return dict_valuesImpl(self);
}
