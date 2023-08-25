#pragma once










#include "alifConfig.h"

#ifdef HAVE_STD_ATOMIC
#  include <stdatomic.h>
#endif


#if defined(_MSC_VER)
#include <intrin.h>
#if defined(_M_IX86) || defined(_M_X64)
#  include <immintrin.h>
#endif
#endif
























































































































































































































#if defined(_M_IX86) || defined(_M_X64)
enum AlifMemoryOrder {
	AlifMemory_OrderRelaxed,
	AlifMemory_OrderAcquire,
	AlifMemory_OrderRelease,
	AlifMemory_OrderAcqrel,
	AlifMemory_OrderSeqCst
};

class AlifAtomicAddress {
public:
	volatile uintptr_t value;
};

class AlifAtomicInt {
public:
	volatile int value;
};
#if defined(M_X64)
#define ALIFATOMIC_STORE64BIT(atomicVal, newVal, order) \
    switch (order) { \
    case AlifMemory_OrderAcquire: \
      _InterlockedExchange64_HLEAcquire((__int64 volatile*)&((atomicVal)->value), (__int64)(newVal)); \
      break; \
    case AlifMemory_OrderRelease: \
      _InterlockedExchange64_HLERelease((__int64 volatile*)&((atomicVal)->value), (__int64)(newVal)); \
      break; \
    default: \
      _InterlockedExchange64((__int64 volatile*)&((atomicVal)->value), (__int64)(newVal)); \
      break; \
  }
#else
#define ALIFATOMIC_STORE64BIT(atomicVal, newVal, order) ((void)0);
#endif

#define ALIFATOMIC_STORE32BIT(atomicVal, newVal, order) \
  switch (order) { \
  case AlifMemory_OrderAcquire: \
    _InterlockedExchange_HLEAcquire((volatile long*)&((atomicVal)->value), (int)(newVal)); \
    break; \
  case AlifMemory_OrderRelease: \
    _InterlockedExchange_HLERelease((volatile long*)&((atomicVal)->value), (int)(newVal)); \
    break; \
  default: \
    _InterlockedExchange((volatile long*)&((atomicVal)->value), (int)(newVal)); \
    break; \
  }

#if defined(_M_X64)
/*  This has to be an intptr_t for now.
	gil_created() uses -1 as a sentinel value, if this returns
	a uintptr_t it will do an unsigned compare and crash
*/
inline intptr_t alifAtomic_load64bitImpl(volatile uintptr_t* value, int order) {
	__int64 old;
	switch (order) {
	case AlifMemory_OrderAcquire:
	{
		do {
			old = *value;
		} while (_InterlockedCompareExchange64_HLEAcquire((volatile __int64*)value, old, old) != old);
		break;
	}
	case AlifMemory_OrderRelease:
	{
		do {
			old = *value;
		} while (_InterlockedCompareExchange64_HLERelease((volatile __int64*)value, old, old) != old);
		break;
	}
	case AlifMemory_OrderRelaxed:
		old = *value;
		break;
	default:
	{
		do {
			old = *value;
		} while (_InterlockedCompareExchange64((volatile __int64*)value, old, old) != old);
		break;
	}
	}
	return old;
}

#define ALIFATOMIC_LOAD64BIT(atomicVal, order) \
    alifAtomic_load64bitImpl((volatile uintptr_t*)&((atomicVal)->value), (order))

#else
#define ALIFATOMIC_LOAD64BIT(atomicVal, order) ((atomicVal)->value)
#endif

inline int alifAtomic_load32bitImpl(volatile int* value, int order) {
	long old;
	switch (order) {
	case AlifMemory_OrderAcquire:
	{
		do {
			old = *value;
		} while (_InterlockedCompareExchange_HLEAcquire((volatile long*)value, old, old) != old);
		break;
	}
	case AlifMemory_OrderRelease:
	{
		do {
			old = *value;
		} while (_InterlockedCompareExchange_HLERelease((volatile long*)value, old, old) != old);
		break;
	}
	case AlifMemory_OrderRelaxed:
		old = *value;
		break;
	default:
	{
		do {
			old = *value;
		} while (_InterlockedCompareExchange((volatile long*)value, old, old) != old);
		break;
	}
	}
	return old;
}

#define ALIFATOMIC_LOAD32BIT(atomicVal, order) \
    alifAtomic_load32bitImpl((volatile int*)&((atomicVal)->value), (order))

#define ALIFATOMIC_STORE_EXPLICIT(atomicVal, newVal, order) \
  if (sizeof((atomicVal)->value) == 8) { \
    ALIFATOMIC_STORE64BIT((atomicVal), newVal, order) } else { \
    ALIFATOMIC_STORE32BIT((atomicVal), newVal, order) }

#define ALIFATOMIC_LOAD_EXPLICIT(atomicVal, order) \
  ( \
    sizeof((atomicVal)->value) == 8 ? \
    ALIFATOMIC_LOAD64BIT((atomicVal), order) : \
    ALIFATOMIC_LOAD32BIT((atomicVal), order) \
  )

#elif defined(_M_ARM) || defined(_M_ARM64)
enum AlifMemoryOrder {
	AlifMemory_OrderRelaxed,
	AlifMemory_OrderAcquire,
	AlifMemory_OrderRelease,
	AlifMemory_OrderAcqRel,
	AlifMemory_OrderSeqCst
};
class AlifAtomicAddress {
public:
	volatile uintptr_t value;
};
class AlifAtomicInt{
public:
	volatile int value;
};
#endif


























































































































































#define ALIFATOMIC_STORE_RELAXED(atomicVal, newVal) \
    ALIFATOMIC_STORE_EXPLICIT((atomicVal), (newVal), AlifMemory_OrderRelaxed)
#define ALIFATOMIC_LOAD_RELAXED(atomicVal) \
    ALIFATOMIC_LOAD_EXPLICIT((atomicVal), AlifMemory_OrderRelaxed)

