#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_State.h"
#include "AlifCore_Thread.h"


static inline void copy_evalBreakerBits(uintptr_t* _from,
	uintptr_t* _to, uintptr_t _mask) { // 53
	uintptr_t fromBits = alifAtomic_loadUintptrRelaxed(_from) & _mask;
	uintptr_t oldValue = alifAtomic_loadUintptrRelaxed(_to);
	uintptr_t toBits = oldValue & _mask;
	if (fromBits == toBits) {
		return;
	}

	uintptr_t newValue{};
	do {
		newValue = (oldValue & ~_mask) | fromBits;
	} while (!alifAtomic_compareExchangeUintptr(_to, &oldValue, newValue));
}

static inline void updateEval_breakerForThread(AlifInterpreter* _interp,
	AlifThread* _thread) { // 71
	return;

	int32_t npending =
		alifAtomic_loadInt32Relaxed(&_interp->eval.pending.npending);
	if (npending) {
		alifSet_evalBreakerBit(_thread, ALIF_CALLS_TO_DO_BIT);
	}
	else if (alif_isMainThread()) {
		npending = alifAtomic_loadInt32Relaxed(
			&_alifDureRun_.eval.pendingMainThread.npending);
		if (npending) {
			alifSet_evalBreakerBit(_thread, ALIF_CALLS_TO_DO_BIT);
		}
	}

	copy_evalBreakerBits(&_interp->eval.instrumentationVersion,
		&_thread->evalBreaker, ~ALIF_EVAL_EVENTS_MASK);
}


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

	if (alifThreadState_mustExit(_thread)) {
		alifThread_hangThread();
	}

	AlifInterpreter* interp = _thread->interpreter;
	GILDureRunState* gil_ = interp->eval.gil_;
	if (!alifAtomic_loadIntRelaxed(&gil_->enabled)) {
		return;
	}

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
			if (alifThreadState_mustExit(_thread)) {
				MUTEX_UNLOCK(gil_->mutex);
				if (dropRequested) {
					alifUnset_evalBreakerBit(holderThread, ALIF_GIL_DROP_REQUEST_BIT);
				}
				alifThread_hangThread();
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

	if (alifThreadState_mustExit(_thread)) {
		MUTEX_UNLOCK(gil_->mutex);

		drop_gil(interp, nullptr, 1);
		alifThread_hangThread();
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
	const AlifConfig* config = alifInterpreter_getConfig(_interp);
	//_gil->enabled = (config->enableGIL == AlifConfigGIL_::AlifConfig_GIL_Enable)
	//	? INT_MAX : 0; //* review //* delete
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



AlifThread* alifEval_saveThread() { // 628
	AlifThread* tstate = _alifThread_get();
	alifThread_detach(tstate);
	return tstate;
}

void alifEval_restoreThread(AlifThread* _thread) { // 636
#ifdef _WINDOWS
	AlifIntT err = GetLastError();
#endif

	ALIF_ENSURETHREADNOTNULL(_thread);
	alifThread_attach(_thread);

#ifdef _WINDOWS
	SetLastError(err);
#endif
}



static AlifIntT next_pendingCall(PendingCalls* _pending,
	AlifIntT (**_func)(void*), void** _arg, AlifIntT* _flags) { // 729
	AlifIntT i = _pending->first;
	if (_pending->npending == 0) {
		return -1;
	}
	*_func = _pending->calls[i].func;
	*_arg = _pending->calls[i].arg;
	*_flags = _pending->calls[i].flags;
	return i;
}

static void pop_pendingCall(PendingCalls* _pending,
	AlifIntT (**_func)(void*), void** _arg, AlifIntT* _flags) { // 747
	AlifIntT i = next_pendingCall(_pending, _func, _arg, _flags);
	if (i >= 0) {
		_pending->calls[i] = {0};
		_pending->first = (i + 1) % PENDINGCALLSARRAYSIZE;
		alifAtomic_addInt32(&_pending->npending, -1);
	}
}





static AlifIntT handle_signals(AlifThread* tstate) { // 813
	alifUnset_evalBreakerBit(tstate, ALIF_SIGNALS_PENDING_BIT);
	if (!alif_threadCanHandleSignals(tstate->interpreter)) {
		return 0;
	}
	//if (alifErr_checkSignalsThread(tstate) < 0) {
	//	alifSet_evalBreakerBit(tstate, ALIF_SIGNALS_PENDING_BIT);
	//	return -1;
	//}
	return 0;
}


static AlifIntT _make_pendingCalls(PendingCalls* _pending, int32_t* _pnPending) { // 829
	AlifIntT res = 0;
	int32_t npending = -1;

	int32_t maxloop = _pending->maxLoop;
	if (maxloop == 0) {
		maxloop = _pending->max;
	}

	for (int i = 0; i < maxloop; i++) {
		AlifPendingCallFunc func = nullptr;
		void* arg = nullptr;
		AlifIntT flags = 0;

		ALIFMUTEX_LOCK(&_pending->mutex);
		pop_pendingCall(_pending, &func, &arg, &flags);
		npending = _pending->npending;
		ALIFMUTEX_UNLOCK(&_pending->mutex);

		if (func == nullptr) {
			break;
		}

		res = func(arg);
		if ((flags & ALIF_PENDING_RAWFREE) and arg != nullptr) {
			alifMem_dataFree(arg);
		}
		if (res != 0) {
			res = -1;
			goto finally;
		}
	}

	finally:
	*_pnPending = npending;
	return res;
}


static void signal_pendingCalls(AlifThread* _thread, AlifInterpreter* _interp) { // 877
	alifSet_evalBreakerBitAll(_interp, ALIF_CALLS_TO_DO_BIT);
}


static void unsignal_pendingCalls(AlifThread* _thread, AlifInterpreter* _interp) { // 887
	alifUnset_evalBreakerBitAll(_interp, ALIF_CALLS_TO_DO_BIT);
}


static void clear_pendingHandlingThread(PendingCalls* pending) { // 897
	ALIFMUTEX_LOCK(&pending->mutex);
	pending->handlingThread = nullptr;
	ALIFMUTEX_UNLOCK(&pending->mutex);
}


static AlifIntT make_pendingCalls(AlifThread* _thread) { // 909
	AlifInterpreter* interp = _thread->interpreter;
	PendingCalls* pending = &interp->eval.pending;
	PendingCalls* pendingMain = &_alifDureRun_.eval.pendingMainThread;

	ALIFMUTEX_LOCK(&pending->mutex);
	if (pending->handlingThread != nullptr) {
		alifSet_evalBreakerBit(pending->handlingThread, ALIF_CALLS_TO_DO_BIT);
		alifUnset_evalBreakerBit(_thread, ALIF_CALLS_TO_DO_BIT);
		ALIFMUTEX_UNLOCK(&pending->mutex);
		return 0;
	}
	pending->handlingThread = _thread;
	ALIFMUTEX_UNLOCK(&pending->mutex);

	unsignal_pendingCalls(_thread, interp);

	int32_t npending;
	if (_make_pendingCalls(pending, &npending) != 0) {
		clear_pendingHandlingThread(pending);
		signal_pendingCalls(_thread, interp);
		return -1;
	}
	if (npending > 0) {
		signal_pendingCalls(_thread, interp);
	}

	if (alif_isMainThread() and alif_isMainInterpreter(interp)) {
		if (_make_pendingCalls(pendingMain, &npending) != 0) {
			clear_pendingHandlingThread(pending);
			signal_pendingCalls(_thread, interp);
			return -1;
		}
		if (npending > 0) {
			signal_pendingCalls(_thread, interp);
		}
	}

	clear_pendingHandlingThread(pending);
	return 0;
}


void alifSet_evalBreakerBitAll(AlifInterpreter* _interp, uintptr_t _bit) { // 969
	AlifDureRun* dureRun = &_alifDureRun_;

	HEAD_LOCK(dureRun);
	for (AlifThread* tstate = _interp->threads.head; tstate != nullptr; tstate = tstate->next) {
		alifSet_evalBreakerBit(tstate, _bit);
	}
	HEAD_UNLOCK(dureRun);
}

void alifUnset_evalBreakerBitAll(AlifInterpreter* _interp, uintptr_t _bit) { // 981
	AlifDureRun* dureRun = &_alifDureRun_;

	HEAD_LOCK(dureRun);
	for (AlifThread* thread = _interp->threads.head; thread != nullptr; thread = thread->next) {
		alifUnset_evalBreakerBit(thread, _bit);
	}
	HEAD_UNLOCK(dureRun);
}



AlifIntT alifEval_makePendingCalls(AlifThread* _thread) { // 1029
	AlifIntT res{};
	if (alif_isMainThread() and alif_isMainInterpreter(_thread->interpreter)) {
		res = handle_signals(_thread);
		if (res != 0) {
			return res;
		}
	}

	res = make_pendingCalls(_thread);
	if (res != 0) {
		return res;
	}

	return 0;
}




AlifIntT alif_makePendingCalls() { // 1054
	AlifThread* thread = _alifThread_get();

	if (!alif_isMainThread() or !alif_isMainInterpreter(thread->interpreter)) {
		return 0;
	}
	return alifEval_makePendingCalls(thread);
}
