#pragma once







































#ifndef CONDVAR_IMPL_H
#define CONDVAR_IMPL_H

#include "alif.h"
#include "alifCore_condVar.h"

#ifdef POSIX_THREADS
 /*
  * POSIX support
  */


int alifThread_cond_init(AlifCondT* cond);
void alifThread_cond_after(long long us, struct timespec* abs);

/* The following functions return 0 on success, nonzero on error */
#define alifMutex_init(mut)       pthread_mutex_init((mut), NULL)
#define alifMutex_fini(mut)       pthread_mutex_destroy(mut)
#define alifMutex_lock(mut)       pthread_mutex_lock(mut)
#define ALIFMUTEX_UNLOCK(mut)     pthread_mutex_unlock(mut)

#define ALIFCOND_INIT(cond)       alifThread_cond_init(cond)
#define ALIFCOND_FINI(cond)       pthread_cond_destroy(cond)
#define ALIFCOND_SIGNAL(cond)     pthread_cond_signal(cond)
#define ALIFCOND_BROADCAST(cond)  pthread_cond_broadcast(cond)
#define ALIFCOND_WAIT(cond, mut)  pthread_cond_wait((cond), (mut))


/* return 0 for success, 1 on timeout, -1 on error */
ALIF_LOCAL_INLINE(int)alifCond_TimedWait(AlifCond_T* cond, AlifMutexT* mut, long long us)
{
	struct timespec absTimeout;
	alifThread_cond_after(us, &absTimeout);
	int ret = pthread_cond_timedWait(cond, mut, &absTimeout);
	if (ret == ETIMEDOUT) {
		return 1;
	}
	if (ret) {
		return -1;
	}
	return 0;
}

#elif defined(NT_THREADS)







#if ALIF_EMULATED_WINCV
















ALIF_LOCAL_INLINE(int)alifMutex_init(AlifMutexT* cs)
{
	InitializeCriticalSection(cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_fini(AlifMutexT* cs)
{
	DeleteCriticalSection(cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_lock(AlifMutexT* cs)
{
	EnterCriticalSection(cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_unlock(AlifMutexT* cs)
{
	LeaveCriticalSection(cs);
	return 0;
}



ALIF_LOCAL_INLINE(int)alifCond_init(AlifCondT* cv)
{
	/* A semaphore with a "large" max value,  The positive value
	 * is only needed to catch those "lost wakeup" events and
	 * race conditions when a timed wait elapses.
	 */
	cv->sem = CreateSemaphore(NULL, 0, 100000, NULL);
	if (cv->sem == NULL)
		return -1;
	cv->waiting = 0;
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_fini(AlifCondT* cv)
{
	return CloseHandle(cv->sem) ? 0 : -1;
}

/* this implementation can detect a timeout.  Returns 1 on timeout,
 * 0 otherwise (and -1 on error)
 */

ALIF_LOCAL_INLINE(int)alifCond_wait_ms(AlifCondT* cv, AlifMutexT* cs, DWORD ms)
{
	DWORD wait;
	cv->waiting++;
	alifMutex_unlock(cs);
	/* "lost wakeup bug" would occur if the caller were interrupted here,
	 * but we are safe because we are using a semaphore which has an internal
	 * count.
	 */
	wait = WaitForSingleObjectEx(cv->sem, ms, FALSE);
	alifMutex_lock(cs);
	if (wait != WAIT_OBJECT_0)
		--cv->waiting;
	/* Here we have a benign race condition with alifCond_signal.
	 * When failure occurs or timeout, it is possible that
	 * alifCond_signal also decrements this value
	 * and signals releases the mutex.  This is benign because it
	 * just means an extra spurious wakeup for a waiting thread.
	 * ('waiting' corresponds to the semaphore's "negative" count and
	 * we may end up with e.g. (waiting == -1 && sem.count == 1).  When
	 * a new thread comes along, it will pass right through, having
	 * adjusted it to (waiting == 0 && sem.count == 0).
	 */

	if (wait == WAIT_FAILED)
		return -1;
	/* return 0 on success, 1 on timeout */
	return wait != WAIT_OBJECT_0;
}


ALIF_LOCAL_INLINE(int)alifCond_wait(AlifCondT* cv, AlifMutexT* cs)
{
	int result = alifCond_wait_ms(cv, cs, INFINITE);
	return result >= 0 ? 0 : result;
}


ALIF_LOCAL_INLINE(int)alifCond_timedWait(AlifCondT* cv, AlifMutexT* cs, long long us)
{
	return alifCond_wait_ms(cv, cs, (DWORD)(us / 1000));
}

ALIF_LOCAL_INLINE(int)alifCond_signal(AlifCondT* cv)
{
	/* this test allows alifCond_signal to be a no-op unless required
	 * to wake someone up, thus preventing an unbounded increase of
	 * the semaphore's internal counter.
	 */
	if (cv->waiting > 0) {
		/* notifying thread decreases the cv->waiting count so that
		 * a delay between notify and actual wakeup of the target thread
		 * doesn't cause a number of extra ReleaseSemaphore calls.
		 */
		cv->waiting--;
		return ReleaseSemaphore(cv->sem, 1, NULL) ? 0 : -1;
	}
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_broadcast(AlifCondT* cv)
{
	int waiting = cv->waiting;
	if (waiting > 0) {
		cv->waiting = 0;
		return ReleaseSemaphore(cv->sem, waiting, NULL) ? 0 : -1;
	}
	return 0;
}

#else /* !ALIF_EMULATED_WINCV */

ALIF_LOCAL_INLINE(int)alifMutex_init(AlifMutexT* cs)
{
	InitializeSRWLock(cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_fini(AlifMutexT* cs)
{
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_lock(AlifMutexT* cs)
{
	AcquireSRWLockExclusive(cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_unlock(AlifMutexT* cs)
{
	ReleaseSRWLockExclusive(cs);
	return 0;
}



ALIF_LOCAL_INLINE(int)alifCond_init(AlifCondT* cv)
{
	InitializeConditionVariable(cv);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_fini(AlifCondT* cv)
{
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_wait(AlifCondT* cv, AlifMutexT* cs)
{
	return SleepConditionVariableSRW(cv, cs, INFINITE, 0) ? 0 : -1;
}


/* This implementation makes no distinction about timeouts.  Signal
 * 2 to indicate that we don't know.
 */
ALIF_LOCAL_INLINE(int)alifCond_timedWait(AlifCondT* cv, AlifMutexT* cs, long long us)
{
	return SleepConditionVariableSRW(cv, cs, (DWORD)(us / 1000), 0) ? 2 : -1;
}


ALIF_LOCAL_INLINE(int)alifCond_signal(AlifCondT* cv)
{
	WakeConditionVariable(cv);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_broadcast(AlifCondT* cv)
{
	WakeAllConditionVariable(cv);
	return 0;
}



#endif /* ALIF_EMULATED_WINCV */

#endif /* POSIX_THREADS, NT_THREADS */

#endif /* CONDVAR_IMPL_H */
