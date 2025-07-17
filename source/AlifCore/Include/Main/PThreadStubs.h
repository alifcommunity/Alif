#pragma once





#ifndef POSIX_THREADS // 8
#  define POSIX_THREADS 1
#endif


// 21
#ifdef __wasi__
#  ifndef __NEED_pthread_cond_t
#    define __NEED_pthread_cond_t 1
#  endif
#  ifndef __NEED_pthread_condattr_t
#    define __NEED_pthread_condattr_t 1
#  endif
#  ifndef __NEED_pthread_mutex_t
#    define __NEED_pthread_mutex_t 1
#  endif
#  ifndef __NEED_pthread_mutexattr_t
#    define __NEED_pthread_mutexattr_t 1
#  endif
#  ifndef __NEED_pthread_key_t
#    define __NEED_pthread_key_t 1
#  endif
#  ifndef __NEED_pthread_t
#    define __NEED_pthread_t 1
#  endif
#  ifndef __NEED_pthread_attr_t
#    define __NEED_pthread_attr_t 1
#  endif
#  include <bits/alltypes.h>
#else
typedef struct { void* __x; } pthread_cond_t;
typedef struct { unsigned __attr; } pthread_condattr_t;
typedef struct { void* __x; } pthread_mutex_t;
typedef struct { unsigned __attr; } pthread_mutexattr_t;
typedef unsigned pthread_key_t;
typedef unsigned pthread_t;
typedef struct { unsigned __attr; } pthread_attr_t;
#endif



AlifIntT pthread_mutexDestroy(pthread_mutex_t*); // 61

AlifIntT pthread_condInit(pthread_cond_t* restrict cond,
	const pthread_condattr_t* restrict attr); // 67

AlifIntT pthread_condTimedWait(pthread_cond_t* restrict cond,
	pthread_mutex_t* restrict mutex, const struct timespec* restrict abstime); // 72


#ifndef PTHREAD_KEYS_MAX // 95
#  define PTHREAD_KEYS_MAX 128
#endif








