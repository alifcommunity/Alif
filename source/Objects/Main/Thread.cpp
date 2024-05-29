#include "alif.h"
#include "AlifCore_AlifThread.h"
#include "AlifCore_Memory.h"

#ifndef DONT_HAVE_STDIO_H
#  include <stdio.h>
#endif

#include <stdlib.h>


#ifdef _POSIX_THREADS

#  define ALIF_TIMEOUT_MAX_VALUE (LLONG_MAX / 1000)
#elif defined (NT_THREADS)

#  if 0xFFFFFFFELL < LLONG_MAX / 1000
#    define ALIF_TIMEOUT_MAX_VALUE (0xFFFFFFFELL * 1000)
#  else
#    define ALIF_TIMEOUT_MAX_VALUE LLONG_MAX
#  endif
#else
#  define ALIF_TIMEOUT_MAX_VALUE LLONG_MAX
#endif

const long long ALIF_TIMEOUT_MAX = ALIF_TIMEOUT_MAX_VALUE;

//static void alifThread__init_thread(void); 

//#define INITIALIZED _alifRuntime_.threads_.initialized_

void alifThread_init_thread(void)
{
    //if (INITIALIZED) {
        //return;
    //}
    //INITIALIZED = 1;
    //alifThread__init_thread();
}

#if defined(HAVE_PTHREAD_STUBS)
#   define ALIFTHREAD_NAME "pthread-stubs"
#   include "thread_pthread_stubs.h"
#elif defined(_USE_PTHREADS)  /* AKA _PTHREADS */
#   if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
#     define ALIFTHREAD_NAME "pthread-stubs"
#   else
#     define ALIFTHREAD_NAME "pthread"
#   endif
#   include "thread_pthread.h"
#elif defined(NT_THREADS)
#   define ALIFTHREAD_NAME "nt"
#   include "thread_nt.h"
#endif

int alifThread_parseTimeoutArg(AlifObject* _arg, int _blocking, AlifTimeT* _timeoutP)
{
    if (_arg == NULL || _arg == ALIF_NONE) {
        *_timeoutP = _blocking ? ALIFTHREAD_UNSET_TIMEOUT : 0;
        return 0;
    }
    if (!_blocking) {

        return -1;
    }

    AlifTimeT timeout_;
    if (alifSubTime_fromSecondsObject(&timeout_, _arg, AlifSubTime_Round_Timeout) < 0) {
        return -1;
    }
    if (timeout_ < 0) {
        return -1;
    }

    if (alifSubTime_asMicroseconds(timeout_,
        AlifSubTime_Round_Timeout) > ALIF_TIMEOUT_MAX) {
        return -1;
    }
    *_timeoutP = timeout_;
    return 0;
}

/* Thread Specific Storage (TSS) API

   Cross-platform components of TSS API implementation.
*/

AlifTSST* alifThread_tss_alloc()
{
    AlifTSST* newKey = (AlifTSST*)alifMem_dataAlloc(sizeof(AlifTSST));
    if (newKey == nullptr) {
        return nullptr;
    }
    newKey->isInitialized = 0;
    return newKey;
}

void alifThread_tss_free(AlifTSST* _key)
{
    if (_key != nullptr) {
        alifThread_tss_delete(_key);
        alifMem_dataFree((void*)_key);
    }
}

int alifThread_tss_is_created(AlifTSST* _key)
{
    return _key->isInitialized;
}
