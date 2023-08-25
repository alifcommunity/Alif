#pragma once
#include "alifCore_interp.h"  





#include <windows.h>
#include <limits.h>
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

/* options */
#ifndef ALIFUSE_CV_LOCKS
#define ALIFUSE_CV_LOCKS 1     
#endif





#if ALIFUSE_CV_LOCKS

#include "condVar.h"

class NRMutex{
public:
	AlifMutexT cs;
	AlifCondT cv;
	int locked;
};
typedef NRMutex* PNRMutex;






























static DWORD enterNonRecursiveMutex(PNRMutex mutex, DWORD milliseconds)
{
	DWORD result = WAIT_OBJECT_0;
	if (alifMutex_lock(&mutex->cs))
		return WAIT_FAILED;
	if (milliseconds == INFINITE) {
		while (mutex->locked) {
			if (alifCond_wait(&mutex->cv, &mutex->cs)) {
				result = WAIT_FAILED;
				break;
			}
		}
	}
	else if (milliseconds != 0) {

		alifTimeT nanoseconds = alifTime_fromNanoseconds((alifTimeT)milliseconds * 1000000);
		alifTimeT deadLine = alifTime_add(alifTime_getPerfCounter(), nanoseconds);
		while (mutex->locked) {
			alifTimeT microseconds = alifTime_asMicroseconds(nanoseconds,
				AlifTime_Round_TimeOut);
			if (alifCond_timedWait(&mutex->cv, &mutex->cs, microseconds) < 0) {
				result = WAIT_FAILED;
				break;
			}
			nanoseconds = deadLine - alifTime_getPerfCounter();
			if (nanoseconds <= 0) {
				break;
			}
		}
	}
	if (!mutex->locked) {
		mutex->locked = 1;
		result = WAIT_OBJECT_0;
	}
	else if (result == WAIT_OBJECT_0)
		result = WAIT_TIMEOUT;
	alifMutex_unlock(&mutex->cs); /* must ignore result here */
	return result;
}

static BOOL leaveNonRecursiveMutex(PNRMutex mutex)
{
	BOOL result;
	if (alifMutex_lock(&mutex->cs))
		return FALSE;
	mutex->locked = 0;

	result = !alifCond_signal(&mutex->cv);
	alifMutex_unlock(&mutex->cs);
	return result;
}


#else /* if ! ALIF_USE_CVLOCKS */




































































































































#endif







































const DWORD TIMEOUT_MS_MAX = 0xFFFFFFFE;







AlifLockStatus alifThread_acquireLockTimed(AlifThreadTypeLock aLock,
	ALIF_TIMEOUT_T microseconds, int intrFlag)
{





	AlifLockStatus success;
	ALIF_TIMEOUT_T milliseconds;

	if (microseconds >= 0) {
		milliseconds = microseconds / 1000;
		// Round milliseconds away from zero
		if (microseconds % 1000 > 0) {
			milliseconds++;
		}
		if (milliseconds > (ALIF_TIMEOUT_T)TIMEOUT_MS_MAX) {






			milliseconds = TIMEOUT_MS_MAX;
		}

	}
	else {
		milliseconds = INFINITE;
	}

	if (enterNonRecursiveMutex((PNRMutex)aLock,
		(DWORD)milliseconds) == WAIT_OBJECT_0) {
		success = Alif_Lock_Acquired;
	}
	else {
		success = Alif_Lock_Failure;
	}

	return success;
}

int alifThread_acquire_lock(AlifThreadTypeLock aLock, int waitFlag)
{
	return alifThread_acquireLockTimed(aLock, waitFlag ? -1 : 0, 0);
}

void alifThread_release_lock(AlifThreadTypeLock aLock)
{

	(void)leaveNonRecursiveMutex((PNRMutex)aLock);
}
