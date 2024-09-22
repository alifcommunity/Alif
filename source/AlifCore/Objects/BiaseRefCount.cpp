#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_Eval.h"
#include "AlifCore_LList.h"
#include "AlifCore_State.h"













static BRCBucket* get_bucket(AlifInterpreter* _interp, uintptr_t _threadID) { // 29
	return &_interp->brc.table[_threadID % ALIF_BRC_NUM_BUCKETS];
}


static AlifThreadImpl* find_threadState(BRCBucket* _bucket, uintptr_t _threadID) { // 36
	LListNode* node{};
	LLIST_FOR_EACH(node, &_bucket->root) {
		AlifThreadImpl* ts = LLIST_DATA(node, AlifThreadImpl, brc.bucketNode);
		if (ts->brc.threadID == _threadID) {
			return ts;
		}
	}
	return nullptr;
}



void alifBRC_queueObject(AlifObject* ob) { // 53
	AlifInterpreter* interp = _alifInterpreter_get();

	uintptr_t threadID = alifAtomic_loadUintptr(&ob->threadID);
	if (threadID == 0) {
		ALIF_DECREF(ob);
		return;
	}

	BRCBucket* bucket = get_bucket(interp, threadID);
	ALIFMUTEX_LOCK(&bucket->mutex);
	AlifThreadImpl* tstate = find_threadState(bucket, threadID);
	if (tstate == nullptr) {
		AlifSizeT refcount = alif_explicitMergeRefcount(ob, -1);
		ALIFMUTEX_UNLOCK(&bucket->mutex);
		if (refcount == 0) {
			alif_dealloc(ob);
		}
		return;
	}

	if (alifObjectStack_push(&tstate->brc.objectsToMerge, ob) < 0) {
		ALIFMUTEX_UNLOCK(&bucket->mutex);

		alifEval_stopTheWorld(interp);
		AlifSizeT refcount = alif_explicitMergeRefcount(ob, -1);
		alifEval_startTheWorld(interp);

		if (refcount == 0) {
			alif_dealloc(ob);
		}
		return;
	}

	alifSet_evalBreakerBit(&tstate->base, ALIF_EVAL_EXPLICIT_MERGE_BIT);

	ALIFMUTEX_UNLOCK(&bucket->mutex);
}


void alifBRC_initState(AlifInterpreter* _interp) { // 134
	BRCState* brc = &_interp->brc;
	for (AlifSizeT i = 0; i < ALIF_BRC_NUM_BUCKETS; i++) {
		llist_init(&brc->table[i].root);
	}
}



void alifBRC_initThread(AlifThread* _thread) { // 143
	BRCThreadState* brc = &((AlifThreadImpl*)_thread)->brc;
	uintptr_t threadID = alif_threadID();

	BRCBucket* bucket = get_bucket(_thread->interpreter, threadID);
	ALIFMUTEX_LOCK(&bucket->mutex);
	brc->threadID = threadID;
	llist_insertTail(&bucket->root, &brc->bucketNode);
	ALIFMUTEX_UNLOCK(&bucket->mutex);
}








#ifdef ALIF_GIL_DISABLED // 309







AlifSizeT alif_explicitMergeRefcount(AlifObject* op, AlifSizeT extra) { // 405

	AlifSizeT local = (AlifSizeT)op->refLocal;
	alifAtomic_storeUint32Relaxed(&op->refLocal, 0);
	alifAtomic_storeUintptrRelaxed(&op->threadID, 0);

	AlifSizeT refcnt{};
	AlifSizeT new_shared{};
	AlifSizeT shared = alifAtomic_loadSizeRelaxed(&op->refShared);
	do {
		refcnt = ALIF_ARITHMETIC_RIGHT_SHIFT(AlifSizeT, shared, ALIF_REF_SHARED_SHIFT);
		refcnt += local;
		refcnt += extra;

		new_shared = ALIF_REF_SHARED(refcnt, ALIF_REF_MERGED);
	} while (!alifAtomic_compareExchangeSize(&op->refShared,
		&shared, new_shared));
	return refcnt;
}
#endif  /* ALIF_GIL_DISABLED */
