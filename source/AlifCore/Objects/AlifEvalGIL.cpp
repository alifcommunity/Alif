#include "alif.h"

#include "AlifCore_State.h"



#include "CondVar.h" // 107

// 109
#define MUTEX_LOCK(_mut) \
    if (alifMutex_LOCK(&(_mut))) { \
        /*alif_fatalError("alifMutex_LOCK(" #_mut ") failed");*/	\
	};
#define MUTEX_UNLOCK(_mut) \
    if (alifMutex_UNLOCK(&(_mut))) { \
        /*alif_fatalError("alifMutex_UNLOCK(" #_mut ") failed");*/	\
	};


static void take_gil(AlifThread* tstate) { // 284
	int err = errno;

	if (alifThread_mustExit(tstate)) {
		alifThread_exitThread();
	}

	AlifInterpreter* interp = tstate->interpreter;
	GILDureRunState* gil = interp->eval.gil_;
	if (!alifAtomic_loadIntRelaxed(&gil->enabled)) {
		return;
	}

	MUTEX_LOCK(gil->mutex);

	int drop_requested = 0;
	while (alifAtomic_loadIntRelaxed(&gil->locked)) {
		unsigned long saved_switchnum = gil->switchNumber;

		unsigned long interval = (gil->interval >= 1 ? gil->interval : 1);
		int timed_out = 0;
		COND_TIMED_WAIT(gil->cond, gil->mutex, interval, timed_out);

		if (timed_out and
			alifAtomic_loadIntRelaxed(&gil->locked) and
			gil->switchNumber == saved_switchnum)
		{
			AlifThread* holder_tstate =
				(AlifThread*)alifAtomic_loadPtrRelaxed(&gil->lastHolder);
			if (alifThread_mustExit(tstate)) {
				MUTEX_UNLOCK(gil->mutex);
				// gh-96387: If the loop requested a drop request in a previous
				// iteration, reset the request. Otherwise, drop_gil() can
				// block forever waiting for the thread which exited. Drop
				// requests made by other threads are also reset: these threads
				// may have to request again a drop request (iterate one more
				// time).
				if (drop_requested) {
					alifUnset_evalBreakerBit(holder_tstate, ALIFGIL_DROP_REQUEST_BIT);
				}
				alifThread_exitThread();
			}

			alifSet_evalBreakerBit(holder_tstate, ALIFGIL_DROP_REQUEST_BIT);
			drop_requested = 1;
		}
	}

	if (!alifAtomic_loadIntRelaxed(&gil->enabled)) {
		COND_SIGNAL(gil->cond);
		MUTEX_UNLOCK(gil->mutex);
		return;
	}


	MUTEX_LOCK(gil->switchMutex);

	/* We now hold the GIL */
	alifAtomic_storeIntRelaxed(&gil->locked, 1);
	ALIF_ANNOTATE_RWLOCK_ACQUIRED(&gil->locked, /*is_write=*/1);

	if (tstate != (AlifThread*)alifAtomic_loadPtrRelaxed(&gil->lastHolder)) {
		alifAtomic_storePtrRelaxed(&gil->lastHolder, tstate);
		++gil->switchNumber;
	}

	COND_SIGNAL(gil->switchCond);
	MUTEX_UNLOCK(gil->switchMutex);

	if (alifThread_mustExit(tstate)) {
		MUTEX_UNLOCK(gil->mutex);

		drop_gil(interp, nullptr, 1);
		alifThread_exitThread();
	}

	tstate->status.holdsGIL = 1;
	alifUnset_evalBreakerBit(tstate, ALIF_GIL_DROP_REQUEST_BIT);
	updateEval_breakerForThread(interp, tstate);

	MUTEX_UNLOCK(gil->mutex);

	errno = err;
	return;
}








void alifEval_acquireLock(AlifThread* _thread) { // 574
	ALIF_ENSURETHREADNOTNULL(_thread);
	take_gil(_thread);
}


void alifEval_acquireThread(AlifThread* _thread) { // 591
	ALIF_ENSURETHREADNOTNULL(_thread);
	alifThread_attach(_thread);
}


void alifEval_releaseThread(AlifThread* _thread) { // 598
	alifThread_detach(_thread);
}
