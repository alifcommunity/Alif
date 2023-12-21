#pragma once

#include <iostream>

/* ------ for test only ------ */
#ifdef _WIN64
	#define _WINDOWS
	#define _WINDOWS64
	#define _OS64
#elif defined(_WIN32)
	#define _WINDOWS
	#define _WINDOWS32
	#define _OS32
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
