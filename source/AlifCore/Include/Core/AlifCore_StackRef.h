#pragma once




#include "AlifCore_ObjectDeferred.h"


















union AlifStackRef { // 52
	uintptr_t bits{};
};

 // 57
#define ALIF_TAG_DEFERRED (1)

#define ALIF_TAG_PTR      (0)
#define ALIF_TAG_BITS     (1)






static inline AlifStackRef _alifStackRef_fromPyObjectSteal(AlifObject* _obj) { // 131
	AlifIntT tag = (_obj == nullptr or ALIF_ISIMMORTAL(_obj)) ? (ALIF_TAG_DEFERRED) : ALIF_TAG_PTR;
	return { .bits = ((uintptr_t)(_obj)) | tag };
}
#   define ALIFSTACKREF_FROMPYOBJECTSTEAL(_obj) _alifStackRef_fromPyObjectSteal(ALIFOBJECT_CAST(_obj))






static inline void alifStackRef_close(AlifStackRef _stackRef) { // 193
	if (ALIFSTACKREF_ISDEFERRED(_stackRef)) {
		return;
	}
	ALIF_DECREF(alifStackRef_asAlifObjectBorrow(_stackRef));
}
