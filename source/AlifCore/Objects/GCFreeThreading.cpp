#include "alif.h"

#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_Object.h"
#include "AlifCore_Dict.h"

typedef class AlifGCDureRun GCState; // 20

#define LOCAL_ALLOC_COUNT_THRESHOLD 512 // 31


static bool gc_shouldCollect(AlifGCDureRun* gcstate) { // 1124
	int count = alifAtomic_loadIntRelaxed(&gcstate->young.count);
	int threshold = gcstate->young.threshold;
	if (count <= threshold or threshold == 0 or !gcstate->enabled) {
		return false;
	}
	return (count > gcstate->longLivedTotal / 4 or
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


void alifObject_gcLink(AlifObject* op_) { // 1763
	record_allocation(alifThread_get());
}

static AlifObject* gc_alloc(AlifTypeObject* _tp, AlifUSizeT _basicSize, AlifUSizeT _preSize) { // 1789
	AlifThread* thread = alifThread_get();
	if (_basicSize > LLONG_MAX - _preSize) {
		return nullptr;
		//return alifErr_noMemory(thread);
	}
	AlifUSizeT size = _preSize + _basicSize;
	char* mem_ = (char*)alifObject_mallocWithType(_tp, size);
	if (mem_ == nullptr) {
		return nullptr;
		//return alifErr_noMemory(thread);
	}
	if (_preSize) {
		((AlifObject**)mem_)[0] = nullptr;
		((AlifObject**)mem_)[1] = nullptr;
	}
	AlifObject* op_ = (AlifObject*)(mem_ + _preSize);
	record_allocation(thread);
	return op_;
}


AlifObject* alifObject_gcNew(AlifTypeObject* tp) { // 1800
	AlifUSizeT preSize = alifType_preHeaderSize(tp);
	AlifUSizeT size = alifObject_size(tp);
	if (alifType_hasFeature(tp, ALIF_TPFLAGS_INLINE_VALUES)) {
		size += alifInline_valuesSize(tp);
	}
	AlifObject* op_ = gc_alloc(tp, size, preSize);
	if (op_ == nullptr) {
		return nullptr;
	}
	alifObject_init(op_, tp);
	if (tp->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		alifObject_initInlineValues(op_, tp);
	}
	return op_;
}

AlifVarObject* alifObject_gcNewVar(AlifTypeObject* _tp, AlifSizeT _nItems) { // 1820
	AlifVarObject* op_{};

	if (_nItems < 0) {
		//alifErr_badInternalCall();
		return nullptr;
	}
	AlifUSizeT preSize = alifType_preHeaderSize(_tp);
	AlifUSizeT size = alifObject_varSize(_tp, _nItems);
	op_ = (AlifVarObject*)gc_alloc(_tp, size, preSize);
	if (op_ == nullptr) {
		return nullptr;
	}
	alifObject_initVar(op_, _tp, _nItems);
	return op_;
}
