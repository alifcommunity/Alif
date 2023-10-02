#pragma once










#ifndef POSIX_THREADS
/* This means pthreads are not implemented in libc headers, hence the macro
   not present in unistd.h. But they still can be implemented as an external
   library (e.g. gnu pth in pthread emulation) */
# ifdef HAVE_PTHREAD_H
#  include <pthread.h> /* POSIX_THREADS */
# endif
# ifndef POSIX_THREADS








#  ifdef __hpux
#   ifdef _SC_THREADS
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

class AlifTtubTlsEntry {
public:
	bool inUse;
	void* value;
};
#endif
class AlifThreadRuntimeState {
public:
	int initialized;

#ifdef USE_PTHREADS

	class CondAttrMonotonic{
	public:
		pthread_condattr_t* ptr;
# ifdef CONDATTR_MONOTONIC

		pthread_condattr_t val;
# endif
	} ;

#endif  
#if defined(HAVE_PTHREAD_STUBS)
	class {
	public:
		class AlifThreadRuntimeState tls_entries[PTHREAD_KEYS_MAX];
	} stubs;
#endif
};