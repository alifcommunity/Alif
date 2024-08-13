#pragma once


typedef void* AlifThreadLock;

enum AlifLockStatus {
        Alif_Lock_Failure = 0,
        Alif_Lock_Acquired = 1,
        Alif_Lock_Intr
} ;

typedef class AlifTSST AlifTSST;


#ifdef USE_PTHREADS
#   include <pthread.h>
#   define NATIVE_TSS_KEY_T     PThreadKeyT
#elif defined(NT_THREADS)
/* In Windows, native TSS key type is DWORD,
   but hardcode the unsigned long to avoid errors for include directive.
*/
#   define NATIVE_TSS_KEY_T     unsigned long
#else
#   error "Require native threads."
#endif

class AlifTSST {
public:
    AlifIntT isInitialized{};
    NATIVE_TSS_KEY_T key_{};
};

#undef NATIVE_TSS_KEY_T

#define ALIFTSS_NEEDS_INIT   {0}



