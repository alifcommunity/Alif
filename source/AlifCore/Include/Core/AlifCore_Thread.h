#pragma once


#include "AlifCore_BiaseRefCount.h"
#include "AlifCore_Mimalloc.h"
#include "AlifCore_QSBR.h"



 // 15
#if (defined(HAVE_UNISTD_H) and !defined(_POSIX_THREADS) \
                            and !defined(_POSIX_SEMAPHORES))
#  include <unistd.h>             // _POSIX_THREADS, _POSIX_SEMAPHORES
#endif
#if (defined(HAVE_PTHREAD_H) and !defined(_POSIX_THREADS) \
                             and !defined(_POSIX_SEMAPHORES))
#  include <pthread.h>            // _POSIX_THREADS, _POSIX_SEMAPHORES
#endif
#if !defined(_POSIX_THREADS) and defined(__hpux) and defined(_SC_THREADS)
#  define _POSIX_THREADS
#endif



#if defined(HAVE_PTHREAD_STUBS) // 48
#include "PThreadStubs.h"  // PTHREAD_KEYS_MAX
#include <stdbool.h>                // bool

class AlifStubTLSEntry {
public:
	bool inUse{};
	void* value{};
};
#endif



class AlifThreadDureRunState { // 59
public:
	AlifIntT initialized{};

#ifdef USE_PTHREADS
	// This matches when ThreadPThread.h is used.
	class {
	public:
		/* nullptr when pthread_condattr_setclock(CLOCK_MONOTONIC) is not supported. */
		pthread_condattr_t* ptr;
# ifdef CONDATTR_MONOTONIC
		/* The value to which condattr_monotonic is set. */
		pthread_condattr_t val;
# endif
	} condAttrMonotonic;

#endif  // USE_PTHREADS

#if defined(HAVE_PTHREAD_STUBS)
	class {
	public:
		class AlifStubTLSEntry tlsEntries[PTHREAD_KEYS_MAX];
	} stubs;
#endif

	// Linked list of ThreadHandles
	LListNode handles{};
};


// 85
#define ALIFTHREAD_DURERUN_INIT(_threads) \
    { \
        .handles = LLIST_INIT(_threads.handles), \
    }




void ALIF_NO_RETURN alifThread_hangThread(void); // 167
