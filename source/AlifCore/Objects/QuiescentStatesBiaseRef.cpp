#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_Lock.h"
#include "AlifCore_QSBR.h"
#include "AlifCore_State.h"




#define MIN_ARRAY_SIZE 8 // 42

#define QSBR_DEFERRED_LIMIT 10 // 47


static QSBRThreadState* qsbr_allocate(QSBRShared* _shared) { // 49
	QSBRThreadState* qsbr = _shared->freeList;
	if (qsbr == nullptr) {
		return nullptr;
	}
	_shared->freeList = qsbr->freeListNext;
	qsbr->freeListNext = nullptr;
	qsbr->shared = _shared;
	qsbr->allocated = true;
	return qsbr;
}



static void initialize_newArray(QSBRShared* _shared) { // 64
	for (AlifSizeT i = 0; i != _shared->size; i++) {
		QSBRThreadState* qsbr = &_shared->array[i].qsbr;
		if (qsbr->thread != nullptr) {
			AlifThreadImpl* tstate = (AlifThreadImpl*)qsbr->thread;
			tstate->qsbr = qsbr;
		}
		if (!qsbr->allocated) {
			// Push to freelist
			qsbr->freeListNext = _shared->freeList;
			_shared->freeList = qsbr;
		}
	}
}


static AlifIntT grow_threadArray(QSBRShared* _shared) { // 83
	AlifSizeT newSize = _shared->size * 2;
	if (newSize < MIN_ARRAY_SIZE) {
		newSize = MIN_ARRAY_SIZE;
	}

	QSBRPad* array = (QSBRPad*)alifMem_dataAlloc(newSize * sizeof(*array)); // need review
	if (array == nullptr) {
		return -1;
	}

	QSBRPad* old = _shared->array;
	if (old != nullptr) {
		memcpy(array, _shared->array, _shared->size * sizeof(*array));
	}

	_shared->array = array;
	_shared->size = newSize;
	_shared->freeList = nullptr;
	initialize_newArray(_shared);

	alifMem_dataFree(old);
	return 0;
}



uint64_t alifQSBR_advance(QSBRShared* _shared) { // 111
	return alifAtomic_addUint64(&_shared->wrSeq, QSBR_INCR) + QSBR_INCR;
}

uint64_t alifQSBR_deferredAdvance(QSBRThreadState* _qsbr) { // 120
	if (++_qsbr->deferrals < QSBR_DEFERRED_LIMIT) {
		return alifQSBR_sharedCurrent(_qsbr->shared) + QSBR_INCR;
	}
	_qsbr->deferrals = 0;
	return alifQSBR_advance(_qsbr->shared);
}

static uint64_t qsbr_pollScan(QSBRShared* _shared) { // 130

	alifAtomic_fenceSeqCst();

	uint64_t minSeq = alifAtomic_loadUint64(&_shared->wrSeq);
	QSBRPad* array = _shared->array;
	for (AlifSizeT i = 0, size = _shared->size; i != size; i++) {
		class QSBRThreadState* qsbr = &array[i].qsbr;

		uint64_t seq = alifAtomic_loadUint64(&qsbr->seq);
		if (seq != QSBR_OFFLINE && QSBR_LT(seq, minSeq)) {
			minSeq = seq;
		}
	}

	uint64_t rdSeq = alifAtomic_loadUint64(&_shared->rdSeq);
	if (QSBR_LT(rdSeq, minSeq)) {
		(void)alifAtomic_compareExchangeUint64(&_shared->rdSeq, &rdSeq, minSeq);
		rdSeq = minSeq;
	}

	return rdSeq;
}


bool alifQSBR_poll(QSBRThreadState* _qsbr, uint64_t _goal) { // 161

	if (alifQSBR_goalReached(_qsbr, _goal)) {
		return true;
	}

	uint64_t rdSeq = qsbr_pollScan(_qsbr->shared);
	return QSBR_LEQ(_goal, rdSeq);
}





void alifQSBR_attach(QSBRThreadState* _qsbr) { // 173
	uint64_t seq = alifQSBR_sharedCurrent(_qsbr->shared);
	alifAtomic_storeUint64(&_qsbr->seq, seq);  // needs seq_cst
}

void alifQSBR_detach(QSBRThreadState* _qsbr) { // 182
	alifAtomic_storeUint64Release(&_qsbr->seq, QSBR_OFFLINE);
}




AlifSizeT alifQSBR_reserve(AlifInterpreter* _interp) { // 190
	QSBRShared* shared = &_interp->qsbr;

	ALIFMUTEX_LOCK(&shared->mutex);
	QSBRThreadState* qsbr = qsbr_allocate(shared);

	if (qsbr == nullptr) {
		alifEval_stopTheWorld(_interp);
		if (grow_threadArray(shared) == 0) {
			qsbr = qsbr_allocate(shared);
		}
		alifEval_startTheWorld(_interp);
	}
	ALIFMUTEX_UNLOCK(&shared->mutex);

	if (qsbr == nullptr) {
		return -1;
	}

	return (QSBRPad*)qsbr - shared->array;
}





void alifQSBR_register(AlifThreadImpl* _thread,
	AlifInterpreter* _interp, AlifSizeT _index) { // 219
	QSBRShared* shared = &_interp->qsbr;

	ALIFMUTEX_LOCK(&shared->mutex);
	QSBRThreadState* qsbr = &_interp->qsbr.array[_index].qsbr;
	qsbr->thread = (AlifThread*)_thread;
	_thread->qsbr = qsbr;
	ALIFMUTEX_UNLOCK(&shared->mutex);
}
