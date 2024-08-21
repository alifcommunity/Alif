#pragma once








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
