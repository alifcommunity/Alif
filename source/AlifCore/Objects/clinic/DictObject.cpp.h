#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"








// 75
#define DICT_GET_METHODDEF    \
    {"احضر", ALIF_CPPFUNCTION_CAST(dict_get), METHOD_FASTCALL},

static AlifObject* dict_getImpl(AlifDictObject*, AlifObject*, AlifObject*);

static AlifObject* dict_get(AlifDictObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs) { // 81
	AlifObject* returnValue = nullptr;
	AlifObject* key{};
	AlifObject* defaultValue = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("احضر", _nargs, 1, 2)) {
		goto exit;
	}
	key = _args[0];
	if (_nargs < 2) {
		goto skip_optional;
	}
	defaultValue = _args[1];
skip_optional:
	ALIF_BEGIN_CRITICAL_SECTION(_self);
	returnValue = dict_getImpl(_self, key, defaultValue);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}







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
