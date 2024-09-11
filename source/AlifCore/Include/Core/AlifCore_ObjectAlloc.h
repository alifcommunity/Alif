#pragma once


#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_ThreadState.h"








#ifdef ALIF_GIL_DISABLED
static inline mi_heap_t* alifObject_getAllocationHeap(AlifThreadImpl* _thread, AlifTypeObject* _tp) { // 17
	MimallocThreadState* m = &_thread->mimalloc;
	if (_alifType_hasFeature(_tp, ALIF_TPFLAGS_PREHEADER)) {
		return &m->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_GCPre];
	}
	else if (ALIFTYPE_IS_GC(_tp)) {
		return &m->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_GC];
	}
	else {
		return &m->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Object];
	}
}
#endif




static inline void* alifObject_mallocWithType(AlifTypeObject* _tp, AlifUSizeT _size) { // 38
	AlifThreadImpl* tState = (AlifThreadImpl*)alifThread_get();
	MimallocThreadState* m_ = &tState->mimalloc;
	m_->currentObjectHeap = alifObject_getAllocationHeap(tState, _tp);

	void* mem_ = alifMem_objAlloc(_size);
	m_->currentObjectHeap = &m_->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Object];
	return mem_;
}
