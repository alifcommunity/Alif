#pragma once

/*
	هذا الملف يعمل على تحديد بعض المعلوامات الخاصة بنظام التشغيل
*/

#define NT_THREADS
#define WITH_THREAD

#ifdef _WIN32
	#define _WINDOWS
	#if defined(_WIN64)
		#define _WINDOWS64
		#define _OS64
	#else
		#define _WINDOWS32
		#define _OS32
	#endif
#elif defined(__linux__)
	#ifdef __x86_64__
		#define _LINUX64
		#define _OS64
	#else
		#define _LINUX32
		#define _OS32
	#endif
#elif defined(__APPLE__)
	#ifdef TARGET_OS_MAC
		#ifdef TARGET_CPU_X86
			#define _MAC32
			#define _OS32
		#elif defined(TARGET_CPU_X86_64)
			#define _MAC64
			#define _OS64
		#elif defined(TARGET_CPU_ARM64)
			#define _MAC64_ARM
			#define _OS64
		#endif
	#endif 
#elif defined(__ARM_ARCH)
	#if __ARM_ARCH >= 8
		#define _ARM64
		#define _OS64
	#else
		#define _ARM32
		#define _OS32
	#endif
#else
	#error L"منصة تشغيل غير معروفة"
#endif



/* --------------------------------- مترجم مايكروسوفت --------------------------------- */
#ifdef _MSC_VER

	#define ALIF_COMPILER_VERSION(SUFFIX) \
			(L"[MSC v." ALIF_STRINGIZE(_MSC_VER) L" " SUFFIX L"]")


	#define ALIF_STRINGIZE(X) ALIF_STRINGIZE1(X)
	#define ALIF_STRINGIZE1(X) #X

	#ifdef _WINDOWS64
		#if defined(_M_X64) || defined(_M_AMD64)
			#if defined(__clang__)
				#define COMPILER (L"[Clang " __clang_version__ L"] 64 bit (AMD64) with MSC v." ALIF_STRINGIZE(_MSC_VER) L" CRT]")
			#elif defined(__INTEL_COMPILER)
				#define COMPILER (L"[ICC v." ALIF_STRINGIZE(__INTEL_COMPILER) L" 64 bit (amd64) with MSC v." ALIF_STRINGIZE(_MSC_VER) L" CRT]")
			#else
				#define COMPILER ALIF_COMPILER_VERSION(L"64 bit (AMD64)")
			#endif
				#define ALIFD_PLATFORM_TAG L"win_amd64"
		#elif defined(_M_ARM64)
			#define COMPILER ALIF_COMPILER_VERSION(L"64 bit (ARM64)")
			#define ALIFD_PLATFORM_TAG L"win_arm64"
		#else
			#define COMPILER ALIF_COMPILER_VERSION(L"64 bit (Unknown)")
		#endif
	#endif


	#if defined(_WINDOWS32) && !defined(_WINDOWS64)
		#if defined(_M_IX86)
			#if defined(__clang__)
				#define COMPILER (L"[Clang " __clang_version__ L"] 32 bit (Intel) with MSC v." ALIF_STRINGIZE(_MSC_VER) L" CRT]")
			#elif defined(__INTEL_COMPILER)
				#define COMPILER (L"[ICC v." ALIF_STRINGIZE(__INTEL_COMPILER) L" 32 bit (Intel) with MSC v." ALIF_STRINGIZE(_MSC_VER) L" CRT]")
			#else
				#define COMPILER ALIF_COMPILER_VERSION(L"32 bit (Intel)")
			#endif
				#define ALIFD_PLATFORM_TAG L"win32"
		#elif defined(_M_ARM)
			#define COMPILER ALIF_COMPILER_VERSION(L"32 bit (ARM)")
			#define ALIFD_PLATFORM_TAG L"win_arm32"
		#else
			#define COMPILER ALIF_COMPILER_VERSION(L"32 bit (Unknown)")
		#endif
	#endif

	/* define some ANSI types that are not defined in earlier Win headers */
	#if _MSC_VER >= 1200
		/* This file only exists in VC 6.0 or higher */
		#include <basetsd.h>
	#endif
#endif



#if defined(_OS64)
	using AlifSizeT = __int64;
	using AlifUSizeT = unsigned __int64;
	using AlifIntT = __int32;
	using AlifUIntT = unsigned __int32;
	#define ALIF_SIZET_MAX LLONG_MAX
#else
	using AlifSizeT = __int32;
	using AlifUSizeT = unsigned __int32;
	using AlifIntT = __int16;
	using AlifUIntT = unsigned __int16;
	#define ALIF_SIZET INT_MAX
#endif



#ifdef _WINDOWS
	#include <io.h>
	#define SIZEOF_WCHART 2
	#define SIZEOF_VOID_P 8

#else
#include <cstring> // memcpy()
#endif // _WINDOWS



#define WITH_FREELISTS 1
