#pragma once



/* Declarations for symbol visibility.

ALIFAPI_FUNC(type): Declares a public Alif API function and return type
ALIFAPI_DATA(type): Declares public Alif data and its type
ALIFMODINIT_FUNC:   A Alif module init function.  If these functions are
inside Alif core, they are private to the core.
If in an extension module, it may be declared with
external linkage depending on the platform.

As a number of platforms support/require "__declspec(dllimport/dllexport)",
we support a HAVE_DECLSPEC_DLL macro to save duplication.
*/

/*
All windows ports, except cygwin, are handled in AlifConfig.h.

Cygwin is the only other autoconf platform requiring special
linkage handling and it uses __declspec().
*/
#if defined(__CYGWIN__)
#       define HAVE_DECLSPEC_DLL
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(ALIF_ENABLE_SHARED)
#define ALIF_IMPORTED_SYMBOL __declspec(dllimport)
#define ALIF_EXPORTED_SYMBOL __declspec(dllexport)
#define ALIF_LOCAL_SYMBOL
#else
#define ALIF_IMPORTED_SYMBOL
#define ALIF_EXPORTED_SYMBOL
#define ALIF_LOCAL_SYMBOL
#endif
#else
/*
* If we only ever used gcc >= 5, we could use __has_attribute(visibility)
* as a cross-platform way to determine if visibility is supported. However,
* we may still need to support gcc >= 4, as some Ubuntu LTS and Centos versions
* have 4 < gcc < 5.
*/
#if (defined(__GNUC__) && (__GNUC__ >= 4)) ||\
        (defined(__clang__) && ALIF_HAS_ATTRIBUTE(visibility))
#define ALIF_IMPORTED_SYMBOL __attribute__ ((visibility ("default")))
#define ALIF_EXPORTED_SYMBOL __attribute__ ((visibility ("default")))
#define ALIF_LOCAL_SYMBOL  __attribute__ ((visibility ("hidden")))
#else
#define ALIF_IMPORTED_SYMBOL
#define ALIF_EXPORTED_SYMBOL
#define ALIF_LOCAL_SYMBOL
#endif
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
/* Building an extension module, or an embedded situation */
/* public Alif functions and data are imported */
/* Under Cygwin, auto-import functions to prevent compilation */
/* failures similar to those described at the bottom of 4.1: */
#                       if !defined(__CYGWIN__)
#                               define ALIFAPI_FUNC(RTYPE) ALIF_IMPORTED_SYMBOL RTYPE
#                       endif /* !__CYGWIN__ */
#                       define ALIFAPI_DATA(RTYPE) extern ALIF_IMPORTED_SYMBOL RTYPE
/* module init functions outside the core must be exported */
#                       if defined(__cplusplus)
#                               define ALIFMODINIT_FUNC extern "C" ALIF_EXPORTED_SYMBOL AlifObject*
#                       else /* __cplusplus */
#                               define ALIFMODINIT_FUNC ALIF_EXPORTED_SYMBOL AlifObject*
#                       endif /* __cplusplus */
#               endif /* ALIF_BUILD_CORE */
#       endif /* HAVE_DECLSPEC_DLL */
#endif /* ALIF_ENABLE_SHARED */

/* If no external linkage macros defined by now, create defaults */
#ifndef ALIFAPI_FUNC
#       define ALIFAPI_FUNC(RTYPE) ALIF_EXPORTED_SYMBOL RTYPE
#endif
#ifndef ALIFAPI_DATA
#       define ALIFAPI_DATA(RTYPE) extern ALIF_EXPORTED_SYMBOL RTYPE
#endif
#ifndef ALIFMODINIT_FUNC
#       if defined(__cplusplus)
#               define ALIFMODINIT_FUNC extern "C" ALIF_EXPORTED_SYMBOL AlifObject*
#       else /* __cplusplus */
#               define ALIFMODINIT_FUNC ALIF_EXPORTED_SYMBOL AlifObject*
#       endif /* __cplusplus */
#endif
