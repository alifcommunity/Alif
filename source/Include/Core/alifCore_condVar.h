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
   /*
	* Windows (XP, 2003 server and later, as well as (hopefully) CE) support
	*
	* Emulated condition variables ones that work with XP and later, plus
	* example native support on VISTA and onwards.
	*/
#define ALIF_HAVE_CONDVAR

	/* include windows if it hasn't been done before */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* options */
/* non-emulated condition variables are provided for those that want
 * to target Windows Vista.  Modify this macro to enable them.
 */
#ifndef ALIF_EMULATED_WINCV
#define ALIF_EMULATED_WINCV 1  /* use emulated condition variables */
#endif

 /* fall back to emulation if not targeting Vista */
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

#endif /* ALIF_EMULATED_WINCV */

#endif /* POSIX_THREADS, NT_THREADS */

