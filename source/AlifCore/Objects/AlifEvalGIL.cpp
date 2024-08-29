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






static void drop_gil(AlifInterpreter* _interp,
	AlifThread* _thread, AlifIntT _finalRelease) { // 214

	AlifEval* ceval = &_interp->eval;

	GILDureRunState* gil = ceval->gil_;
	if (_thread != nullptr and !_thread->status.holdsGIL) {
		return;
	}
	if (!alifAtomic_loadIntRelaxed(&gil->locked)) {
		//alif_fatalError("drop_gil: GIL is not locked");
		return;
	}

	if (!_finalRelease) {
		alifAtomic_storePtrRelaxed(&gil->lastHolder, _thread);
	}

	drop_gilImpl(_thread, gil);

	if (!_finalRelease and
		alifEval_breakerBitIsSet(_thread, ALIF_GILDROP_REQUEST_BIT)) {
		MUTEX_LOCK(gil->switchMutex);
		if (((AlifThread*)alifAtomic_loadPtrRelaxed(&gil->lastHolder)) == _thread)
		{
			alifUnset_evalBreakerBit(_thread, ALIF_GILDROP_REQUEST_BIT);
			COND_WAIT(gil->switchCond, gil->switchMutex);
		}
		MUTEX_UNLOCK(gil->switchMutex);
	}
}






static void take_gil(AlifThread* _thread) { // 284
	int err = errno;

	if (alifThread_mustExit(_thread)) {
		alifThread_exitThread();
	}

	AlifInterpreter* interp = _thread->interpreter;
	GILDureRunState* gil_ = interp->eval.gil_;
	if (!alifAtomic_loadIntRelaxed(&gil_->enabled)) {
		return;
	}

	MUTEX_LOCK(gil_->mutex);

	AlifIntT dropRequested = 0;
	while (alifAtomic_loadIntRelaxed(&gil_->locked)) {
		unsigned long saved_switchnum = gil_->switchNumber;

		unsigned long interval = (gil_->interval >= 1 ? gil_->interval : 1);
		int timed_out = 0;
		COND_TIMED_WAIT(gil_->cond, gil_->mutex, interval, timed_out);

		if (timed_out and
			alifAtomic_loadIntRelaxed(&gil_->locked) and
			gil_->switchNumber == saved_switchnum)
		{
			AlifThread* holder_tstate =
				(AlifThread*)alifAtomic_loadPtrRelaxed(&gil_->lastHolder);
			if (alifThread_mustExit(_thread)) {
				MUTEX_UNLOCK(gil_->mutex);
				// gh-96387: If the loop requested a drop request in a previous
				// iteration, reset the request. Otherwise, drop_gil() can
				// block forever waiting for the thread which exited. Drop
				// requests made by other threads are also reset: these threads
				// may have to request again a drop request (iterate one more
				// time).
				if (dropRequested) {
					alifUnset_evalBreakerBit(holder_tstate, ALIFGIL_DROP_REQUEST_BIT);
				}
				alifThread_exitThread();
			}

			alifSet_evalBreakerBit(holder_tstate, ALIFGIL_DROP_REQUEST_BIT);
			dropRequested = 1;
		}
	}

	if (!alifAtomic_loadIntRelaxed(&gil_->enabled)) {
		COND_SIGNAL(gil_->cond);
		MUTEX_UNLOCK(gil_->mutex);
		return;
	}


	MUTEX_LOCK(gil_->switchMutex);

	/* We now hold the GIL */
	alifAtomic_storeIntRelaxed(&gil_->locked, 1);
	ALIF_ANNOTATE_RWLOCK_ACQUIRED(&gil_->locked, /*is_write=*/1);

	if (_thread != (AlifThread*)alifAtomic_loadPtrRelaxed(&gil_->lastHolder)) {
		alifAtomic_storePtrRelaxed(&gil_->lastHolder, _thread);
		++gil_->switchNumber;
	}

	COND_SIGNAL(gil_->switchCond);
	MUTEX_UNLOCK(gil_->switchMutex);

	if (alifThread_mustExit(_thread)) {
		MUTEX_UNLOCK(gil_->mutex);

		drop_gil(interp, nullptr, 1);
		alifThread_exitThread();
	}

	_thread->status.holdsGIL = 1;
	alifUnset_evalBreakerBit(_thread, ALIF_GIL_DROP_REQUEST_BIT);
	updateEval_breakerForThread(interp, _thread);

	MUTEX_UNLOCK(gil_->mutex);

	errno = err;
	return;
}








void alifEval_acquireLock(AlifThread* _thread) { // 574
	ALIF_ENSURETHREADNOTNULL(_thread);
	take_gil(_thread);
}

void alifEval_releaseLock(AlifInterpreter* _interp,
	AlifThread* _thread, AlifIntT _finalRelease) { // 581
	drop_gil(_interp, _thread, _finalRelease);
}


void alifEval_acquireThread(AlifThread* _thread) { // 591
	ALIF_ENSURETHREADNOTNULL(_thread);
	alifThread_attach(_thread);
}


void alifEval_releaseThread(AlifThread* _thread) { // 598
	alifThread_detach(_thread);
}
