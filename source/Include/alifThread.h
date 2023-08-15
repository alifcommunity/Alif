#pragma once

typedef void* AlifThreadTypeLock;

// failure -> 0, success -> 1.
enum AlifLockStatus {

	alifLockFailure = 0,
	alifLockAcquired = 1,
	alifLockIntr
};

ALIFAPI_FUNC(unsigned long) alifThread_get_thread_ident(void);

#if (defined(__APPLE__) || defined(__linux__) || defined(_WIN32) \
     || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
     || defined(__DragonFly__) || defined(_AIX))
#define ALIF_HAVE_THREAD_NATIVE_ID
#endif

#define ALIF_TIMEOUT_T long long

typedef class AlifTSST _alifTSST;

ALIFAPI_FUNC(AlifThreadTypeLock) alifThread_allocateLock();
ALIFAPI_FUNC(void) alifThread_freeLock(AlifThreadTypeLock);
#define WAIT_LOCK       1
#define NOWAIT_LOCK     0

ALIFAPI_FUNC(int) alifThread_tss_isCreated(_alifTSST* key);
ALIFAPI_FUNC(int) alifThread_tss_create(_alifTSST* key);
ALIFAPI_FUNC(void) alifThread_tss_delete(_alifTSST* key);
ALIFAPI_FUNC(int) alifThread_tss_set(_alifTSST* key, void* value);
ALIFAPI_FUNC(void*) alifThread_tss_get(_alifTSST* key);

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


#define ALIFTHREAD_INVALID_THREAD_ID ((unsigned long)-1)

#ifdef HAVE_PTHREAD_H
#   include <pthread.h>
#   define NATIVE_TSS_KEY_T     PthreadKeyT
#elif defined(NT_THREADS)
#   define NATIVE_TSS_KEY_T     unsigned long
#elif defined(HAVE_PTHREAD_STUBS)
#   include "Pthread_Stubs.h"
#   define NATIVE_TSS_KEY_T     PthreadKeyT
#endif

struct AlifTSST {
	int isInitialized;
	NATIVE_TSS_KEY_T key;
};

#undef NATIVE_TSS_KEY_T

#define ALIF_TSS_NEEDS_INIT   {0}

