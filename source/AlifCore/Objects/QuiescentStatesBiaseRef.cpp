#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_Lock.h"
#include "AlifCore_QSBR.h"
#include "AlifCore_State.h"






#define QSBR_DEFERRED_LIMIT 10 // 47



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
