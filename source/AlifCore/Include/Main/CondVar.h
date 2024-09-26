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
#define ALIFMUTEX_INIT(mut)       pthreadMutex_init((mut), nullptr)
#define ALIFMUTEX_FINI(mut)       pthreadMutex_destroy(mut)
#define ALIFMUTEX_LOCK(mut)       pthreadMutex_lock(mut)
#define ALIFMUTEX_UNLOCK(mut)     pthreadMutex_unlock(mut)

#define ALIFCOND_INIT(cond)       _PyThread_cond_init(cond)
#define ALIFCOND_FINI(cond)       pthread_cond_destroy(cond)
#define ALIFCOND_SIGNAL(cond)     pthread_cond_signal(cond)
#define ALIFCOND_BROADCAST(cond)  pthread_cond_broadcast(cond)
#define ALIFCOND_WAIT(cond, mut)  pthread_cond_wait((cond), (mut))

/* return 0 for success, 1 on timeout, -1 on error */
ALIF_LOCAL_INLINE(AlifIntT)
alifCond_TimedWait(AlifCondT* cond, AlifMutexT* mut, long long us) { // 69
	timespec abs_timeout;
	alifThread_condAfter(us, &abs_timeout);
	AlifIntT ret = pthread_condTimedWait(cond, mut, &abs_timeout);
	if (ret == ETIMEDOUT) {
		return 1;
	}
	if (ret) {
		return -1;
	}
	return 0;
}

#elif defined(NT_THREADS)

















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
