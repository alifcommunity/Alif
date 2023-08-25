#pragma once






#ifndef POSIX_THREADS



# ifdef HAVE_PTHREAD_H
#  include <pthread.h> /* POSIX_THREADS */
# endif
#endif

#ifdef POSIX_THREADS
   /*
	* POSIX support
	*/
#define ALIF_HAVE_CONDVAR

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#define ALIFMUTEXT pthread_mutex_t
#define ALIFCONDT pthread_cond_t

#elif defined(NT_THREADS)






#define ALIF_HAVE_CONDVAR


#define WIN32_LEAN_AND_MEAN
#include <windows.h>





#ifndef ALIF_EMULATED_WINCV
#define ALIF_EMULATED_WINCV 1  
#endif


#if !defined NTDDI_VISTA || NTDDI_VERSION < NTDDI_VISTA
#undef ALIF_EMULATED_WINCV
#define ALIF_EMULATED_WINCV 1
#endif

#if ALIF_EMULATED_WINCV

typedef CRITICAL_SECTION AlifMutexT;


















class AlifCondT{
public:
	HANDLE sem;
	int waiting;
};

#else /* !ALIF_EMULATED_WINCV */





typedef SRWLOCK AlifMutexT;

typedef CONDITION_VARIABLE  AlifCondT;

#endif 

#endif 

