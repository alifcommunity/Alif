#include "alif.h"

#include "AlifCore_Lock.h"
#include "AlifCore_QSBR.h"
#include "AlifCore_State.h"












void alifQSBR_attach(QSBRThreadState* qsbr) { // 173
	uint64_t seq = alifQSBR_sharedCurrent(qsbr->shared);
	alifAtomic_storeUint64(&qsbr->seq, seq);  // needs seq_cst
}
