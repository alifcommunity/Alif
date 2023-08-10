#pragma once

#include "alif.h"
#include "alifCore_condVar.h"

#ifdef POSIX_THREADS

#define ALIFMUTEX_INIT(mut)       pthread_mutex_init((mut), nullptr)
#define ALIFMUTEX_FINI(mut)       pthread_mutex_destroy(mut)
#define ALIFMUTEX_LOCK(mut)       pthread_mutex_lock(mut)
#define ALIFMUTEX_UNLOCK(mut)     pthread_mutex_unlock(mut)

#define ALIFCOND_INIT(cond)       thread_cond_init(cond)
#define ALIFCOND_FINI(cond)       pthread_cond_destroy(cond)
#define ALIFCOND_SIGNAL(cond)     pthread_cond_signal(cond)
#define ALIFCOND_BROADCAST(cond)  pthread_cond_broadcast(cond)
#define ALIFCOND_WAIT(cond, mut)  pthread_cond_wait((cond), (mut))


#elif defined(NT_THREADS)

#if ALIF_EMULATED_WIN_CV

ALIF_LOCAL_INLINE(int)alifMutex_init(AlifMutex_T* cs) {
	InitializeCriticalSection(cs);
	return 0;
}

ALIF_LOCAL_INLINE(int)alifMutex_fini(AlifMutex_T* cs) {
	DeleteCriticalSection(cs);
	return 0;
}

ALIF_LOCAL_INLINE(int)alifMutex_lock(AlifMutex_T* cs) {
	EnterCriticalSection(cs);
	return 0;
}

ALIF_LOCAL_INLINE(int)alifMutex_unlock(AlifMutex_T* cs) {
	LeaveCriticalSection(cs);
	return 0;
}

ALIF_LOCAL_INLINE(int)alifCond_init(AlifCond_T* cv) {

	cv->sem = CreateSemaphore(nullptr, 0, 100000, nullptr);
	if (cv->sem == nullptr)
		return -1;
	cv->waiting = 0;
	return 0;
}

ALIF_LOCAL_INLINE(int)alifCond_finit(AlifCond_T* cv) {

	return CloseHandle(cv->sem) ? 0 : -1;
}

ALIF_LOCAL_INLINE(int)alifCond_wait_ms(AlifCond_T* cv, AlifMutex_T* cs, DWORD ms) {

	DWORD wait;
	cv->waiting++;
	alifMutex_unlock(cs);


	wait = WaitForSingleObjectEx(cv->sem, ms, 0);
	alifMutex_lock(cs);
	if (wait != ((0x00000000L) + 0))
		--cv->waiting;

	if (wait == 0xFFFFFFFF)
		return -1;
	return wait != ((0x00000000L) + 0);

}

ALIF_LOCAL_INLINE(int)alifCond_wait(AlifCond_T* cv, AlifMutex_T* cs) {

	int result = alifCond_wait_ms(cv, cs, 0xFFFFFFFF);
	return result >= 0 ? 0 : result;
}


ALIF_LOCAL_INLINE(int)alifCond_timedWait(AlifCond_T* cv, AlifMutex_T* cs, long long us) {

	return alifCond_wait_ms(cv, cs, (DWORD)(us/ 1000));
	
}

ALIF_LOCAL_INLINE(int)alifCond_signal(AlifCond_T* cv) {

	if (cv->waiting > 0) {

		cv->waiting--;
		return ReleaseSemaphore(cv->sem, 1, nullptr) ? 0 : -1;

	}
	return 0;

}

ALIF_LOCAL_INLINE(int)alifCond_broadcast(AlifCond_T* cv) {

	int waiting = cv->waiting;

	if (waiting > 0) {
		cv->waiting = 0;
		ReleaseSemaphore(cv->sem, waiting, nullptr) ? 0 : -1;
	}
	return 0;
}

#else

ALIF_LOCAL_INLINE(int)alifMutex_init(AlifMutex_T* cs) {
	InitializeSRWLock(cs);
	return 0;
}

ALIF_LOCAL_INLINE(int)alifMutex_finit(AlifMutex_T* cs)
{
	return 0;
}

ALIF_LOCAL_INLINE(int)alifMutex_lock(AlifMutex_T* cs)
{
	AcquireSRWLockExclusive(cs);
	return 0;
}

ALIF_LOCAL_INLINE(int)alifMutex_unlock(AlifMutex_T* cs)
{
	ReleaseSRWLockExclusive(cs);
	return 0;
}


ALIF_LOCAL_INLINE(int)alifCond_init(AlifCond_T* cv)
{
	InitializeConditionVariable(cv);
	return 0;
}
ALIF_LOCAL_INLINE(int)alifCond_finit(AlifCond_T* cv)
{
	return 0;
}

ALIF_LOCAL_INLINE(int)alifCond_wait(AlifCond_T* cv, AlifMutex_T* cs)
{
	return SleepConditionVariableSRW(cv, cs, 0xFFFFFFFF, 0) ? 0 : -1;
}

/* This implementation makes no distinction about timeouts.  Signal
 * 2 to indicate that we don't know.
 */
ALIF_LOCAL_INLINE(int)alifCond_timedWait(AlifCond_T* cv, AlifMutex_T* cs, long long us)
{
	return SleepConditionVariableSRW(cv, cs, (DWORD)(us / 1000), 0) ? 2 : -1;
}

ALIF_LOCAL_INLINE(int)alifCond_signal(AlifCond_T* cv)
{
	WakeConditionVariable(cv);
	return 0;
}

ALIF_LOCAL_INLINE(int)alifCond_broadCast(AlifCond_T* cv)
{
	WakeAllConditionVariable(cv);
	return 0;
}

#endif

#endif
