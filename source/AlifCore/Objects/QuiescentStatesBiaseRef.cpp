#include "alif.h"

#include "AlifCore_Lock.h"
#include "AlifCore_QSBR.h"
#include "AlifCore_State.h"












void alifQSBR_attach(QSBRThreadState* _qsbr) { // 173
	uint64_t seq = alifQSBR_sharedCurrent(_qsbr->shared);
	alifAtomic_storeUint64(&_qsbr->seq, seq);  // needs seq_cst
}

void alifQSBR_detach(QSBRThreadState* _qsbr) { // 182
	alifAtomic_storeUint64Release(&_qsbr->seq, QSBR_OFFLINE);
}
