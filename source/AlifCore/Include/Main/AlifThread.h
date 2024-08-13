#pragma once





AlifSizeT alifThread_getThreadID(); // 21







AlifIntT alifThreadTSS_create(AlifTssT*); // 97


/* --------------------------------------------------------------------------------------------------------- */









#ifdef USE_PTHREADS
#   include <pthread.h>
#   define NATIVE_TSS_KEY_T		unsigned
#elif defined(NT_THREADS)
/* In Windows, native TSS key type is DWORD,
   but hardcode the unsigned long to avoid errors for include directive.
*/
#   define NATIVE_TSS_KEY_T     unsigned long
#else
#   error "Require native threads."
#endif

class AlifTssT { // 35
public:
	AlifIntT isInitialized{};
	NATIVE_TSS_KEY_T key;
};
