#include "alif.h"

#include "AlifCore_State.h"















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
