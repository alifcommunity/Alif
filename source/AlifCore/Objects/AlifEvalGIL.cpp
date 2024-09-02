#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_State.h"



#include "CondVar.h" // 107

// 109
#define MUTEX_INIT(_mut) \
    if (alifMutex_INIT(&(_mut))) { \
		return;	\
        /*ALIF_FATALERROR("alifMutex_INIT(" #_mut ") failed");*/ \
	};
#define MUTEX_FINI(_mut) \
    if (alifMutex_FINI(&(_mut))) { \
		return;		\
        /*ALIF_FATALERROR("alifMutex_FINI(" #_mut ") failed");*/	\
	};
#define MUTEX_LOCK(_mut) \
    if (alifMutex_LOCK(&(_mut))) { \
		return;		\
        /*ALIF_FATALERROR("alifMutex_LOCK(" #_mut ") failed");*/	\
	};
#define MUTEX_UNLOCK(_mut) \
    if (alifMutex_UNLOCK(&(_mut))) { \
		return;		\
        /*ALIF_FATALERROR("alifMutex_UNLOCK(" #_mut ") failed");*/	\
	};
#define COND_INIT(_cond) \
    if (alifCond_INIT(&(_cond))) { \
		return; \
        /*ALIF_FATALERROR("alifCond_INIT(" #_cond ") failed");*/	\
	};
#define COND_FINI(_cond) \
    if (alifCond_FINI(&(_cond))) { \
		return;		\
        /*ALIF_FATALERROR("alifCond_FINI(" #_cond ") failed");*/	\
	};
#define COND_SIGNAL(_cond) \
    if (alifCond_SIGNAL(&(_cond))) { \
		return;	\
        /*ALIF_FATALERROR("alifCond_SIGNAL(" #_cond ") failed");*/	\
	};
#define COND_WAIT(_cond, _mut) \
    if (alifCond_WAIT(&(_cond), &(_mut))) { \
		return;	\
        /*ALIF_FATALERROR("alifCond_WAIT(" #_cond ") failed");*/	\
	};
#define COND_TIMED_WAIT(_cond, _mut, _microseconds, _timeOutResult) \
    { \
        AlifIntT r = alifCond_TIMEDWAIT(&(_cond), &(_mut), (_microseconds)); \
        if (r < 0) \
			return; \
            /*ALIF_FATALERROR("alifCond_WAIT(" #_cond ") failed");*/ \
        if (r) /* 1 == timeout, 2 == impl. can't say, so assume timeout */ \
            _timeOutResult = 1; \
        else \
            _timeOutResult = 0; \
    } \


static AlifIntT gil_created(GILDureRunState* gil) { // 154
	if (gil == nullptr) {
		return 0;
	}
	return (alifAtomic_loadIntAcquire(&gil->locked) >= 0);
}

static void create_gil(struct GILDureRunState* gil) { // 162
	MUTEX_INIT(gil->mutex);
#ifdef FORCE_SWITCHING
	MUTEX_INIT(gil->switchMutex);
#endif
	COND_INIT(gil->cond);
#ifdef FORCE_SWITCHING
	COND_INIT(gil->switchCond);
#endif
	alifAtomic_storePtrRelaxed(&gil->lastHolder, 0);
	alifAtomic_storeIntRelease(&gil->locked, 0);
}

static void destroy_gil(GILDureRunState* gil) { // 177
	COND_FINI(gil->cond);
	MUTEX_FINI(gil->mutex);
#ifdef FORCE_SWITCHING
	COND_FINI(gil->switchCond);
	MUTEX_FINI(gil->switchMutex);
#endif
	alifAtomic_storeIntRelease(&gil->locked, -1);
}

