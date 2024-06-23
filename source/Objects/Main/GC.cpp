#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Memory.h"

typedef class AlifGCDureRun AlifGC;

#define GC_NEXT alifSubGCHead_next
#define GC_PREV alifSubGCHead_prev

#define AS_GC(_gc) alifSub_asGC(_gc) // 50

static AlifGC* get_gc_state(void)
{
	AlifInterpreter* interp = alifInterpreter_get();
	return &interp->gc;
}

void alifSubGC_initState(AlifGC* _gcState)
{
#define INIT_HEAD(_GEN) \
    do { \
        _GEN.head.gcNext = (uintptr_t)&_GEN.head; \
        _GEN.head.gcPrev = (uintptr_t)&_GEN.head; \
    } while (0)

	INIT_HEAD(_gcState->young);
	INIT_HEAD(_gcState->old[0]);
	INIT_HEAD(_gcState->old[1]);
	INIT_HEAD(_gcState->permanentGeneration);

#undef INIT_HEAD
}

AlifIntT alifSubGC_init(AlifInterpreter* _interp)
{
	AlifGC* gcState = &_interp->gc;

	gcState->garbage = alifNew_list(0);
	if (gcState->garbage == nullptr) {
		return -1;
	}

	gcState->callbacks = alifNew_list(0);
	if (gcState->callbacks == nullptr) {
		return -1;
	}
	gcState->heapSize = 0;

	return 0;
}

static inline void gc_list_remove(AlifGCHead* _node)
{
	AlifGCHead* prev = GC_PREV(_node);
	AlifGCHead* next = GC_NEXT(_node);

	alifSubGCHead_set_next(prev, next);
	alifSubGCHead_set_prev(next, prev);

	_node->gcNext = 0; /* object is not currently tracked */
}

void alifObject_gc_track(void* _opRaw)
{
	AlifObject* op_ = ALIFOBJECT_CAST(_opRaw);
	//if (ALIFSUBOBJECT_GC_ISTRACKED(op)) {
	//}
	ALIFOBJECT_GC_TRACK(op_);

#ifdef ALIF_DEBUG
	traverseproc traverse = ALIF_TYPE(op_)->traverse_;
	(void)traverse(op, visitValidate, op_);
#endif
}

void alifObject_gcUnTrack(void* _opRaw)
{
	AlifObject* op_ = ALIFOBJECT_CAST(_opRaw);

	if (ALIFSUBOBJECT_GC_ISTRACKED(op_)) {
		ALIFOBJECT_GC_UNTRACK(op_);
	}
}


void alifSubObjectGC_link(AlifObject* _gc) { // 1983
	AlifGCHead* gc = AS_GC(_gc);

	AlifThread* thread = alifThread_get();
	AlifGC* gcState = &thread->interpreter->gc;

	gc->gcNext = 0;
	gc->gcPrev = 0;
	gcState->young.count++;
	gcState->heapSize++;

	if (gcState->young.count > gcState->young.threshold and gcState->enabled and gcState->young.threshold
		//and
		//!alifAtomicLoad_intRelax(&gcState->collecting)
		//and
		//!alifErrpr_occurred(thread)
		)
	{
		//alif_scheduleGC(thread);
	}
}


static AlifObject* gc_alloc(AlifTypeObject* _tp, AlifSizeT _size, AlifSizeT _preSize) { // 2014

	AlifThread* thread = alifThread_get();
	if (_size > ALIF_SIZET_MAX - _preSize) {
		// memory error
		return nullptr; //
	}

	AlifSizeT size = _size + _preSize;
	char* mem = (char*)alifMem_objAlloc(size);
	if (mem == nullptr) {
		// memory error
		return nullptr; //
	}

	((AlifObject**)mem)[0] = nullptr;
	((AlifObject**)mem)[1] = nullptr;
	AlifObject* gc = (AlifObject*)(mem + _preSize);

	alifSubObjectGC_link(gc);

	return gc;
}



AlifObject* alifSubObjectGC_new(AlifTypeObject* _tp) { // 2034
	AlifSizeT preSize = alifSubType_preHeaderSize(_tp);
	AlifSizeT size = alifSubObject_size(_tp);
	if (alifSubType_hasFeature(_tp, ALIFTPFLAGS_INLINE_VALUES)) {
		size += alifInline_valuesSize(_tp);
	}

	AlifObject* gc = gc_alloc(_tp, size, preSize);
	if (gc == nullptr) return nullptr;

	alifSubObject_init(gc, _tp);
	return gc;
}


AlifVarObject* alifSubObjectGC_newVar(AlifTypeObject* _tp, int64_t _nItems)
{
	AlifVarObject* op_;

	if (_nItems < 0) {
		return NULL;
	}
	size_t preSize = alifSubType_preHeaderSize(_tp);
	size_t size_ = alifSubObject_varSize(_tp, _nItems);
	op_ = (AlifVarObject*)gc_alloc(_tp, size_, preSize);
	if (op_ == NULL) {
		return NULL;
	}
	alifSubObject_initVar(op_, _tp, _nItems);
	return op_;
}

void alifObject_gcDel(void* op)
{
	size_t presize = alifSubType_preHeaderSize(((AlifObject*)op)->type_);
	AlifGCHead* g = AS_GC((AlifObject*)op);
	if (ALIFSUBOBJECT_GC_ISTRACKED(op)) {
		gc_list_remove(g);
	}
	AlifGC* gcState = get_gc_state();
	if (gcState->young.count > 0) {
		gcState->young.count--;
	}
	gcState->heapSize--;
	alifMem_objFree(((char*)op)-presize);
}
