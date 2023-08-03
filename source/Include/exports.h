#pragma once

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
	#ifndef __has_attribute
		#define __has_attribute(x) 0  // Compatibility with non-clang compilers.
	#endif
	#if (defined(__GNUC__) && (__GNUC__ >= 4)) || (defined(__clang__) && __has_attribute(visibility))
		#define ALIF_IMPORTED_SYMBOL __attribute__ ((visibility (L"إفتراضي")))
		#define ALIF_EXPORTED_SYMBOL __attribute__ ((visibility (L"إفتراضي")))
		#define ALIF_LOCAL_SYMBOL  __attribute__ ((visibility (L"مخفي")))
	#else
		#define ALIF_IMPORTED_SYMBOL
		#define ALIF_EXPORTED_SYMBOL
		#define ALIF_LOCAL_SYMBOL
	#endif
#endif
