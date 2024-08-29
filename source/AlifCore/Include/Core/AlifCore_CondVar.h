#pragma once


#include "AlifCore_Thread.h"


#ifdef _POSIX_THREADS
/*
 * POSIX support
 */
#define ALIF_HAVE_CONDVAR

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>    
#endif

#define AlifMutexT pthread_mutex_t
#define AlifCondT pthread_cond_t

#elif defined(NT_THREADS)
/*
 * Windows (XP, 2003 server and later, as well as (hopefully) CE) support
 *
 * Emulated condition variables ones that work with XP and later, plus
 * example native support on VISTA and onwards.
 */
#define ALIF_HAVE_CONDVAR

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef ALIF_EMULATED_WIN_CV
#define ALIF_EMULATED_WIN_CV 0
#endif

#if !defined NTDDI_VISTA or NTDDI_VERSION < NTDDI_VISTA
#undef ALIF_EMULATED_WIN_CV
#define ALIF_EMULATED_WIN_CV 1
#endif

#if ALIF_EMULATED_WIN_CV

typedef CRITICAL_SECTION AlifMutexT;


class AlifCondT {
public:
	HANDLE sem{};
	AlifIntT waiting{};
};

#else
typedef SRWLOCK AlifMutexT;

typedef CONDITION_VARIABLE  AlifCondT;

#endif
#endif
