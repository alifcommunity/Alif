#pragma once

#include "alifMemory.h"
#include "alifCore_time.h"
#include "alifThread.h"

#include <windows.h>
#include <limits.h>
#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

/* options */
#ifndef ALIF_USE_CV_LOCKS
#define ALIF_USE_CV_LOCKS 1     /* use locks based on cond vars */
#endif

/* Now, define a non-recursive mutex using either condition variables
 * and critical sections (fast) or using operating system mutexes
 * (slow)
 */

#if ALIF_USE_CV_LOCKS

#include "condVar.h"
#include <thread.cpp>

class NRMutex
{
public:
	AlifMutexT cs;
	AlifCondT cv;
	int locked;
} ;
typedef NRMutex* PNRMutex;

PNRMutex alloc_nonRecursiveMutex() {

	PNRMutex m = (PNRMutex)raw_malloc(sizeof(NRMutex));
	if (!m)
		return nullptr;
	if (alifCond_init(&m->cv))
		goto fail;
	if (alifMutex_init(&m->cs)) {
		alifCond_finit(&m->cv);
		goto fail;
	}
	m->locked = 0;
	return m;
fail:
	raw_free(m);
	return nullptr;
}

VOID freeNonRecursiveMutex(PNRMutex mutex) {

	if (mutex) {

		alifCond_finit(&mutex->cv);
		alifMutex_fini(&mutex->cs);
		raw_free(mutex);
	}

}

const DWORD TIMEOUT_MS_MAX = 0xFFFFFFFE;

DWORD enterNonRecursiveMutex(PNRMutex mutex, DWORD millisecodes) {

	DWORD result = WAIT_OBJECT_0;

	if (alifMutex_lock(&mutex->cs)) {
		return WAIT_FAILED;
	}
	if (millisecodes == INFINITE) {
		while (mutex->locked)
		{
			if (alifCond_wait(&mutex->cv, &mutex->cs)) {
				result = WAIT_FAILED;
				break;
			}
		}
	}
	else if (millisecodes != 0) {
		int64_t nanoseconds = (int64_t)millisecodes * 1000000; // هنا تم ارجاع قيمة مباشرة 
		int64_t deadline = alifTime_add(alifTime_getPerfCounter(), nanoseconds);
		while (mutex->locked)
		{
			int64_t microseconds = alifTime_asMicroseconds(nanoseconds, alifTimeRoundTimeout);

			if (alifCond_timedWait(&mutex->cv, &mutex->cs, microseconds) < 0) {
				result = WAIT_FAILED;
				break;
			}
			nanoseconds = deadline - alifTime_getPerfCounter();
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
	/* else, it is WAIT_FAILED */
	alifMutex_unlock(&mutex->cs); /* must ignore result here */
	return result;
}

void alifThread__init_thread()
{
	// Initialization of the C package should not be needed.
}

unsigned long alifThread_get_thread_ident(void)
{
	if (!INITIALIZED)
		alifThread_init_thread();

	return GetCurrentThreadId();
}

AlifThreadTypeLock alifThread_allocateLock() {

	PNRMutex mutex;

	if (!INITIALIZED) {
		alifThread_initThread();
	}

	mutex = alloc_nonRecursiveMutex();

	AlifThreadTypeLock alock = (AlifThreadTypeLock)mutex;
	//assert(aLock);

	return alock;

}

void alifThread_freeLock(void* alock) {

	freeNonRecursiveMutex((PNRMutex)alock);

}

AlifLockStatus alifThread_acquire_lock_timed(void* alock, ALIF_TIMEOUT_T microseconds, int intrFloag) {

	AlifLockStatus success;
	long long milliseconds;

	if (microseconds >= 0) {
		milliseconds = microseconds / 1000;

		if (microseconds % 100 > 0) {
			milliseconds++;
		}
		if (milliseconds > (ALIF_TIMEOUT_T)TIMEOUT_MS_MAX) {
			milliseconds = TIMEOUT_MS_MAX;

		}

	}
	else {
		milliseconds = INFINITE; // هذا يعني ان الوقت سيستمر الى ما لا نهاية
	}
	if (enterNonRecursiveMutex((PNRMutex)alock,
		(DWORD)milliseconds) == WAIT_OBJECT_0) {
		success = alifLockAcquired;
	}
	else {
		success = alifLockFailure;
	}

	return success;
}

BOOL leaveNonRecursiveMutex(PNRMutex mutex) {

	BOOL result;
	if (alifMutex_lock(&mutex->cs)) {
		return FALSE;
	}
	mutex->locked = 0;
	result = !alifCond_signal(&mutex->cv);
	alifMutex_unlock(&mutex->cs);
	return result;
}

int alifThread_acquire_lock(void* alock, int waitflag) {
	return alifThread_acquire_lock_timed(alock, waitflag ? -1 : 0, 0);
}

void alifThread_release_lock(void* alock) {

	(void)leaveNonRecursiveMutex((PNRMutex) alock);

}

#endif 

int alifThread_tss_create(_alifTSST* key)
{
	if (key->isInitialized) {
		return 0;
	}

	DWORD result = TlsAlloc();
	if (result == TLS_OUT_OF_INDEXES) {
		return -1;
	}
	/* In Windows, platform-specific key type is DWORD. */
	key->key = result;
	key->isInitialized = 1;
	return 0;
}

void alifThread_tss_delete(_alifTSST* key)
{
	if (!key->isInitialized) {
		return;
	}

	TlsFree(key->key);
	key->key = TLS_OUT_OF_INDEXES;
	key->isInitialized = 0;
}

int alifThread_tss_set(_alifTSST* key, void* value)
{
	BOOL ok = TlsSetValue(key->key, value);
	return ok ? 0 : -1;
}

void* alifThread_tss_get(_alifTSST* key)
{
	DWORD error = GetLastError();
	void* result = TlsGetValue(key->key);
	SetLastError(error);
	return result;
}
