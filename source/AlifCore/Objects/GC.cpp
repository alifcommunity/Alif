#include "alif.h"
#include "AlifCore_Dict.h"
#include "AlifCore_Object.h"

typedef class AlifGCDureRun GCState; // 20


#define AS_GC(op) alif_asGC(op) // 32
#define FROM_GC(gc) alif_fromGC(gc) // 33



void alifObject_gcLink(AlifObject* _op) // 1995
{
	AlifGCHead* gc = AS_GC(_op);

	AlifThread* tstate = alifThread_get();
	GCState* gcstate = &tstate->interpreter->gc;
	gc->gcNext = 0;
	gc->gcPrev = 0;
	gcstate->young.count++; /* number of allocated GC objects */
	gcstate->heapSize++;
	if (gcstate->young.count > gcstate->young.threshold &&
		gcstate->enabled &&
		gcstate->young.threshold
		//&&
		//!alifAtomic_load_intRelaxed(&gcstate->collecting) &&
		//!alifErr_occurred(tstate)
		)
	{
		//alif_scheduleGC(tstate);
	}
}


AlifObject* alifObject_gcNew(AlifTypeObject* _tp) // 2046
{
	size_t presize = alifType_preHeaderSize(_tp);
	size_t size = alifObject_size(_tp);
	if (alifType_hasFeature(_tp, ALIF_TPFLAGS_INLINE_VALUES)) {
		size += alifInlineValuesSize(_tp);
	}
	AlifObject* op = gc_alloc(_tp, size, presize);
	if (op == NULL) {
		return NULL;
	}
	alifObject_init(op, _tp);
	if (_tp->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		//alifObject_initInlineValues(op, _tp);
	}
	return op;
}


static AlifObject* gc_alloc(AlifTypeObject* _tp, size_t _basicSize, size_t _preSize)
{
	AlifThread* tState = alifThread_get();
	if (_basicSize > LLONG_MAX - _preSize) {
		//return alifErr_noMemory(tState);
	}
	size_t size = _preSize + _basicSize;
	char* mem_ = (char*)alifObject_mallocWithType(_tp, size);
	if (mem_ == NULL) {
		//return alifErr_noMemory(tState);
	}
	((AlifObject**)mem_)[0] = NULL;
	((AlifObject**)mem_)[1] = NULL;
	AlifObject* op_ = (AlifObject*)(mem_ + _preSize);
	alifObject_gcLink(op_);
	return op_;
}
