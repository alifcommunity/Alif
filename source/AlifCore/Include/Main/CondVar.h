#pragma once


#include "alif.h"
#include "AlifCore_Thread.h"


#ifdef _POSIX_THREADS
/*
 * POSIX support
 */

AlifIntT alifThread_condInit(AlifCondT*);
void alifThread_condAfter(long long, timespec*);

/* The following functions return 0 on success, nonzero on error */
#define alifMutex_INIT(mut)       pthread_mutex_init((mut), nullptr)
#define alifMutex_FINI(mut)       pthread_mutex_destroy(mut)
#define alifMutex_LOCK(mut)       pthread_mutex_lock(mut)
#define alifMutex_UNLOCK(mut)     pthread_mutex_unlock(mut)

#define alifCond_INIT(cond)       alifThread_condInit(cond)
#define alifCond_FINI(cond)       pthread_cond_destroy(cond)
#define alifCond_SIGNAL(cond)     pthread_cond_signal(cond)
#define alifCond_BROADCAST(cond)  pthread_cond_broadcast(cond)
#define alifCond_WAIT(cond, mut)  pthread_cond_wait((cond), (mut))

/* return 0 for success, 1 on timeout, -1 on error */
ALIF_LOCAL_INLINE(AlifIntT)
alifCond_TimedWait(AlifCondT* cond, AlifMutexT* mut, long long us) { // 69
	timespec abs_timeout;
	alifThread_condAfter(us, &abs_timeout);
	AlifIntT ret = pthread_cond_timedwait(cond, mut, &abs_timeout);
	if (ret == ETIMEDOUT) {
		return 1;
	}
	if (ret) {
		return -1;
	}
	return 0;
}


#elif defined(NT_THREADS) // 84

#if ALIF_EMULATED_WIN_CV

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_INIT(AlifmutexT* cs) { // 109
	InitializeCriticalSection(cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_FINI(AlifMutexT* cs) { // 116
	DeleteCriticalSection(cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_LOCK(AlifMutexT* cs) { // 123
	EnterCriticalSection(cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_UNLOCK(AlifMutexT* cs) { // 130
	LeaveCriticalSection(cs);
	return 0;
}


ALIF_LOCAL_INLINE(AlifIntT) alifCond_INIT(AlifCondT* cv) { // 138
	cv->sem = CreateSemaphore(nullptr, 0, 100000, nullptr);
	if (cv->sem == nullptr)
		return -1;
	cv->waiting = 0;
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_FINI(AlifCondT* cv) { // 152
	return CloseHandle(cv->sem) ? 0 : -1;
}


ALIF_LOCAL_INLINE(AlifIntT) _alifCond_WAIT_MS(AlifCondT* cv, AlifCondT* cs, DWORD ms) { // 161
	DWORD wait{};
	cv->waiting++;
	alifMutex_UNLOCK(cs);
	wait = WaitForSingleObjectEx(cv->sem, ms, FALSE);
	alifMutex_LOCK(cs);
	if (wait != WAIT_OBJECT_0)
		--cv->waiting;

	if (wait == WAIT_FAILED)
		return -1;
	/* return 0 on success, 1 on timeout */
	return wait != WAIT_OBJECT_0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_WAIT(AlifCondT* cv, AlifMutexT* cs) { // 192
	AlifIntT result = _alifCond_WAIT_MS(cv, cs, INFINITE);
	return result >= 0 ? 0 : result;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_TIMEDWAIT(AlifCondT* cv, AlifCondT* cs, long long us) { // 199
	return _alifCond_WAIT_MS(cv, cs, (DWORD)(us / 1000));
}


ALIF_LOCAL_INLINE(AlifIntT) alifCond_SIGNAL(AlifCondT* cv) { // 205
	if (cv->waiting > 0) {
		cv->waiting--;
		return ReleaseSemaphore(cv->sem, 1, nullptr) ? 0 : -1;
	}
	return 0;
}



#else

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_INIT(AlifMutexT* _cs) { // 236
	InitializeSRWLock(_cs);
	return 0;
}


ALIF_LOCAL_INLINE(AlifIntT) alifMutex_FINI(AlifMutexT* _cs) { // 243
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_LOCK(AlifMutexT* _cs) { // 249
	AcquireSRWLockExclusive(_cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifMutex_UNLOCK(AlifMutexT* _cs) { // 256
	ReleaseSRWLockExclusive(_cs);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_INIT(AlifCondT* _cv) { // 263
	InitializeConditionVariable(_cv);
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_FINI(AlifCondT* _cv) { // 276
	return 0;
}


ALIF_LOCAL_INLINE(AlifIntT) alifCond_WAIT(AlifCondT* _cv, AlifMutexT* _cs) { // 276
	return SleepConditionVariableSRW(_cv, _cs, INFINITE, 0) ? 0 : -1;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_TIMEDWAIT(AlifCondT* _cv,
	AlifMutexT* _cs, long long _us) { // 283
	BOOL success = SleepConditionVariableSRW(_cv, _cs, (DWORD)(_us / 1000), 0);
	if (!success) {
		if (GetLastError() == ERROR_TIMEOUT) {
			return 1;
		}
		return -1;
	}
	return 0;
}

ALIF_LOCAL_INLINE(AlifIntT) alifCond_SIGNAL(AlifCondT* _cv) { // 296
	WakeConditionVariable(_cv);
	return 0;
}



#endif

#endif
