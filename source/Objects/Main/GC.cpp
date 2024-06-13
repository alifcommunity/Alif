#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"
#include "AlifCore_AlifState.h"


typedef class AlifGCDureRun AlifGC;



#define AS_GC(_gc) alif_asGC(_gc) // 50




void alifObjectGC_link(AlifObject* _gc) { // 1983
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

	alifObjectGC_link(gc);

	return gc;
}



AlifObject* alifObjectGC_new(AlifTypeObject* _tp) { // 2034
	//AlifSizeT preSize = alifType_preHeaderSize(_tp);
	//AlifSizeT size = alifObject_size(_tp);
	
	AlifSizeT preSize = 1 * 2 * sizeof(AlifObject*);
	AlifSizeT size = (AlifSizeT)_tp->basicSize;

	AlifObject* gc = gc_alloc(_tp, size, preSize);
	if (gc == nullptr) return nullptr;

	alifSubObject_init(gc, _tp);
	return gc;
}