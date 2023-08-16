#pragma once

#ifndef POSIX_THREADS
# ifdef HAVE_PTHREAD_H
#  include <pthread.h> 
# endif
# ifndef POSIX_THREADS
#  ifdef hpux
#   ifdef SC_THREADS
#    define POSIX_THREADS
#   endif
#  endif
# endif
#endif 

#if defined(POSIX_THREADS) || defined(HAVE_PTHREAD_STUBS)
# define USE_PTHREADS
#endif

#if defined(USE_PTHREADS) && defined(HAVE_PTHREAD_CONDATTR_SETCLOCK) && defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
# define CONDATTR_MONOTONIC
#endif


#if defined(HAVE_PTHREAD_STUBS)
// pthread_key
	class AlifStubTlsEntry {
	public:
		bool inUse;
		void* value;
	};
#endif

class AlifThreadRuntimeState {
public:
		int initialized;

#ifdef USE_PTHREADS
		// This matches when thread_pthread.h is used.
		class CondattrMonotonic{
		public:
			/* NULL when pthread_condattr_setclock(CLOCK_MONOTONIC) is not supported. */
			PthreadCondAttrT* ptr;
# ifdef CONDATTR_MONOTONIC
			/* The value to which condattr_monotonic is set. */
			PthreadCondAttrT val;
# endif
		};

#endif  // USE_PTHREADS

#if defined(HAVE_PTHREAD_STUBS)
		class Stubs{
		public:
			class AlifStubTlsEntry tlsEntries[PTHREAD_KEYS_MAX];
		} ;
#endif
};
