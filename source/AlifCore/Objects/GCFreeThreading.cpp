#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Mimalloc.h"
#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_Object.h"
#include "AlifCore_Dict.h"

typedef class GCDureRunState GCState; // 20

#define LOCAL_ALLOC_COUNT_THRESHOLD 512 // 31




void alifGC_initState(GCState* _gcState) { // 810
	_gcState->young.threshold = 2000;
}









static bool gc_shouldCollect(GCDureRunState* gcstate) { // 1124
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
		alifAtomic_addInt(&gcstate->young.count, (AlifIntT)gc->allocCount);
		gc->allocCount = 0;

		if (gc_shouldCollect(gcstate) &&
			!alifAtomic_loadIntRelaxed(&gcstate->collecting))
		{
			alif_scheduleGC(tstate);
		}
	}
}


static void record_deallocation(AlifThread* _threadState) { // 1161
	class GCThreadState* gc_ = &((AlifThreadImpl*)_threadState)->gc;

	gc_->allocCount--;
	if (gc_->allocCount <= -LOCAL_ALLOC_COUNT_THRESHOLD) {
		GCState* gcState = &_threadState->interpreter->gc;
		alifAtomic_addInt(&gcState->young.count, (AlifIntT)gc_->allocCount);
		gc_->allocCount = 0;
	}
}

void alifObject_gcTrack(void* _opRaw) { // 1717
	AlifObject* op_ = ALIFOBJECT_CAST(_opRaw);
	if (ALIFOBJECT_GC_IS_TRACKED(op_)) {
		//ALIFOBJEC_ASSERT_FAILED_MSG(op_,
		//	"object already tracked "
		//	"by the garbage collector");
	}
	ALIFOBJECT_GC_TRACK(op_);
}


AlifIntT alifObject_isGC(AlifObject* _obj) { // 1748
	return _alifObject_isGC(_obj);
}


void alif_scheduleGC(AlifThread* _thread) { // 1754
	if (!alifEval_breakerBitIsSet(_thread, ALIF_GC_SCHEDULED_BIT))
	{
		alifSet_evalBreakerBit(_thread, ALIF_GC_SCHEDULED_BIT);
	}
}


void alifObject_gcLink(AlifObject* _op) { // 1763
	record_allocation(alifThread_get());
}

static AlifObject* gc_alloc(AlifTypeObject* _tp,
	AlifUSizeT _basicSize, AlifUSizeT _preSize) { // 1779
	AlifThread* thread = alifThread_get();
	if (_basicSize > ALIF_SIZET_MAX - _preSize) {
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

AlifVarObject* alifObject_gcResize(AlifVarObject* _op, AlifSizeT _nItems) { // 1852 
	const AlifUSizeT basicSize = alifObject_varSize(ALIF_TYPE(_op), _nItems);
	const AlifUSizeT preSize = alifType_preHeaderSize(((AlifObject*)_op)->type);
	if (basicSize > (AlifUSizeT)ALIF_SIZET_MAX - preSize) {
		return nullptr;
		//return (AlifVarObject*)alifErr_noMemory();
	}
	char* mem_ = (char*)_op - preSize;
	mem_ = (char*)alifObject_reallocWithType(ALIF_TYPE(_op), mem_, preSize + basicSize);
	if (mem_ == nullptr) {
		return nullptr;
		//return (AlifVarObject*)alifErr_noMemory();
	}
	_op = (AlifVarObject*)(mem_ + preSize);
	ALIF_SET_SIZE(_op, _nItems);
	return _op;
}


void alifObject_gcDel(void* _op) { // 1871
	AlifUSizeT preSize = alifType_preHeaderSize(((AlifObject*)_op)->type);
	if (ALIFOBJECT_GC_IS_TRACKED(_op)) {
		ALIFOBJECT_GC_UNTRACK(_op);
	}

	record_deallocation(alifThread_get());
	AlifObject* self = (AlifObject*)_op;
	if (ALIFOBJECT_GC_IS_SHARED_INLINE(self)) {
		alifObject_freeDelayed(((char*)_op) - preSize);
	}
	else {
		alifMem_dataFree(((char*)_op) - preSize);
	}
}
