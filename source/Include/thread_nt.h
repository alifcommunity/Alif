#pragma once
#include "alifCore_interp.h"  





#include <windows.h>
#include <limits.h>
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif


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

static PNRMutex allocNonRecursiveMutex()
{
	PNRMutex m = (PNRMutex)alifMem_rawMalloc(sizeof(NRMutex));
	if (!m)
		return nullptr;
	if (alifCond_init(&m->cv))
		goto fail;
	if (alifMutex_init(&m->cs)) {
		alifCond_fini(&m->cv);
		goto fail;
	}
	m->locked = 0;
	return m;
fail:
	alifMem_rawFree(m);
	return nullptr;
}












static DWORD enterNonRecursiveMutex(PNRMutex _mutex, DWORD _milliseconds)
{
	DWORD result = WAIT_OBJECT_0;
	if (alifMutex_lock(&_mutex->cs))
		return WAIT_FAILED;
	if (_milliseconds == INFINITE) {
		while (_mutex->locked) {
			if (alifCond_wait(&_mutex->cv, &_mutex->cs)) {
				result = WAIT_FAILED;
				break;
			}
		}
	}
	else if (_milliseconds != 0) {

		alifTimeT nanoseconds = alifTime_fromNanoseconds((alifTimeT)_milliseconds * 1000000);
		alifTimeT deadLine = alifTime_add(alifTime_getPerfCounter(), nanoseconds);
		while (_mutex->locked) {
			alifTimeT microseconds = alifTime_asMicroseconds(nanoseconds,
				AlifTime_Round_TimeOut);
			if (alifCond_timedWait(&_mutex->cv, &_mutex->cs, microseconds) < 0) {
				result = WAIT_FAILED;
				break;
			}
			nanoseconds = deadLine - alifTime_getPerfCounter();
			if (nanoseconds <= 0) {
				break;
			}
		}
	}
	if (!_mutex->locked) {
		_mutex->locked = 1;
		result = WAIT_OBJECT_0;
	}
	else if (result == WAIT_OBJECT_0)
		result = WAIT_TIMEOUT;
	alifMutex_unlock(&_mutex->cs); 
	return result;
}

static BOOL leaveNonRecursiveMutex(PNRMutex _mutex)
{
	BOOL result;
	if (alifMutex_lock(&_mutex->cs))
		return FALSE;
	_mutex->locked = 0;

	result = !alifCond_signal(&_mutex->cv);
	alifMutex_unlock(&_mutex->cs);
	return result;
}


#else 




































































































































#endif














AlifThreadTypeLock alifThread_allocateLock()
{
	PNRMutex mutex;

	if (!INITIALIZED)
		alifThread_initThread();

	mutex = allocNonRecursiveMutex();

	AlifThreadTypeLock aLock = (AlifThreadTypeLock)mutex;


	return aLock;
}











const DWORD TIMEOUT_MS_MAX = 0xFFFFFFFE;







AlifLockStatus alifThread_acquireLockTimed(AlifThreadTypeLock _aLock,
	ALIF_TIMEOUT_T _microseconds, int _intrFlag)
{





	AlifLockStatus success;
	ALIF_TIMEOUT_T milliseconds;

	if (_microseconds >= 0) {
		milliseconds = _microseconds / 1000;
		// Round _milliseconds away from zero
		if (_microseconds % 1000 > 0) {
			milliseconds++;
		}
		if (milliseconds > (ALIF_TIMEOUT_T)TIMEOUT_MS_MAX) {






			milliseconds = TIMEOUT_MS_MAX;
		}

	}
	else {
		milliseconds = INFINITE;
	}

	if (enterNonRecursiveMutex((PNRMutex)_aLock,
		(DWORD)milliseconds) == WAIT_OBJECT_0) {
		success = Alif_Lock_Acquired;
	}
	else {
		success = Alif_Lock_Failure;
	}

	return success;
}

int alifThread_acquire_lock(AlifThreadTypeLock _aLock, int _waitFlag)
{
	return alifThread_acquireLockTimed(_aLock, _waitFlag ? -1 : 0, 0);
}

void alifThread_release_lock(AlifThreadTypeLock _aLock)
{

	(void)leaveNonRecursiveMutex((PNRMutex)_aLock);
}
