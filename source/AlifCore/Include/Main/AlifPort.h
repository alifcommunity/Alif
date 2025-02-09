#pragma once

#ifndef UCHAR_MAX
#  error "<limits.h> header must define UCHAR_MAX"
#endif
#if UCHAR_MAX != 255
#  error "Alif's source code assumes Cpp's unsigned char is an 8-bit type"
#endif


#ifdef __has_builtin
#  define ALIF_HAS_BUILTIN(x) __has_builtin(x)
#else
#  define ALIF_HAS_BUILTIN(x) 0
#endif

#ifdef __has_attribute
#  define ALIF_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#  define ALIF_HAS_ATTRIBUTE(x) 0
#endif

/*
	من الأفضل إستخدام ALIF_STATIC_CAST
	بدلا من ALIF_CAST
	لانها خاصة ب cpp
	ولأنها اكثر أمان
*/
#define ALIF_STATIC_CAST(_type, _expr) static_cast<_type>(_expr)
#define ALIF_CAST(_type, _expr) ((_type)(_expr))






#ifndef ALIFLONG_BITS_IN_DIGIT
#define ALIFLONG_BITS_IN_DIGIT 30
#endif







typedef AlifSizeT AlifHashT;
#define SIZEOF_ALIF_UHASH_T SIZEOF_SIZE_T
typedef AlifUSizeT AlifUHashT;






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



#ifdef SIGNED_RIGHT_SHIFT_ZERO_FILLS
#define ALIF_ARITHMETIC_RIGHT_SHIFT(TYPE, I, J) \
    ((I) < 0 ? -1-((-1-(I)) >> (J)) : (I) >> (J))
#else
#define ALIF_ARITHMETIC_RIGHT_SHIFT(TYPE, I, J) ((I) >> (J))
#endif


#define ALIF_SAFE_DOWNCAST(_val, _wide, _narrow) ALIF_STATIC_CAST(_narrow, (_val)) // 256



// 323 
#if defined(__GNUC__) \
    && ((__GNUC__ >= 5) or (__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#define ALIF_HOT_FUNCTION __attribute__((hot))
#else
#define ALIF_HOT_FUNCTION
#endif


#if defined(__GNUC__) or defined(__clang__) or defined(__INTEL_COMPILER)
#  define ALIF_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define ALIF_ALWAYS_INLINE __forceinline
#else
#  define ALIF_ALWAYS_INLINE
#endif



#if defined(__GNUC__) or defined(__clang__) or defined(__INTEL_COMPILER)
#  define ALIF_NO_INLINE __attribute__ ((noinline))
#elif defined(_MSC_VER)
#  define ALIF_NO_INLINE __declspec(noinline)
#else
#  define ALIF_NO_INLINE
#endif




#ifdef WORDS_BIGENDIAN
#  define ALIF_BIG_ENDIAN 1
#  define ALIF_LITTLE_ENDIAN 0
#else
#  define ALIF_BIG_ENDIAN 0
#  define ALIF_LITTLE_ENDIAN 1
#endif


#define ALIF_DWORD_MAX 4294967295U



#ifndef WITH_THREAD
#define WITH_THREAD
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










#ifndef ALIF_NO_RETURN
#if defined(__clang__) or \
    (defined(__GNUC__) and \
     ((__GNUC__ >= 3) or \
      (__GNUC__ == 2) and (__GNUC_MINOR__ >= 5)))
#  define ALIF_NO_RETURN __attribute__((__noreturn__))
#elif defined(_MSC_VER)
#  define ALIF_NO_RETURN __declspec(noreturn)
#else
#  define ALIF_NO_RETURN
#endif
#endif










// 629
#if ALIF_HAS_ATTRIBUTE(fallthrough)
#  define ALIF_FALLTHROUGH __attribute__((fallthrough))
#else
#  define ALIF_FALLTHROUGH do { } while (0)
#endif
