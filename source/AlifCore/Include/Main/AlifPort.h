#pragma once

#ifndef UCHAR_MAX
#  error "<limits.h> header must define UCHAR_MAX"
#endif
#if UCHAR_MAX != 255
#  error "Alif's source code assumes Cpp's unsigned char is an 8-bit type"
#endif


/*
	من الأفضل إستخدام ALIF_STATIC_CAST
	بدلا من ALIF_CAST
	لانها خاصة ب cpp
	ولأنها اكثر أمان
*/
#define ALIF_STATIC_CAST(_type, _expr) static_cast<_type>(_expr)
#define ALIF_CAST(_type, _expr) ((_type)(_expr))


#if defined(_MSC_VER)
/* ignore warnings if the compiler decides not to inline a function */
#  pragma warning(disable: 4710)
   /* fastest possible local call under MSVC */
#  define ALIF_LOCAL(_type) static _type __fastcall
#  define ALIF_LOCAL_INLINE(_type) static __inline _type __fastcall
#else
#  define ALIF_LOCAL(_type) static _type
#  define ALIF_LOCAL_INLINE(_type) static inline _type
#endif


#ifdef WITH_THREAD
#    ifdef HAVE_LOCAL_THREAD
#      error "HAVE_LOCAL_THREAD معرف بالفعل"
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