static inline void drop_gilImpl(AlifThread* _thread, GILDureRunState* _gil) { // 201
	MUTEX_LOCK(_gil->mutex);
	alifAtomic_storeIntRelaxed(&_gil->locked, 0);
	if (_thread != nullptr) {
		_thread->status.holdsGIL = 0;
	}
	COND_SIGNAL(_gil->cond);
	MUTEX_UNLOCK(_gil->mutex);
}

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
		alifEval_breakerBitIsSet(_thread, ALIF_GIL_DROP_REQUEST_BIT)) {
		MUTEX_LOCK(gil->switchMutex);
		if (((AlifThread*)alifAtomic_loadPtrRelaxed(&gil->lastHolder)) == _thread)
		{
			alifUnset_evalBreakerBit(_thread, ALIF_GIL_DROP_REQUEST_BIT);
			COND_WAIT(gil->switchCond, gil->switchMutex);
		}
		MUTEX_UNLOCK(gil->switchMutex);
	}
}






static void take_gil(AlifThread* _thread) { // 284
	AlifIntT err = errno;

	if (alifThread_mustExit(_thread)) {
		alifThread_exitThread();
	}

	AlifInterpreter* interp = _thread->interpreter;
	GILDureRunState* gil_ = interp->eval.gil_;
#ifdef ALIF_GIL_DISABLED
	if (!alifAtomic_loadIntRelaxed(&gil_->enabled)) {
		return;
	}
#endif

	MUTEX_LOCK(gil_->mutex);

	AlifIntT dropRequested = 0;
	while (alifAtomic_loadIntRelaxed(&gil_->locked)) {
		unsigned long savedSwitchNum = gil_->switchNumber;

		unsigned long interval = (gil_->interval >= 1 ? gil_->interval : 1);
		AlifIntT timedOut = 0;
		COND_TIMED_WAIT(gil_->cond, gil_->mutex, interval, timedOut);

		if (timedOut and
			alifAtomic_loadIntRelaxed(&gil_->locked) and
			gil_->switchNumber == savedSwitchNum)
		{
			AlifThread* holderThread =
				(AlifThread*)alifAtomic_loadPtrRelaxed(&gil_->lastHolder);
			if (alifThread_mustExit(_thread)) {
				MUTEX_UNLOCK(gil_->mutex);
				if (dropRequested) {
					alifUnset_evalBreakerBit(holderThread, ALIF_GIL_DROP_REQUEST_BIT);
				}
				alifThread_exitThread();
			}

			alifSet_evalBreakerBit(holderThread, ALIF_GIL_DROP_REQUEST_BIT);
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

static void init_sharedGIL(AlifInterpreter* _interp, GILDureRunState* _gil) { // 466
	_interp->eval.gil_ = _gil;
	_interp->eval.ownGIL = 0;
}

static void init_ownGIL(AlifInterpreter* _interp, GILDureRunState* _gil) { // 474
#ifdef ALIF_GIL_DISABLED
	const AlifConfig* config = alifInterpreter_getConfig(_interp);
	_gil->enabled = (config->enableGIL == AlifConfigGIL_::AlifConfig_GIL_Enable)
		? INT_MAX
		: 0;
#endif
	create_gil(_gil);
	_interp->eval.gil_ = _gil;
	_interp->eval.ownGIL = 1;
}

void alifEval_initGIL(AlifThread* _thread, AlifIntT _ownGIL) { // 488
	if (!_ownGIL) {
		AlifInterpreter* main_interp = alifInterpreter_main();
		GILDureRunState* gil = main_interp->eval.gil_;
		init_sharedGIL(_thread->interpreter, gil);
	}
	else {
		init_ownGIL(_thread->interpreter, &_thread->interpreter->gil_);
	}

	alifThread_attach(_thread);
}



void alifEval_finiGIL(AlifInterpreter* interp) { // 509
	GILDureRunState* gil = interp->eval.gil_;
	if (gil == nullptr) {
		return;
	}
	else if (!interp->eval.ownGIL) {
		interp->eval.gil_ = nullptr;
		return;
	}

	if (!gil_created(gil)) {
		return;
	}

	destroy_gil(gil);
	interp->eval.gil_ = nullptr;
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
