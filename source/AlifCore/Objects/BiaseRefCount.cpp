#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_LList.h"
#include "AlifCore_State.h"













static BRCBucket* get_bucket(AlifInterpreter* _interp, uintptr_t _threadID) { // 29
	return &_interp->brc.table[_threadID % ALIF_BRC_NUM_BUCKETS];
}




void alifBRC_queueObject(AlifObject* ob) { // 53
	AlifInterpreter* interp = alifInterpreter_get();

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

		aifEval_stopTheWorld(interp);
		AlifSizeT refcount = alif_explicitMergeRefcount(ob, -1);
		alifEval_startTheWorld(interp);

		if (refcount == 0) {
			alif_dealloc(ob);
		}
		return;
	}

	alifSetEval_breakerBit(&tstate->base, ALIFEVAL_EXPLICIT_MERGE_BIT);

	ALIFMUTEX_UNLOCK(&bucket->mutex);
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
