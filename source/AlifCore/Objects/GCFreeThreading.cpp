#include "alif.h"

#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_Object.h"
#include <AlifCore_Dict.h>

typedef class AlifGCDureRun GCState; // 20

#define LOCAL_ALLOC_COUNT_THRESHOLD 512 // 31


static bool gc_shouldCollect(AlifGCDureRun* gcstate) { // 1124
	int count = alifAtomic_loadIntRelaxed(&gcstate->young.count);
	int threshold = gcstate->young.threshold;
	if (count <= threshold || threshold == 0 || !gcstate->enabled) {
		return false;
	}
	return (count > gcstate->longLivedTotal / 4 ||
		gcstate->old[0].threshold == 0);
}

static void record_allocation(AlifThread* tstate) { // 1140
	GCThreadState * gc = &((AlifThreadImpl*)tstate)->gc;
	gc->allocCount++;
	if (gc->allocCount >= LOCAL_ALLOC_COUNT_THRESHOLD) {
		GCState* gcstate = &tstate->interpreter->gc;
		alifAtomic_addInt(&gcstate->young.count, (int)gc->allocCount);
		gc->allocCount = 0;

		if (gc_shouldCollect(gcstate) &&
			!alifAtomic_loadIntRelaxed(&gcstate->collecting))
		{
			alif_scheduleGC(tstate);
		}
	}
}




AlifIntT alifObject_isGC(AlifObject* _obj) { // 1748
	return _alifObject_isGC(_obj);
}


void alifObject_gcLink(AlifObject* op) { // 1763
	record_allocation(alifThread_get());
}

static AlifObject* gc_alloc(AlifTypeObject* _tp, size_t _basicSize, size_t _preSize) { // 1789
	AlifThread* tstate = alifThread_get();
	if (_basicSize > LLONG_MAX - _preSize) {
		return nullptr;
		//return alifErr_noMemory(tstate);
	}
	size_t size = _preSize + _basicSize;
	char* mem_ = (char*)alifObject_mallocWithType(_tp, size);
	if (mem_ == NULL) {
		return nullptr;
		//return alifErr_noMemory(tstate);
	}
	if (_preSize) {
		((AlifObject**)mem_)[0] = NULL;
		((AlifObject**)mem_)[1] = NULL;
	}
	AlifObject* op_ = (AlifObject*)(mem_ + _preSize);
	record_allocation(tstate);
	return op_;
}


AlifObject* alifObject_gcNew(AlifTypeObject* tp) { // 1800
	size_t presize = alifType_preHeaderSize(tp);
	size_t size = alifObject_size(tp);
	if (alifType_hasFeature(tp, ALIF_TPFLAGS_INLINE_VALUES)) {
		size += alifInlineValuesSize(tp);
	}
	AlifObject* op = gc_alloc(tp, size, presize);
	if (op == NULL) {
		return NULL;
	}
	alifObject_init(op, tp);
	if (tp->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		alifObject_initInlineValues(op, tp);
	}
	return op;
}
