#pragma once




#include "AlifCore_ObjectDeferred.h"


















union AlifStackRef { // 52
	uintptr_t bits{};
};

 // 57
#define ALIF_TAG_DEFERRED (1)

#define ALIF_TAG_PTR      ((uintptr_t)0)
#define ALIF_TAG_BITS     ((uintptr_t)1)

static const AlifStackRef _alifStackRefNull_ = { .bits = 0 | ALIF_TAG_DEFERRED }; // 64
#define ALIFSTACKREF_ISNULL(_stackRef) ((_stackRef).bits == _alifStackRefNull_.bits) // 65
#define ALIFSTACKREF_TRUE AlifStackRef({.bits = ((uintptr_t)&_alifTrueClass_) | ALIF_TAG_DEFERRED }) // 66
#define ALIFSTACKREF_FALSE AlifStackRef({.bits = ((uintptr_t)&_alifFalseClass_) | ALIF_TAG_DEFERRED }) // 67
#define ALIFSTACKREF_NONE AlifStackRef({.bits = ((uintptr_t)&_alifNoneClass_) | ALIF_TAG_DEFERRED }) // 68


static inline AlifObject* alifStackRef_asAlifObjectBorrow(AlifStackRef _stackRef) { // 70
	AlifObject* cleared = ((AlifObject*)((_stackRef).bits & (~ALIF_TAG_BITS)));
	return cleared;
}

#define ALIFSTACKREF_ISDEFERRED(_ref) (((_ref).bits & ALIF_TAG_BITS) == ALIF_TAG_DEFERRED) // 77

static inline AlifObject* alifStackRef_asAlifObjectSteal(AlifStackRef _stackRef) { // 79
	if (ALIFSTACKREF_ISDEFERRED(_stackRef)) {
		return ALIF_NEWREF(alifStackRef_asAlifObjectBorrow(_stackRef));
	}
	return alifStackRef_asAlifObjectBorrow(_stackRef);
}

static inline AlifStackRef _alifStackRef_fromAlifObjectSteal(AlifObject* _obj) { // 89
	AlifUIntT tag = ALIF_ISIMMORTAL(_obj) ? (ALIF_TAG_DEFERRED) : ALIF_TAG_PTR;
	return { .bits = ((uintptr_t)(_obj)) | tag };
}
#define ALIFSTACKREF_FROMALIFOBJECTSTEAL(_obj) _alifStackRef_fromAlifObjectSteal(ALIFOBJECT_CAST(_obj))

static inline AlifStackRef alifStackRef_fromAlifObjectNew(AlifObject* _obj) { // 100
	if (ALIF_ISIMMORTAL(_obj) or _alifObject_hasDeferredRefCount(_obj)) {
		return { .bits = (uintptr_t)_obj | ALIF_TAG_DEFERRED };
	}
	else {
		return { .bits = (uintptr_t)(ALIF_NEWREF(_obj)) | ALIF_TAG_PTR };
	}
}
#define ALIFSTACKREF_FROMALIFOBJECTNEW(_obj) alifStackRef_fromAlifObjectNew(ALIFOBJECT_CAST(_obj))




// 126
#define ALIFSTACKREF_CLOSE(_ref)                                        \
    do {                                                            \
        AlifStackRef closeTmp = (_ref);                             \
        if (!ALIFSTACKREF_ISDEFERRED(closeTmp)) {                   \
            ALIF_DECREF(alifStackRef_asAlifObjectBorrow(closeTmp));     \
        }                                                           \
    } while (0)



static inline AlifStackRef alifStackRef_dup(AlifStackRef _stackRef) { // 135
	if (ALIFSTACKREF_ISDEFERRED(_stackRef)) {
		return _stackRef;
	}
	ALIF_INCREF(alifStackRef_asAlifObjectBorrow(_stackRef));
	return _stackRef;
}




#define ALIFSTACKREF_IS(_a, _b) ((_a).bits == (_b).bits) // 185


#define ALIFSTACKREF_ASALIFOBJECTNEW(_stackref) ALIF_NEWREF(alifStackRef_asAlifObjectBorrow(_stackref)) // 189

#define ALIFSTACKREF_TYPE(_stackref) ALIF_TYPE(alifStackRef_asAlifObjectBorrow(_stackref)) // 191






// 193
#define ALIFSTACKREF_CLEAR(_op) \
    do { \
        AlifStackRef *_tmp_op_ptr = &(_op); \
        AlifStackRef _tmp_old_op = (*_tmp_op_ptr); \
        if (!ALIFSTACKREF_ISNULL(_tmp_old_op)) { \
            *_tmp_op_ptr = _alifStackRefNull_; \
            ALIFSTACKREF_CLOSE(_tmp_old_op); \
        } \
    } while (0)



 // 203
#define ALIFSTACKREF_XCLOSE(_stackref) \
    do {                            \
        AlifStackRef _tmp = (_stackref); \
        if (!ALIFSTACKREF_ISNULL(_tmp)) { \
            ALIFSTACKREF_CLOSE(_tmp); \
        } \
    } while (0);






