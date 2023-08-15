#pragma once
























































#ifndef _UINTPTR_T_DEFINED
	#define _UINTPTR_T_DEFINED
	#ifdef _WIN64
		typedef unsigned __int64  uintptr_t;
	#else
		typedef unsigned int uintptr_t;
	#endif
#endif
