#pragma once

#ifndef POSIX_THREADS
# ifdef HAVE_PTHREAD_H
#  include <pthread.h>  
# endif
#endif

#ifdef POSIX_THREADS

#define ALIF_HAVE_CONDVAR

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#define ALIFMUTEX_T PTHREAD_MUTEX_T
#define ALIFCOND_T PTHREAD_COND_T

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
#ifndef ALIF_EMULATED_WIN_CV
#define ALIF_EMULATED_WIN_CV 1  /* use emulated condition variables */
#endif

#if !defined NTDDI_VISTA || NTDDI_VERSION < NTDDI_VISTA
#undef ALIF_EMULATED_WIN_CV
#define ALIF_EMULATED_WIN_CV 1
#endif

#if ALIF_EMULATED_WIN_CV

typedef CRITICAL_SECTION AlifMutex_T;

class AlifCond_T
{
public:
	HANDLE sem;
	int waiting;
};

#else 

typedef SRWLOCK AlifMutex_T;

typedef CONDITION_VARIABLE  AlifCond_T;

#endif

#endif 
