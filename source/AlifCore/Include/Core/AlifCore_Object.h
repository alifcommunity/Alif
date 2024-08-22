#pragma once

#include "AlifCore_State.h"
#include "AlifCore_TypeID.h"





















static inline AlifIntT alifType_hasFeature(AlifTypeObject* _type, unsigned long _feature) { // 281
	return ((ALIFATOMIC_LOAD_ULONG_RELAXED(&_type->flags) & _feature) != 0);
}






static inline void alif_increaseRefType(AlifTypeObject* type) { // 296
	if (!alifType_hasFeature(type, ALIF_TPFLAGS_HEAPTYPE)) {
		return;
	}

#if defined(__GNUC__) && __GNUC__ >= 11
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

	AlifThreadImpl* thread = (AlifThreadImpl*)alifThread_get();
	AlifHeapTypeObject* ht = (AlifHeapTypeObject*)type;

	if ((AlifUSizeT)ht->uniqueID < (AlifUSizeT)thread->types.size) {
		thread->types.refCounts[ht->uniqueID]++;
	}
	else {
		alifType_incRefSlow(ht);
	}

#if defined(__GNUC__) && __GNUC__ >= 11
#  pragma GCC diagnostic pop
#endif
}





static inline void alifObject_init(AlifObject* _op, AlifTypeObject* _typeObj) { // 370
	ALIF_SET_TYPE(_op, _typeObj);
	alif_increaseRefType(_typeObj);
	alif_newReference(_op);
}
