#pragma once


#include "AlifCore_Thread.h"
#include "AlifCore_Time.h"


#include <unistd.h>


 // 20
#if defined(__linux__)
#   include <sys/syscall.h>     /* syscall(SYS_gettid) */
#elif defined(__FreeBSD__)
#   include <pthread_np.h>      /* pthread_getthreadid_np() */
#elif defined(__FreeBSD_kernel__)
#   include <sys/syscall.h>     /* syscall(SYS_thr_self) */
#elif defined(_AIX)
#   include <sys/thread.h>      /* thread_self() */
#elif defined(__NetBSD__)
#   include <lwp.h>             /* _lwp_self() */
#elif defined(__DragonFly__)
#   include <sys/lwp.h>         /* lwp_gettid() */
#endif









#define _CONDATTR_MONOTONIC _alifDureRun_.threads.condAttrMonotonic.ptr // 130


AlifIntT alifThread_condInit(AlifCondT* _cond) { // 147
	return pthread_cond_init(_cond, _CONDATTR_MONOTONIC);
}


void alifThread_condAfter(long long us, struct timespec* abs) { // 154
	AlifTimeT timeout = _alifTime_fromMicrosecondsClamp(us);
	AlifTimeT t{};
#ifdef CONDATTR_MONOTONIC
	if (_CONDATTR_MONOTONIC) {
		(void)alifTime_monotonicRaw(&t);
	}
	else
#endif
	{
		(void)alifTime_timeRaw(&t);
	}
	t = _alifTime_add(t, timeout);
	_alifTime_asTimeSpecClamp(t, abs);
}


class pthread_lock { // 189
public:
	char locked; /* 0=unlocked, 1=locked */
	/* a <cond, mutex> pair to handle an acquire of a locked lock */
	pthread_cond_t   lock_released{};
	pthread_mutex_t  mut{};
};

#define CHECK_STATUS(name)  if (status != 0) { perror(name); error = 1; }
#define CHECK_STATUS_PTHREAD(name)  if (status != 0) { fprintf(stderr, \
    "%s: %s\n", name, strerror(status)); error = 1; }



AlifUIntT alifThread_getThreadID() { // 365
	volatile pthread_t threadID{};

	if (!INITIALIZED) INITIALIZED = 1;

	threadID = pthread_self();
	return (AlifUSizeT)threadID;
}


#ifdef ALIF_HAVE_THREAD_NATIVE_ID
unsigned long alifThread_getThreadNativeID() { // 372
	if (!INITIALIZED)
	{/*alifThread_initThread(); */}
#ifdef __APPLE__
	uint64_t nativeID;
	(void)pthread_threadid_np(nullptr, &nativeID);
#elif defined(__linux__)
	pid_t nativeID{};
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





void ALIF_NO_RETURN alifThread_hangThread(void) { // 421
	while (1) {
#if defined(__wasi__)
		sleep(9999999);  // WASI doesn't have pause() ?!
#else
		pause();
#endif
	}
}



#ifdef USE_SEMAPHORES // 433


void alifThread_freeLock(AlifThreadTypeLock lock) { // 463
	SemT* thelock = (SemT*)lock;
	AlifIntT status{}, error = 0;

	(void)error; /* silence unused-but-set-variable warning */

	if (!thelock)
		return;

	status = sem_destroy(thelock);
	CHECK_STATUS("sem_destroy");

	alifMem_dataFree((void*)thelock);
}


#else /* USE_SEMAPHORES */ // 622

AlifThreadTypeLock alifThread_allocateLock(void) { // 627
	pthread_lock* lock{};
	AlifIntT status{}, error = 0;

	//if (!INITIALIZED)
	//	alifThread_initThread();

	lock = (pthread_lock*)alifMem_dataAlloc(sizeof(pthread_lock));
	if (lock) {
		lock->locked = 0;

		status = pthread_mutex_init(&lock->mut, NULL);
		CHECK_STATUS_PTHREAD("pthread_mutex_init");

		//_ALIF_ANNOTATE_PURE_HAPPENS_BEFORE_MUTEX(&lock->mut);

		status = alifThread_condInit(&lock->lock_released);
		CHECK_STATUS_PTHREAD("pthread_cond_init");

		if (error) {
			alifMem_dataFree((void*)lock);
			lock = 0;
		}
	}

	return (AlifThreadTypeLock)lock;
}


void alifThread_freeLock(AlifThreadTypeLock lock) { // 661
	pthread_lock* thelock = (pthread_lock*)lock;
	AlifIntT status{}, error = 0;

	(void)error; /* silence unused-but-set-variable warning */

	status = pthread_cond_destroy(&thelock->lock_released);
	CHECK_STATUS_PTHREAD("pthread_cond_destroy");

	status = pthread_mutex_destroy(&thelock->mut);
	CHECK_STATUS_PTHREAD("pthread_mutex_destroy");

	alifMem_dataFree((void*)thelock);
}




#endif /* USE_SEMAPHORES */ // 788




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


AlifIntT alifThreadTSS_set(AlifTssT* _key, void* _value) { // 966
	AlifIntT fail = pthread_setspecific(_key->key, _value);
	return fail ? -1 : 0;
}

void* alifThreadTSS_get(AlifTssT* _key) { // 974
	return pthread_getspecific(_key->key);
}
