#pragma once





// 520
#ifndef ALIF_USEGCC_BUILTIN_ATOMICS
#  if defined(__GNUC__) && (__GNUC__ > 4 or (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
#    define ALIF_USEGCC_BUILTIN_ATOMICS 1
#  elif defined(__clang__)
#    if __has_builtin(__atomic_load)
#      define ALIF_USEGCC_BUILTIN_ATOMICS 1
#    endif
#  endif
#endif

// 530
#if ALIF_USEGCC_BUILTIN_ATOMICS
#  define ALIF_ATOMIC_GCC_H
#  include "AlifAtomicGCC.h"
#  undef ALIF_ATOMIC_GCC_H
#elif __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#  define ALIF_ATOMIC_STD_H
#  include "AlifAtomicSTD.h"
#  undef ALIF_ATOMIC_STD_H
#elif defined(_MSC_VER)
#  define ALIF_ATOMIC_STD_H
#  include "AlifAtomicMSC.h"
#  undef ALIF_ATOMIC_STD_H
#else
#  error "no available AlifAtomic implementation for this platform/compiler"
#endif










// --------------------------------------------------------------- aliases ---------------------------------------------------------------

#if SIZEOF_LONG == 8
# define ALIF_ATOMIC_LOAD_ULONG(_p) \
    alifAtomic_loadUint64((uint64_t*)_p)
# define ALIF_ATOMIC_LOAD_ULONG_RELAXED(_p) \
    alifAtomic_loadUint64Relaxed((uint64_t*)_p)
# define ALIF_ATOMIC_STORE_ULONG(_p, _v) \
    alifAtomic_storeUint64((uint64_t*)_p, _v)
# define ALIF_ATOMIC_STORE_ULONG_RELAXED(_p, _v) \
    alifAtomic_storeUint64Relaxed((uint64_t*)_p, _v)
#elif SIZEOF_LONG == 4
# define ALIF_ATOMIC_LOAD_ULONG(_p) \
    alifAtomic_loadUint32((uint32_t*)_p)
# define ALIFATOMIC_LOAD_ULONG_RELAXED(_p) \
    alifAtomic_loadUint32Relaxed((uint32_t*)_p)
# define ALIF_ATOMIC_STORE_ULONG(_p, _v) \
    alifAtomic_storeUint32((uint32_t*)_p, _v)
# define ALIFATOMIC_STORE_ULONG_RELAXED(_p, _v) \
    alifAtomic_storeUint32Relaxed((uint32_t*)_p, _v)
#else
# error "long must be 4 or 8 bytes in size"
#endif  // SIZEOF_LONG
