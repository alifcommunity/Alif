#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"

















#define DICT_KEYS_METHODDEF    \
    {"مفاتيح", (AlifCPPFunction)dict_keys, METHOD_NOARGS}, // 268

static AlifObject* dict_keysImpl(AlifDictObject*); // 271

static AlifObject* dict_keys(AlifDictObject* self, AlifObject* ALIF_UNUSED(ignored)) { // 274
	return dict_keysImpl(self);
}
