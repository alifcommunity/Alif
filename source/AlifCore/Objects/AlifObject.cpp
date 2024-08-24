#include "alif.h"

#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"





void alif_decRefSharedDebug(AlifObject* _obj, const char* _filename, AlifIntT _lineno) { // 322
	AlifIntT shouldQueue;

	AlifSizeT newShared;
	AlifSizeT shared = alifAtomic_loadSizeRelaxed(&_obj->refShared);
	do {
		shouldQueue = (shared == 0 or shared == ALIF_REF_MAYBE_WEAKREF);

		if (shouldQueue) {
			newShared = ALIF_REF_QUEUED;
		}
		else {
			newShared = shared - (1 << ALIF_REF_SHARED_SHIFT);
		}

	} while (!alifAtomic_compareExchangeSize(&_obj->refShared,
		&shared, newShared));

	if (shouldQueue) {
		alifBRC_queueObject(_obj);
	}
	else if (newShared == ALIF_REF_MERGED) {
		// refcount is zero AND merged
		alif_dealloc(_obj);
	}
}



void alif_decRefShared(AlifObject* _o) { // 368
	alif_decRefSharedDebug(_o, nullptr, 0);
}


void alif_mergeZeroLocalRefcount(AlifObject* _op) { // 374

	AlifSizeT shared = alifAtomic_loadSizeAcquire(&_op->refShared);
	if (shared == 0) {
		alif_dealloc(_op);
		return;
	}

	alifAtomic_storeUintptrRelaxed(&_op->threadID, 0);

	AlifSizeT newShared{};
	do {
		newShared = (shared & ~ALIFREF_SHARED_FLAG_MASK) | ALIF_REF_MERGED;
	} while (!alifAtomic_compareExchangeSize(&_op->refShared,
		&shared, newShared));

	if (newShared == ALIF_REF_MERGED) {
		alif_dealloc(_op);
	}
}









static inline void new_reference(AlifObject* _op) { // 2405
	_op->threadID = alif_threadId();
	_op->padding = 0;
	_op->mutex = {.bits = 0};
	_op->gcBits = 0;
	_op->refLocal = 1;
	_op->refShared = 0;

	RefTracerDureRunState* tracer = &_alifDureRun_.refTracer;
	if (tracer->tracerFunc != nullptr) {
		void* data = tracer->tracerData;
		tracer->tracerFunc(_op, AlifRefTracerEvent_::Alif_RefTracer_Create, data);
	}
}

void alif_newReference(AlifObject* _op) { // 2429
	new_reference(_op);
}



void alif_newReferenceNoTotal(AlifObject* _op) { //2438
	new_reference(_op);
}

void alif_setImmortalUntracked(AlifObject* _op) { // 2444

	_op->threadID = ALIF_UNOWNED_TID;
	_op->refLocal = ALIF_IMMORTAL_REFCNT_LOCAL;
	_op->refShared = 0;
}

void alif_setImmortal(AlifObject* _op) { // 2463
	if (alifObject_isGC(_op) and ALIFOBJECT_GC_IS_TRACKED(_op)) {
		ALIFOBJECT_GC_UNTRACK(_op);
	}
	alif_setImmortalUntracked(_op);
}





void alif_dealloc(AlifObject* op) { // 2868
	AlifTypeObject* type = ALIF_TYPE(op);
	Destructor dealloc = type->dealloc;

	RefTracerDureRunState* tracer = &_alifDureRun_.refTracer;
	if (tracer->tracerFunc != nullptr) {
		void* data = tracer->tracerData;
		tracer->tracerFunc(op, AlifRefTracerEvent_::Alif_RefTracer_Destroy, data);
	}

	(*dealloc)(op);
}
