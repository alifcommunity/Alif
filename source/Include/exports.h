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





#ifndef __has_attribute
#define __has_attribute(x) 0 
#endif
#if (defined(__GNUC__) && (__GNUC__ >= 4)) ||\
        (defined(__clang__) && __has_attribute(visibility))
#define ALIF_IMPORTED_SYMBOL __attribute__ ((visibility ("default")))
#define ALIF_EXPORTED_SYMBOL __attribute__ ((visibility ("default")))
#define ALIF_LOCAL_SYMBOL  __attribute__ ((visibility ("hidden")))
#else
#define ALIF_IMPORTED_SYMBOL
#define ALIF_EXPORTED_SYMBOL
#define ALIF_LOCAL_SYMBOL
#endif
#endif
