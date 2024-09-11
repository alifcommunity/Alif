#pragma once


#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_ThreadState.h"


static inline void* alifObject_mallocWithType(AlifTypeObject* _tp, AlifUSizeT _size) { // 38
	AlifThreadImpl* tState = (AlifThreadImpl*)alifThread_get();
	MimallocThreadState* m_ = &tState->mimalloc;
	m_->currentObjectHeap = alifObject_getAllocationHeap(tState, _tp);

	void* mem_ = alifMem_objAlloc(_size);
	m_->currentObjectHeap = &m_->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Object];
	return mem_;
}
