#pragma once


typedef void* AlifThreadTypeLock;







enum AlifLockStatus {
	Alif_Lock_Failure = 0,
	Alif_Lock_Acquired = 1,
	Alif_Lock_Intr
};

ALIFAPI_FUNC(void) alifThread_initThread();


ALIFAPI_FUNC(unsigned long) alifThread_getThreadIdent();

#if (defined(__APPLE__) || defined(__linux__) || defined(_WIN32) \
     || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
     || defined(__DragonFly__) || defined(_AIX))
#define ALIFHAVE_THREAD_NATIVEID
ALIFAPI_FUNC(unsigned long) alifThread_getThread_nativeID();
#endif

ALIFAPI_FUNC(AlifThreadTypeLock) alifThread_allocateLock();
ALIFAPI_FUNC(void) alifThread_freeLock(AlifThreadTypeLock);
ALIFAPI_FUNC(int) alifThread_acquireLock(AlifThreadTypeLock, int);
#define WAIT_LOCK       1
#define NOWAIT_LOCK     0










#define ALIF_TIMEOUT_T long long

#if defined(POSIX_THREADS)


#  define ALIF_TIMEOUT_MAX (LLONG_MAX / 1000)
#elif defined (NT_THREADS)



#  if 0xFFFFFFFELL * 1000 < LLONG_MAX
#    define ALIF_TIMEOUT_MAX (0xFFFFFFFELL * 1000)
#  else
#    define ALIF_TIMEOUT_MAX LLONG_MAX
#  endif
#else
#  define ALIF_TIMEOUT_MAX LLONG_MAX
#endif


















ALIFAPI_FUNC(void) alifThread_releaseLock(AlifThreadTypeLock);































typedef class AlifTssT AlifTssT; 





ALIFAPI_FUNC(int) alifThread_tssIsCreated(AlifTssT* );
ALIFAPI_FUNC(int) alifThread_tssCreate(AlifTssT* );
ALIFAPI_FUNC(void) alifThread_tssDelete(AlifTssT* );

ALIFAPI_FUNC(void*) alifThread_tssGet(AlifTssT* );


#ifndef ALIF_LIMITED_API
#  define ALIF_ALIFTHREAD_H
#  include "AlifCpp/alifThread.h"
#  undef ALIF_ALIFTHREAD_H
#endif
