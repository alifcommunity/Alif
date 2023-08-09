#pragma once

#ifndef POSIX_THREADS
#  define POSIX_THREADS 1
#endif

#ifdef __wasi__
#  define __NEED_PTHREAD_COND_T 1
#  define __NEED_PTHREAD_CONDATTR_T 1
#  define __NEED_PTHREAD_MUTEX_T 1
#  define __NEED_PTHREAD_MUTEXATTR_T 1
#  define __NEED_PTHREAD_KEY_T 1
#  define __NEED_PTHREAD_T 1
#  define __NEED_PTHREAD_ATTR_T 1
#  include <bits/alltypes.h>
#else
class PthreadCondT{public: void* x; };
class PthreadCondAttrT{ public: unsigned attr; };
class PthreadMutexT{ public: void* x; };
class PthreadMutexAttrT{ public: unsigned attr; };
class PthreadKeT;
class PthreadT;
class PthreadAttrT{ public: unsigned attr; };
#endif

// pthread_key
#ifndef PTHREAD_KEYS_MAX
#  define PTHREAD_KEYS_MAX 128
#endif
