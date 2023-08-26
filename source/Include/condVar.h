#pragma once







































#ifndef CONDVAR_IMPL_H
#define CONDVAR_IMPL_H

#include "alif.h"
#include "alifCore_condVar.h"

#ifdef POSIX_THREADS









#define alifMutex_init(mut)       pthread_mutexInit((mut), NULL)
#define alifMutex_fini(mut)       pthread_mutexDestroy(mut)
#define alifMutex_lock(mut)       pthread_mutexLock(mut)
#define ALIFMUTEX_UNLOCK(mut)     pthread_mutexUnlock(mut)

#define ALIFCOND_INIT(cond)       alifThread_condInit(cond)
#define ALIFCOND_FINI(cond)       pthread_condDestroy(cond)
#define ALIFCOND_SIGNAL(cond)     pthread_condSignal(cond)
#define ALIFCOND_BROADCAST(cond)  pthread_condBroadcast(cond)
#define ALIFCOND_WAIT(cond, mut)  pthread_condWait((cond), (mut))



ALIF_LOCAL_INLINE(int)alifCond_TimedWait(AlifCond_T* cond, AlifMutexT* mut, long long _us)
{
	struct timespec absTimeout;
	alifThread_cond_after(_us, &absTimeout);
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
















ALIF_LOCAL_INLINE(int)alifMutex_init(AlifMutexT* _cs)
{
	InitializeCriticalSection(_cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_fini(AlifMutexT* _cs)
{
	DeleteCriticalSection(_cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_lock(AlifMutexT* _cs)
{
	EnterCriticalSection(_cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_unlock(AlifMutexT* _cs)
{
	LeaveCriticalSection(_cs);
	return 0;
}



ALIF_LOCAL_INLINE(int)alifCond_init(AlifCondT* _cv)
{




	_cv->sem = CreateSemaphore(NULL, 0, 100000, NULL);
	if (_cv->sem == NULL)
		return -1;
	_cv->waiting = 0;
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_fini(AlifCondT* _cv)
{
	return CloseHandle(_cv->sem) ? 0 : -1;
}






ALIF_LOCAL_INLINE(int)alifCond_wait_ms(AlifCondT* _cv, AlifMutexT* _cs, DWORD ms)
{
	DWORD wait;
	_cv->waiting++;
	alifMutex_unlock(_cs);




	wait = WaitForSingleObjectEx(_cv->sem, ms, FALSE);
	alifMutex_lock(_cs);
	if (wait != WAIT_OBJECT_0)
		--_cv->waiting;











	if (wait == WAIT_FAILED)
		return -1;

	return wait != WAIT_OBJECT_0;
}


ALIF_LOCAL_INLINE(int)alifCond_wait(AlifCondT* _cv, AlifMutexT* _cs)
{
	int result = alifCond_wait_ms(_cv, _cs, INFINITE);
	return result >= 0 ? 0 : result;
}


ALIF_LOCAL_INLINE(int)alifCond_timedWait(AlifCondT* _cv, AlifMutexT* _cs, long long _us)
{
	return alifCond_wait_ms(_cv, _cs, (DWORD)(_us / 1000));
}

ALIF_LOCAL_INLINE(int)alifCond_signal(AlifCondT* _cv)
{




	if (_cv->waiting > 0) {




		_cv->waiting--;
		return ReleaseSemaphore(_cv->sem, 1, NULL) ? 0 : -1;
	}
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_broadcast(AlifCondT* _cv)
{
	int waiting = _cv->waiting;
	if (waiting > 0) {
		_cv->waiting = 0;
		return ReleaseSemaphore(_cv->sem, waiting, NULL) ? 0 : -1;
	}
	return 0;
}

#else 

ALIF_LOCAL_INLINE(int)alifMutex_init(AlifMutexT* _cs)
{
	InitializeSRWLock(_cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_fini(AlifMutexT* _cs)
{
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_lock(AlifMutexT* _cs)
{
	AcquireSRWLockExcl_usive(_cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifMutex_unlock(AlifMutexT* _cs)
{
	ReleaseSRWLockExcl_usive(_cs);
	return 0;
}



ALIF_LOCAL_INLINE(int)alifCond_init(AlifCondT* _cv)
{
	InitializeConditionVariable(_cv);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_fini(AlifCondT* _cv)
{
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_wait(AlifCondT* _cv, AlifMutexT* _cs)
{
	return SleepConditionVariableSRW(_cv, _cs, INFINITE, 0) ? 0 : -1;
}





ALIF_LOCAL_INLINE(int)alifCond_timedWait(AlifCondT* _cv, AlifMutexT* _cs, long long _us)
{
	return SleepConditionVariableSRW(_cv, _cs, (DWORD)(_us / 1000), 0) ? 2 : -1;
}


ALIF_LOCAL_INLINE(int)alifCond_signal(AlifCondT* _cv)
{
	WakeConditionVariable(_cv);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_broadcast(AlifCondT* _cv)
{
	WakeAllConditionVariable(_cv);
	return 0;
}



#endif 

#endif 

#endif
