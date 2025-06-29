#pragma once


typedef void* AlifThreadTypeLock; // 4


enum AlifLockStatus_ { // 12
	Alif_Lock_Failure = 0,
	Alif_Lock_Acquired = 1,
	Alif_Lock_Intr
};

//void ALIF_NO_RETURN alifThread_exitThread(void); // 20
AlifUIntT alifThread_getThreadID(); // 21

// 23
#if (defined(__APPLE__) or defined(__linux__) or defined(_WIN32) \
     or defined(__FreeBSD__) or defined(__FreeBSD_kernel__) \
     or defined(__OpenBSD__) or defined(__NetBSD__) \
     or defined(__DragonFly__) or defined(_AIX))
#define ALIF_HAVE_THREAD_NATIVE_ID
unsigned long alifThread_getThreadNativeID();
#endif


AlifIntT alifThread_acquireLock(AlifThreadTypeLock, AlifIntT); // 33
#define WAIT_LOCK       1
#define NOWAIT_LOCK     0


// Forward Declaration
class AlifTssT;

AlifIntT alifThreadTSS_isCreated(AlifTssT*); // 96
AlifIntT alifThreadTSS_create(AlifTssT*); // 97
void alifThreadTSS_delete(AlifTssT*); // 98
AlifIntT alifThreadTSS_set(AlifTssT*, void*); // 99
void* alifThreadTSS_get(AlifTssT*); // 100

/* --------------------------------------------------------------------------------------------------------- */









#ifdef HAVE_PTHREAD_H
#   include <pthread.h>
#   define NATIVE_TSS_KEY_T		pthread_key_t
#elif defined(NT_THREADS)
/* In Windows, native TSS key type is DWORD,
   but hardcode the unsigned long to avoid errors for include directive.
*/
#   define NATIVE_TSS_KEY_T     unsigned long
#elif defined(HAVE_PTHREAD_STUBS)
#   include "PThreadStubs.h"
#   define NATIVE_TSS_KEY_T     pthread_key_t
#else
#   error "Require native threads."
#endif

class AlifTssT { // 35
public:
	AlifIntT isInitialized{};
	NATIVE_TSS_KEY_T key;
};
