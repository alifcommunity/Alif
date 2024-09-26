#pragma once


#include "AlifCore_Thread.h"

















AlifIntT alifThread_condInit(AlifCondT* _cond) { // 147
	return pthread_condInit(_cond, condattr_monotonic);
}


void alifThread_condAfter(long long us, struct timespec* abs) { // 154
	AlifTimeT timeout = alifTime_fromMicrosecondsClamp(us);
	AlifTimeT t;
#ifdef CONDATTR_MONOTONIC
	if (condattr_monotonic) {
		(void)alifTime_monotonicRaw(&t);
	}
	else
#endif
	{
		(void)alifTime_timeRaw(&t);
	}
	t = alifTime_add(t, timeout);
	alifTime_asTimeSpecClamp(t, abs);
}




AlifUIntT alifThread_getThreadID() { // 365
	volatile pthread_t threadID{};

	if (!INITIALIZED) INITIALIZED = 1;

	threadID = pthread_self();
	return (AlifUSizeT)threadID;
}


#ifdef ALIF_HAVE_THREAD_NATIVE_ID
unsigned long alifThread_getThreadNativeID() { // 372
	if (!INITIALIZED)
		//alifThread_initThread();
#ifdef __APPLE__
	uint64_t nativeID;
	(void)pthread_threadid_np(nullptr, &nativeID);
#elif defined(__linux__)
	pid_t nativeID;
	nativeID = syscall(SYS_gettid);
#elif defined(__FreeBSD__)
	int nativeID;
	nativeID = pthread_getthreadid_np();
#elif defined(__FreeBSD_kernel__)
	long nativeID;
	syscall(SYS_thr_self, &nativeID);
#elif defined(__OpenBSD__)
	pid_t nativeID;
	nativeID = getthrid();
#elif defined(_AIX)
	tid_t nativeID;
	nativeID = thread_self();
#elif defined(__NetBSD__)
	lwpid_t nativeID;
	nativeID = _lwp_self();
#elif defined(__DragonFly__)
	lwpid_t nativeID;
	nativeID = lwp_gettid();
#endif
	return (unsigned long)nativeID;
}
#endif



void ALIF_NO_RETURN alifThread_exitThread(void) { // 406
	if (!INITIALIZED)
		exit(0);
#if defined(__wasi__)
	abort();
#else
	pthread_exit(0);
#endif
}




AlifIntT alifThread_acquireLock(AlifThreadTypeLock lock, AlifIntT waitflag) { // 799
	//return alifThread_acquireLockTimed(lock, waitflag ? -1 : 0, /*intr_flag=*/0);
	return 0; //
}




AlifIntT alifThreadTSS_create(AlifTssT* _key) { // 935
	/* If the key has been created, function is silently skipped. */
	if (_key->isInitialized) {
		return 0;
	}

	AlifIntT fail = pthread_key_create(&(_key->key), nullptr);
	if (fail) {
		return -1;
	}
	_key->isInitialized = 1;
	return 0;
}





void alifThreadTSS_delete(AlifTssT* _key) { // 952
	/* If the key has not been created, function is silently skipped. */
	if (!_key->isInitialized) {
		return;
	}

	pthread_key_delete(_key->key);
	/* pthread has not provided the defined invalid value for the key. */
	_key->isInitialized = 0;
}


AlifIntT alifThreadTSS_set(AlifTSST* _key, void* _value) { // 966
	AlifIntT fail = pthread_setspecific(_key->key, _value);
	return fail ? -1 : 0;
}

void* alifThreadTSS_get(AlifTssT* _key) { // 974
	return pthread_getspecific(_key->key);
}
