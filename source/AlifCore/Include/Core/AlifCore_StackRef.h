#pragma once




#include "AlifCore_ObjectDeferred.h"


















union AlifStackRef { // 52
	uintptr_t bits{};
};

 // 57
#define ALIF_TAG_DEFERRED (1)

#define ALIF_TAG_PTR      (0)
#define ALIF_TAG_BITS     (1)

#ifdef ALIF_GIL_DISABLED // 62
static const AlifStackRef _alifStackRefNull_ = { .bits = 0 | ALIF_TAG_DEFERRED };
#else
static const AlifStackRef _alifStackRefNull_ = { .bits = 0 };
#endif

#define ALIFSTACKREF_ISNULL(_stackRef) ((_stackRef).bits == _alifStackRefNull_.bits) // 68

#define ALIFSTACKREF_ISDEFERRED(ref) (((ref).bits & ALIF_TAG_BITS) == ALIF_TAG_DEFERRED) // 93

static inline AlifObject* alifStackRef_asAlifObjectBorrow(AlifStackRef _stackRef) { // 99
	AlifObject* cleared = ((AlifObject*)((_stackRef).bits & (~ALIF_TAG_BITS)));
	return cleared;
}

static inline AlifObject* alifStackRef_asAlifObjectSteal(AlifStackRef _stackRef) { // 112
	if (!ALIFSTACKREF_ISNULL(_stackRef) and ALIFSTACKREF_ISDEFERRED(_stackRef)) {
		return ALIF_NEWREF(alifStackRef_asAlifObjectBorrow(_stackRef));
	}
	return alifStackRef_asAlifObjectBorrow(_stackRef);
}


static inline AlifStackRef _alifStackRef_fromPyObjectSteal(AlifObject* _obj) { // 131
	AlifIntT tag = (_obj == nullptr or ALIF_ISIMMORTAL(_obj)) ? (ALIF_TAG_DEFERRED) : ALIF_TAG_PTR;
	return { .bits = ((uintptr_t)(_obj)) | tag };
}
#   define ALIFSTACKREF_FROMALIFBJECTSTEAL(_obj) _alifStackRef_fromPyObjectSteal(ALIFOBJECT_CAST(_obj))






static inline void alifStackRef_close(AlifStackRef _stackRef) { // 193
	if (ALIFSTACKREF_ISDEFERRED(_stackRef)) {
		return;
	}
	ALIF_DECREF(alifStackRef_asAlifObjectBorrow(_stackRef));
}
