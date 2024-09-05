#pragma once

#include "AlifCore_State.h"
#include "AlifCore_TypeID.h"
#include "AlifCore_GC.h"








void alif_setImmortal(AlifObject*); // 162
void alif_setImmortalUntracked(AlifObject*); // 163 






AlifSizeT alif_explicitMergeRefcount(AlifObject*, AlifSizeT); // 263



static inline AlifIntT _alifType_hasFeature(AlifTypeObject* _type, unsigned long _feature) { // 281
	return ((ALIFATOMIC_LOAD_ULONG_RELAXED(&_type->flags) & _feature) != 0);
}






static inline void alif_increaseRefType(AlifTypeObject* type) { // 296
	if (!_alifType_hasFeature(type, ALIF_TPFLAGS_HEAPTYPE)) {
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





static inline void _alifObject_init(AlifObject* _op, AlifTypeObject* _typeObj) { // 370
	ALIF_SET_TYPE(_op, _typeObj);
	alif_increaseRefType(_typeObj);
	alif_newReference(_op);
}

static inline void _alifObject_initVar(AlifVarObject* _op,
	AlifTypeObject* _typeObj, AlifSizeT _size) { // 380
	alifObject_init((AlifObject*)_op, _typeObj);
	ALIF_SET_SIZE(_op, _size);
}



static inline void alifObject_gcTrack(AlifObject* _op) { // 403
	alifObject_setGCBits(_op, ALIFGC_BITS_TRACKED);
}

static inline void alifObject_gcUntrack(AlifObject* op) { // 443
	alifObject_clearGCBits(op, ALIFGC_BITS_TRACKED);
}

// 471
#define ALIFOBJECT_GC_TRACK(_op) \
        alifObject_gcTrack(ALIFOBJECT_CAST(_op))
#define ALIFOBJECT_GC_UNTRACK(_op) \
        alifObject_gcUntrack(ALIFOBJECT_CAST(_op))




static inline AlifIntT _alifObject_isGC(AlifObject* obj) { // 714
	AlifTypeObject* type = ALIF_TYPE(obj);
	return (ALIFTYPE_IS_GC(type) and (type->isGC == nullptr or type->isGC(obj)));
}



static inline size_t alifType_preHeaderSize(AlifTypeObject* _tp) { // 740 
	return (alifType_hasFeature(_tp, ALIF_TPFLAGS_PREHEADER) * 2 * sizeof(AlifObject*));
}

void alifObject_gcLink(AlifObject*); // 750

extern AlifObject* alifType_allocNoTrack(AlifTypeObject*, AlifSizeT); // 763

void alifObject_initInlineValues(AlifObject*, AlifTypeObject*); // 771


# define MANAGED_DICT_OFFSET    (((AlifSizeT)sizeof(AlifObject *))*-1) // 778 

union AlifManagedDictPointer{ // 785
	AlifDictObject* dict{};
};

static inline AlifManagedDictPointer* alifObject_managedDictPointer(AlifObject* obj) { // 790
	return (AlifManagedDictPointer*)((char*)obj + MANAGED_DICT_OFFSET);
}

static inline AlifDictValues* alifObject_inlineValues(AlifObject* _obj) { // 803
	AlifTypeObject* tp_ = ALIF_TYPE(_obj);
	return (AlifDictValues*)((char*)_obj + tp_->basicSize);
}







extern AlifTypeObject _alifNoneType_;
