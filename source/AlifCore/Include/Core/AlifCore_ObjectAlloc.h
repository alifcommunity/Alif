#pragma once

static inline void* alifObject_mallocWithType(AlifTypeObject* _tp, size_t _size) {// 38
	AlifThreadImpl* tState = (AlifThreadImpl*)alifThread_get();
	class MimallocThreadState* m_ = &tState->mimalloc;
	m_->currentObjectHeap = alifObject_getAllocationHeap(tState, tp);

	void* mem_ = alifMem_objAlloc(_size);
	m_->currentObjectHeap = &m_->heaps[ALIF_MIMALLOC_HEAP_OBJECT];
	return mem_;
}
