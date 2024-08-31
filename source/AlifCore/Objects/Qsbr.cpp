#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_Lock.h"
#include "AlifCore_Qsbr.h"
#include "AlifCore_State.h"



#define QSBR_DEFERRED_LIMIT 10 // 47


static inline bool alifQbsr_goalReached(class QsbrThreadState* _qsbr, uint64_t _goal) { // 100
	uint64_t rdSeq = alifAtomic_loadUint64(&_qsbr->shared->rdSeq);
	return QSBR_LEQ(_goal, rdSeq);
}


uint64_t alifQsbr_advance(class QsbrShared* _shared) { // 111
	return alifAtomic_addUint64(&_shared->wrSeq, QSBR_INCR) + QSBR_INCR;
}

uint64_t alifQsbr_deferredAdvance(class QsbrThreadState* _qsbr) { // 120
	if (++_qsbr->deferrals < QSBR_DEFERRED_LIMIT) {
		return alifQsbr_sharedCurrent(_qsbr->shared) + QSBR_INCR;
	}
	_qsbr->deferrals = 0;
	return alifQsbr_advance(_qsbr->shared);
}

static uint64_t qsbr_pollScan(class QsbrShared* _shared) { // 130

	alifAtomic_fenceSeqCst();

	uint64_t minSeq = alifAtomic_loadUint64(&_shared->wrSeq);
	class QsbrPad* array = _shared->array;
	for (AlifSizeT i = 0, size = _shared->size; i != size; i++) {
		class QsbrThreadState* qsbr = &array[i].qsbr;

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


bool alifQsbr_poll(class QsbrThreadState* _qsbr, uint64_t _goal) { // 161

	if (alifQbsr_goalReached(_qsbr, _goal)) {
		return true;
	}

	uint64_t rdSeq = qsbr_pollScan(_qsbr->shared);
	return QSBR_LEQ(_goal, rdSeq);
}
