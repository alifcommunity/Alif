#pragma once

#include "AlifCore_State.h"
#include "AlifCore_TypeID.h"
#include "AlifCore_GC.h"








void alif_setImmortal(AlifObject*); // 162
void alif_setImmortalUntracked(AlifObject*); // 163 










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





static inline void alifObject_init(AlifObject* _op, AlifTypeObject* _typeObj) { // 370
	ALIF_SET_TYPE(_op, _typeObj);
	alif_increaseRefType(_typeObj);
	alif_newReference(_op);
}




static inline void alifObject_gcTrack(
//#ifndef NDEBUG
//	const char* filename, int lineno,
//#endif
	AlifObject* _op)
{
#ifdef ALIF_GIL_DISABLED
		alifObject_setGCBits(op, 1);
#else
	AlifGCHead* gc_ = alif_asGC(_op);
	AlifInterpreter* interp = alifInterpreter_get();
	AlifGCHead* generation0 = &interp->gc.young.head;
	AlifGCHead* last = (AlifGCHead*)(generation0->gcPrev);
	alifGCHead_set_next(last, gc_);
	alifGCHead_set_perv(gc_, last);
	gc_->gcNext = ((uintptr_t)generation0) | interp->gc.visited_space;
	generation0->gcPrev = (uintptr_t)gc_;
#endif
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


static inline size_t alifType_preHeaderSize(AlifTypeObject* _tp) // 740
{
	return (
#ifndef ALIF_GIL_DISABLED
		ALIFTYPE_IS_GC(_tp) * sizeof(AlifGCHead) +
#endif
		alifType_hasFeature(_tp, ALIF_TPFLAGS_PREHEADER) * 2 * sizeof(AlifObject*)
		);
}


// ----------------------------------------- AlifCore_Object_Alloc ---------------------------------------------------//

static inline void* alifObject_mallocWithType(AlifTypeObject* _tp, size_t _size) // 38
{
#ifdef ALIF_GIL_DISABLED
	AlifThreadImpl* tState = (AlifThreadImpl*)alifThread_get();
	class MimallocThreadState* m_ = &tState->mimalloc;
	m_->currentObjectHeap = alifObject_getAllocationHeap(tState, tp);
#endif
	void* mem_ = alifMem_objAlloc(_size);
#ifdef ALIF_GIL_DISABLED
	m_->currentObjectHeap = &m->heaps[ALIF_MIMALLOC_HEAP_OBJECT];
#endif
	return mem_;
}
