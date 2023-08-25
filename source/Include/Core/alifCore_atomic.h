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
enum AlifMemory_order {
	AlifMemory__orderRelaxed,
	AlifMemory__orderAcquire,
	AlifMemory__orderRelease,
	AlifMemory__orderAcqrel,
	AlifMemory__orderSeqCst
};

class AlifAtomicAddress {
public:
	volatile uintptr_t _value;
};

class AlifAtomicInt {
public:
	volatile int _value;
};
#if defined(M_X64)
#define ALIFATOMIC_STORE64BIT(_atomicVal, _newVal, _order) \
    switch (_order) { \
    case AlifMemory__orderAcquire: \
      _InterlockedExchange64_HLEAcquire((__int64 volatile*)&((_atomicVal)->_value), (__int64)(_newVal)); \
      break; \
    case AlifMemory__orderRelease: \
      _InterlockedExchange64_HLERelease((__int64 volatile*)&((_atomicVal)->_value), (__int64)(_newVal)); \
      break; \
    default: \
      _InterlockedExchange64((__int64 volatile*)&((_atomicVal)->_value), (__int64)(_newVal)); \
      break; \
  }
#else
#define ALIFATOMIC_STORE64BIT(_atomicVal, _newVal, _order) ((void)0);
#endif

#define ALIFATOMIC_STORE32BIT(_atomicVal, _newVal, _order) \
  switch (_order) { \
  case AlifMemory__orderAcquire: \
    _InterlockedExchange_HLEAcquire((volatile long*)&((_atomicVal)->_value), (int)(_newVal)); \
    break; \
  case AlifMemory__orderRelease: \
    _InterlockedExchange_HLERelease((volatile long*)&((_atomicVal)->_value), (int)(_newVal)); \
    break; \
  default: \
    _InterlockedExchange((volatile long*)&((_atomicVal)->_value), (int)(_newVal)); \
    break; \
  }

#if defined(_M_X64)
/*  This has to be an intptr_t for now.
	gil_created() uses -1 as a sentinel _value, if this returns
	a uintptr_t it will do an unsigned compare and crash
*/
inline intptr_t alifAtomic_load64bitImpl(volatile uintptr_t* _value, int _order) {
	__int64 old;
	switch (_order) {
	case AlifMemory__orderAcquire:
	{
		do {
			old = *_value;
		} while (_InterlockedCompareExchange64_HLEAcquire((volatile __int64*)_value, old, old) != old);
		break;
	}
	case AlifMemory__orderRelease:
	{
		do {
			old = *_value;
		} while (_InterlockedCompareExchange64_HLERelease((volatile __int64*)_value, old, old) != old);
		break;
	}
	case AlifMemory__orderRelaxed:
		old = *_value;
		break;
	default:
	{
		do {
			old = *_value;
		} while (_InterlockedCompareExchange64((volatile __int64*)_value, old, old) != old);
		break;
	}
	}
	return old;
}

#define ALIFATOMIC_LOAD64BIT(_atomicVal, _order) \
    alifAtomic_load64bitImpl((volatile uintptr_t*)&((_atomicVal)->_value), (_order))

#else
#define ALIFATOMIC_LOAD64BIT(_atomicVal, _order) ((_atomicVal)->_value)
#endif

inline int alifAtomic_load32bitImpl(volatile int* _value, int _order) {
	long old;
	switch (_order) {
	case AlifMemory__orderAcquire:
	{
		do {
			old = *_value;
		} while (_InterlockedCompareExchange_HLEAcquire((volatile long*)_value, old, old) != old);
		break;
	}
	case AlifMemory__orderRelease:
	{
		do {
			old = *_value;
		} while (_InterlockedCompareExchange_HLERelease((volatile long*)_value, old, old) != old);
		break;
	}
	case AlifMemory__orderRelaxed:
		old = *_value;
		break;
	default:
	{
		do {
			old = *_value;
		} while (_InterlockedCompareExchange((volatile long*)_value, old, old) != old);
		break;
	}
	}
	return old;
}

#define ALIFATOMIC_LOAD32BIT(_atomicVal, _order) \
    alifAtomic_load32bitImpl((volatile int*)&((_atomicVal)->_value), (_order))

#define ALIFATOMIC_STORE_EXPLICIT(_atomicVal, _newVal, _order) \
  if (sizeof((_atomicVal)->_value) == 8) { \
    ALIFATOMIC_STORE64BIT((_atomicVal), _newVal, _order) } else { \
    ALIFATOMIC_STORE32BIT((_atomicVal), _newVal, _order) }

#define ALIFATOMIC_LOAD_EXPLICIT(_atomicVal, _order) \
  ( \
    sizeof((_atomicVal)->_value) == 8 ? \
    ALIFATOMIC_LOAD64BIT((_atomicVal), _order) : \
    ALIFATOMIC_LOAD32BIT((_atomicVal), _order) \
  )

#elif defined(_M_ARM) || defined(_M_ARM64)
enum AlifMemory_order {
	AlifMemory__orderRelaxed,
	AlifMemory__orderAcquire,
	AlifMemory__orderRelease,
	AlifMemory__orderAcqRel,
	AlifMemory__orderSeqCst
};
class AlifAtomicAddress {
public:
	volatile uintptr_t _value;
};
class AlifAtomicInt{
public:
	volatile int _value;
};
#endif


























































































































































#define ALIFATOMIC_STORE_RELAXED(_atomicVal, _newVal) \
    ALIFATOMIC_STORE_EXPLICIT((_atomicVal), (_newVal), AlifMemory__orderRelaxed)
#define ALIFATOMIC_LOAD_RELAXED(_atomicVal) \
    ALIFATOMIC_LOAD_EXPLICIT((_atomicVal), AlifMemory__orderRelaxed)

