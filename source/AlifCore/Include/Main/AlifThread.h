#pragma once





AlifUSizeT alifThread_getThreadID(); // 21





typedef struct AlifTssT AlifTssT;

AlifIntT alifThreadTSS_isCreated(AlifTssT*); // 96
AlifIntT alifThreadTSS_create(AlifTssT*); // 97
void alifThreadTSS_delete(AlifTssT*); // 98


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
