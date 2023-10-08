#pragma once






























































































#define ALIF_UINT32_T uint32_t
#define ALIF_UINT64_T uint64_t


#define ALIF_INT32_T int32_t
#define ALIF_INT64_T int64_t







































typedef AlifSizeT AlifHashT;

































#if defined(_MSC_VER)
#  pragma warning(disable: 4710)
#  define ALIF_LOCAL(type) static type __fastcall
#  define ALIF_LOCAL_INLINE(type) static __inline type __fastcall
#else
#  define ALIF_LOCAL(type) static type
#  define ALIF_LOCAL_INLINE(type) static inline type
#endif























































































































































#if defined(__clang__)
#define ALIFCOMP_DIAGPUSH _Pragma("clang diagnostic push")
#define ALIFCOMP_DIAGIGNORE_DEPR_DECLS \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define ALIFCOMP_DIAG_POP _Pragma("clang diagnostic pop")
#elif defined(__GNUC__) \
    && ((__GNUC__ >= 5) || (__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))
#define ALIFCOMP_DIAGPUSH _Pragma("GCC diagnostic push")
#define ALIFCOMP_DIAGIGNORE_DEPR_DECLS \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define ALIFCOMP_DIAGPOP _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define ALIFCOMP_DIAGPUSH __pragma(warning(push))
#define ALIFCOMP_DIAGIGNORE_DEPRDECLS __pragma(warning(disable: 4996))
#define ALIFCOMP_DIAGPOP __pragma(warning(pop))
#else
#define ALIFCOMP_DIAGPUSH
#define ALIFCOMP_DIAGIGNORE_DEPR_DECLS
#define ALIFCOMP_DIAGPOP
#endif







































#if defined(ALIF_DEBUG)




#  define ALIF_ALWAYS_INLINE
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#  define ALIF_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define ALIF_ALWAYS_INLINE __forceinline
#else
#  define ALIF_ALWAYS_INLINE
#endif









































































































#include "exports.h"


#if defined(ALIF_ENABLE_SHARED) || defined(__CYGWIN__)
#       if defined(HAVE_DECLSPEC_DLL)
#               if defined(ALIF_BUILD_CORE) && !defined(ALIF_BUILD_CORE_MODULE)
#                       define ALIFAPI_FUNC(RTYPE) ALIF_EXPORTED_SYMBOL RTYPE
#                       define ALIFAPI_DATA(RTYPE) extern ALIF_EXPORTED_SYMBOL RTYPE

#                       if defined(__CYGWIN__)
#                               define ALIFMODINIT_FUNC ALIF_EXPORTED_SYMBOL AlifObject*
#                       else /* __CYGWIN__ */
#                               define ALIFMODINIT_FUNC AlifObject*
#                       endif /* __CYGWIN__ */
#               else /* ALIF_BUILD_CORE */

#                       if !defined(__CYGWIN__)
#                               define ALIFAPI_FUNC(RTYPE) ALIF_IMPORTED_SYMBOL RTYPE
#                       endif /* !__CYGWIN__ */
#                       define ALIFAPI_DATA(RTYPE) extern ALIF_IMPORTED_SYMBOL RTYPE
#                       if defined(__cplusplus)
#                               define ALIFMODINIT_FUNC extern "C" ALIF_EXPORTED_SYMBOL AlifObject*
#                       else /* __cplusplus */
#                               define ALIFMODINIT_FUNC ALIF_EXPORTED_SYMBOL AlifObject*
#                       endif /* __cplusplus */
#               endif /* ALIF_BUILD_CORE */
#       endif /* HAVE_DECLSPEC_DLL */
#endif /* ALIF_ENABLE_SHARED */





























































































































#ifdef WITH_THREAD
#  ifdef ALIF_BUILD_CORE
#    ifdef HAVE_THREAD_LOCAL
#      error "HAVE_THREAD_LOCAL is already defined"
#    endif
#    define HAVE_THREAD_LOCAL 1
#    ifdef thread_local
#      define ALIF_THREAD_LOCAL thread_local
#    elif __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
#      define ALIF_THREAD_LOCAL _Thread_local
#    elif defined(_MSC_VER)  
#      define ALIF_THREAD_LOCAL __declspec(thread)
#    elif defined(__GNUC__)  
#      define ALIF_THREAD_LOCAL __thread
#    else

#      undef HAVE_THREAD_LOCAL
#    endif
#  endif
#endif
























#ifndef ALIF_NO_RETURN
	#if defined(__clang__) || \
		(defined(__GNUC__) && \
		 ((__GNUC__ >= 3) || \
		  (__GNUC__ == 2) && (__GNUC_MINOR__ >= 5)))
	#  define ALIF_NO_RETURN __attribute__((__noreturn__))
	#elif defined(_MSC_VER)
	#  define ALIF_NO_RETURN __declspec(noreturn)
	#else
	#  define ALIF_NO_RETURN
	#endif
#endif
