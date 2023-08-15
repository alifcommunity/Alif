#pragma once

#include "alifConfig.h"

#include <inttypes.h>

#include <limits.h>

#define ALIF_CAST(type, expr) ((type)(expr))

#if defined(__CYGWIN__)
#       define HAVE_DECLSPEC_DLL
#endif

#include "exports.h"




#if defined(__clang__)
#define ALIF_COMP_DIAG_PUSH _Pragma("clang دفع تشخيص")
#define ALIF_COMP_DIAG_IGNORE_DEPR_DECLS \
    _Pragma("clang تجاهل التشخيص \"-Wdeprecated-declarations\"")
#define ALIF_COMP_DIAG_POP _Pragma("clang سحب تشخيص")
#elif defined(__GNUC__) \
    && ((__GNUC__ >= 5) || (__GNUC__ == 4) && (__GNUC_MINOR__ >= 6))
#define ALIF_COMP_DIAG_PUSH _Pragma("GCC دفع تشخيص")
#define ALIF_COMP_DIAG_IGNORE_DEPR_DECLS \
    _Pragma("GCC تجاهل التشخيص \"-Wdeprecated-declarations\"")
#define ALIF_COMP_DIAG_POP _Pragma("GCC سحب تشخيص")
#elif defined(_MSC_VER)
#define ALIF_COMP_DIAG_PUSH __pragma(warning(push))
#define ALIF_COMP_DIAG_IGNORE_DEPR_DECLS __pragma(warning(disable: 4996))
#define ALIF_COMP_DIAG_POP __pragma(warning(pop))
#else
#define ALIF_COMP_DIAG_PUSH
#define ALIF_COMP_DIAG_IGNORE_DEPR_DECLS
#define ALIF_COMP_DIAG_POP
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


/* only get special linkage if built as shared or platform is Cygwin */
#if defined(ALIF_ENABLE_SHARED) || defined(__CYGWIN__)
#       if defined(HAVE_DECLSPEC_DLL)
#               if defined(ALIF_BUILD_CORE) && !defined(ALIF_BUILD_CORE_MODULE)
#                       define ALIFAPI_FUNC(RTYPE) ALIF_EXPORTED_SYMBOL RTYPE
#                       define ALIFAPI_DATA(RTYPE) extern ALIF_EXPORTED_SYMBOL RTYPE
		/* module init functions inside the core need no external linkage */
		/* except for Cygwin to handle embedding */
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
		/* module init functions outside the core must be exported */
#                       define ALIFMODINIT_FUNC /* extern "C" */ ALIF_EXPORTED_SYMBOL AlifObject*
#               endif /* ALIF_BUILD_CORE */
#       endif /* HAVE_DECLSPEC_DLL */
#endif /* ALIF_ENABLE_SHARED */

#if defined(_MSC_VER)
   /* ignore warnings if the compiler decides not to inline a function */
#  pragma warning(disable: 4710)
   /* fastest possible local call under MSVC */
#  define ALIF_LOCAL(type) static type __fastcall
#  define ALIF_LOCAL_INLINE(type) static __inline type __fastcall
#else
#  define ALIF_LOCAL(type) static type
#  define ALIF_LOCAL_INLINE(type) static inline type
#endif























#ifndef WITH_THREAD
#  define WITH_THREAD
#endif

#ifdef WITH_THREAD
#  ifdef ALIF_BUILD_CORE
#    ifdef HAVE_THREAD_LOCAL
#      error "HAVE_THREAD_LOCAL معرف مسبقا"
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
#  define _Py_NO_RETURN __attribute__((__noreturn__))
#elif defined(_MSC_VER)
#  define ALIF_NO_RETURN __declspec(noreturn)
#else
#  define ALIF_NO_RETURN
#endif
#endif
