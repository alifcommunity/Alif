#pragma once

#define ALIF_CAST(type_, expr) ((type_)(expr))



#ifdef WITH_THREAD
#    ifdef HAVE_LOCAL_THREAD
#      error L"HAVE_LOCAL_THREAD معرف بالفعل"
#    endif
#    define HAVE_LOCAL_THREAD 1
#    ifdef thread_local
#      define ALIF_LOCAL_THREAD thread_local
#    elif __STDC_VERSION__ >= 201112L and !defined(__STDC_NO_THREADS__)
#      define ALIF_LOCAL_THREAD _Thread_local
#    elif defined(_MSC_VER) 
#      define ALIF_LOCAL_THREAD __declspec(thread)
#    elif defined(__GNUC__)  /* includes clang */
#      define ALIF_LOCAL_THREAD __thread
#    else
#      undef HAVE_LOCAL_THREAD
#    endif
#endif
