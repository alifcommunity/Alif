#pragma once

#define ALIF_BUILD_CORE

#define HAVE_IO_H

#include <io.h>

#define MS_WIN32
#define MS_WINDOWS
#define NT_THREADS

#include <winapifamily.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define MS_WINDOWS_DESKTOP
#endif
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define MS_WINDOWS_APP
#endif
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_SYSTEM)
#define MS_WINDOWS_SYSTEM
#endif
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_GAMES)
#define MS_WINDOWS_GAMES
#endif

/* ارجع 1 اذا كان يدعم ادخال واخراج طرفية ويندوز */
#if defined(MS_WINDOWS_DESKTOP) || defined(MS_WINDOWS_APP) || defined(MS_WINDOWS_SYSTEM)
#define HAVE_WINDOWS_CONSOLE_IO 1
#endif


/* Compiler specific defines */

/* ------------------------------------------------------------------------*/
/* Microsoft C defines _MSC_VER, as does clang-cl.exe */
#ifdef _MSC_VER

/* We want COMPILER to expand to a string containing _MSC_VER's *value*.
 * This is horridly tricky, because the stringization operator only works
 * on macro arguments, and doesn't evaluate macros passed *as* arguments.
 */
#define _ALIF_PASTE_VERSION(SUFFIX) \
        ("[MSC v." _ALIF_STRINGIZE(_MSC_VER) " " SUFFIX "]")
 /* e.g., this produces, after compile-time string catenation,
  *      ("[MSC v.1900 64 bit (Intel)]")
  *
  * _ALIF_STRINGIZE(_MSC_VER) expands to
  * _ALIF_STRINGIZE1(_MSC_VER) and this second macro call is scanned
  *      again for macros and so further expands to
  * _ALIF_STRINGIZE1(1900) which then expands to
  * "1900"
  */
#define _ALIF_STRINGIZE(X) _ALIF_STRINGIZE1(X)
#define _ALIF_STRINGIZE1(X) #X // يقوم بإرجاع قيمة X كقيمة نصية


#ifdef _WIN64
#define MS_WIN64
#endif

/* set the COMPILER and support tier
   *
   * win_amd64 MSVC (x86_64-pc-windows-msvc): 1
   * win32 MSVC (i686-pc-windows-msvc): 1
   * win_arm64 MSVC (aarch64-pc-windows-msvc): 3
   * other archs and ICC: 0
*/
#ifdef MS_WIN64
#if defined(_M_X64) || defined(_M_AMD64)
#if defined(__clang__)
#define COMPILER ("[Clang " __clang_version__ "] 64 bit (AMD64) with MSC v." _ALIF_STRINGIZE(_MSC_VER) " CRT]")
#define ALIF_SUPPORT_TIER 0
#elif defined(__INTEL_COMPILER)
#define COMPILER ("[ICC v." _ALIF_STRINGIZE(__INTEL_COMPILER) " 64 bit (amd64) with MSC v." _ALIF_STRINGIZE(_MSC_VER) " CRT]")
#define ALIF_SUPPORT_TIER 0
#else
#define COMPILER _ALIF_PASTE_VERSION("64 bit (AMD64)")
#define ALIF_SUPPORT_TIER 1
#endif /* __clang__ */
#define ALIFD_PLATFORM_TAG "win_amd64"
#elif defined(_M_ARM64)
#define COMPILER _ALIF_PASTE_VERSION("64 bit (ARM64)")
#define ALIF_SUPPORT_TIER 3
#define ALIFD_PLATFORM_TAG "win_arm64"
#else
#define COMPILER _ALIF_PASTE_VERSION("64 bit (Unknown)")
#define ALIF_SUPPORT_TIER 0
#endif
#endif /* MS_WIN64 */

/* set the version macros for the windows headers */
/* Alif 5.0+ requires Windows 8 or greater */
#define ALIF_WINVER 0x0602 /* _WIN32_WINNT_WIN8 */
#define ALIF_NTDDI NTDDI_WIN8

/* We only set these values when building Alif - we don't want to force
   these values on extensions, as that will affect the prototypes and
   structures exposed in the Windows headers. Even when building Alif, we
   allow a single source file to override this - they may need access to
   structures etc so it can optionally use new Windows features if it
   determines at runtime they are available.
*/
#ifndef NTDDI_VERSION
#define NTDDI_VERSION ALIF_NTDDI
#endif
#ifndef WINVER
#define WINVER ALIF_WINVER
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT ALIF_WINVER
#endif

/* _W64 is not defined for VC6 or eVC4 */
#ifndef _W64
#define _W64
#endif

/* Define like size_t, omitting the "unsigned" */
#ifdef MS_WIN64
typedef __int64 alif_size_t;
#   define ALIF_SSIZE_T_MAX LLONG_MAX
#else
typedef _W64 int Alif_ssize_t;
#   define ALIF_SSIZE_T_MAX INT_MAX
#endif
#define HAVE_ALIF_SSIZE_T 1

#if defined(MS_WIN32) && !defined(MS_WIN64)
#if defined(_M_IX86)
#if defined(__clang__)
#define COMPILER ("[Clang " __clang_version__ "] 32 bit (Intel) with MSC v." _Py_STRINGIZE(_MSC_VER) " CRT]")
#define ALIF_SUPPORT_TIER 0
#elif defined(__INTEL_COMPILER)
#define COMPILER ("[ICC v." _ALIF_STRINGIZE(__INTEL_COMPILER) " 32 bit (Intel) with MSC v." _Py_STRINGIZE(_MSC_VER) " CRT]")
#define ALIF_SUPPORT_TIER 0
#else
#define COMPILER _ALIF_PASTE_VERSION("32 bit (Intel)")
#define ALIF_SUPPORT_TIER 1
#endif /* __clang__ */
#define ALIFD_PLATFORM_TAG "win32"
#elif defined(_M_ARM)
#define COMPILER _ALIF_PASTE_VERSION("32 bit (ARM)")
#define ALIFD_PLATFORM_TAG "win_arm32"
#define ALIF_SUPPORT_TIER 0
#else
#define COMPILER _ALIF_PASTE_VERSION("32 bit (Unknown)")
#define ALIF_SUPPORT_TIER 0
#endif
#endif /* MS_WIN32 && !MS_WIN64 */

#endif /* _MSC_VER */


#if !defined(ALIF_NO_ENABLE_SHARED)
#       define ALIF_ENABLE_SHARED 1 
#endif 

/*  All windows compilers that use this header support __declspec */
#define HAVE_DECLSPEC_DLL

#ifdef _DEBUG
#define ALIF_DEBUG
#endif

#define WITH_ALIFMALLOC 1
