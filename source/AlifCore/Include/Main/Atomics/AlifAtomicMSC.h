#pragma once



static inline int32_t alifAtomic_addInt32(int32_t* _obj, int32_t _value) { // 38
	return (int32_t)_InterlockedExchangeAdd((volatile long*)_obj, (long)_value);
}

static inline int64_t alifAtomic_addInt64(int64_t* _obj, int64_t _value) { // 45
#if defined(_M_X64) or defined(_M_ARM64)
	return (int64_t)_InterlockedExchangeAdd64((volatile __int64*)_obj, (__int64)_value);
#else
	int64_t old_value = alifAtomic_loadInt64Relaxed(_obj);
	for (;;) {
		int64_t new_value = old_value + _value;
		if (alifAtomic_compareExchangeInt64(_obj, &old_value, new_value)) {
			return old_value;
		}
	}
#endif
}

static inline int alifAtomic_addInt(int* _obj, int _value) { // 81
	return (int)alifAtomic_addInt32((int32_t*)_obj, (int32_t)_value);
}

static inline uint64_t alifAtomic_addUint64(uint64_t* _obj, uint64_t _value) { // 96
	return (uint64_t)alifAtomic_addInt64((int64_t*)_obj, (int64_t)_value);
}

static inline intptr_t alifAtomic_addIntptr(intptr_t* _obj, intptr_t _value) { // 101
#if SIZEOF_VOID_P == 8
	return (intptr_t)alifAtomic_addInt64((int64_t*)_obj, (int64_t)_value);
#else
	return (intptr_t)alifAtomic_addInt32((int32_t*)_obj, (int32_t)_value);
#endif
}

static inline uintptr_t alifAtomic_addUintptr(uintptr_t* _obj, uintptr_t _value) { // 113
	return (uintptr_t)alifAtomic_addIntptr((intptr_t*)_obj, (intptr_t)_value);
}

static inline AlifSizeT alifAtomic_addSize(AlifSizeT* _obj, AlifSizeT _value) { // 120
	return (AlifSizeT)alifAtomic_addIntptr((intptr_t*)_obj, (intptr_t)_value);
}

static inline AlifIntT alifAtomic_compareExchangeInt8(int8_t* _obj,
	int8_t* expected, int8_t _value) { // 130
	int8_t initial = (int8_t)_InterlockedCompareExchange8( (volatile char*)_obj, (char)_value, (char)*expected);
	if (initial == *expected) {
		return 1;
	}
	*expected = initial;
	return 0;
}

static inline AlifIntT alifAtomic_compareExchangeInt32(int32_t* _obj,
	int32_t* _expected, int32_t _value) { // 160
	int32_t initial = (int32_t)_InterlockedCompareExchange(
		(volatile long*)_obj, (long)_value, (long)*_expected);
	if (initial == *_expected) {
		return 1;
	}
	*_expected = initial;
	return 0;
}

static inline AlifIntT alifAtomic_compareExchangeInt64(int64_t* _obj,
	int64_t* _expected, int64_t _value) { // 175
	int64_t initial = (int64_t)_InterlockedCompareExchange64(
		(volatile __int64*)_obj,
		(__int64)_value,
		(__int64)*_expected);
	if (initial == *_expected) {
		return 1;
	}
	*_expected = initial;
	return 0;
}

static inline AlifIntT alifAtomic_compareExchangePtr(void* _obj,
	void* _expected, void* _value) { // 190
	void* initial = _InterlockedCompareExchangePointer((void**)_obj, _value, *(void**)_expected);
	if (initial == *(void**)_expected) {
		return 1;
	}
	*(void**)_expected = initial;
	return 0;
}


static inline AlifIntT alifAtomic_compareExchangeUint8(uint8_t* _obj,
	uint8_t* _expected, uint8_t _value) { // 205
	return alifAtomic_compareExchangeInt8((int8_t*)_obj, (int8_t*)_expected, (int8_t)_value);
}


static inline AlifIntT alifAtomic_compareExchangeInt(AlifIntT* _obj,
	AlifIntT* _expected, AlifIntT _value) { // 229
	return alifAtomic_compareExchangeInt32((int32_t*)_obj,
		(int32_t*)_expected, (int32_t)_value);
}




static inline uintptr_t alifAtomic_loadUintptrRelaxed(const uintptr_t* _obj) { // 236
	return *(volatile uintptr_t*)_obj;
}

static inline AlifIntT alifAtomic_compareExchangeUint64(uint64_t* _obj, uint64_t* _expected, uint64_t _value) {  // 247
	return alifAtomic_compareExchangeInt64((int64_t*)_obj,
		(int64_t*)_expected, (int64_t)_value);
}

static inline AlifIntT alifAtomic_compareExchangeUintptr(uintptr_t* _obj, uintptr_t* expected, uintptr_t _value) { // 264
	return alifAtomic_compareExchangePtr((void**)_obj,
		(void**)expected, (void*)_value);
}


static inline AlifIntT alifAtomic_compareExchangeSize(AlifSizeT* _obj,
	AlifSizeT* _expected, AlifSizeT _value) { // 273
	return alifAtomic_compareExchangePtr((void**)_obj,
		(void**)_expected,
		(void*)_value);
}

static inline int8_t alifAtomic_exchangeInt8(int8_t* _obj, int8_t _value) { // 285
	return (int8_t)_InterlockedExchange8((volatile char*)_obj, (char)_value);
}

static inline int32_t alifAtomic_exchangeInt32(int32_t* _obj, int32_t _value) { // 299
	return (int32_t)_InterlockedExchange((volatile long*)_obj, (long)_value);
}

static inline int64_t alifAtomic_exchangeInt64(int64_t* _obj, int64_t _value) { // 306
#if defined(_M_X64) or defined(_M_ARM64)
	return (int64_t)_InterlockedExchange64((volatile __int64*)_obj, (__int64)_value);
#else
	int64_t oldValue = alifAtomic_loadInt64Relaxed(_obj);
	for (;;) {
		if (alifAtomic_compareExchangeInt64(_obj, &oldValue, _value)) {
			return oldValue;
		}
	}
#endif
}

static inline void* alifAtomic_exchangePtr(void* _obj, void* _value) { // 322
	return (void*)_InterlockedExchangePointer((void* volatile*)_obj, (void*)_value);
}

static inline uint8_t alifAtomic_exchangeUint8(uint8_t* _obj, uint8_t _value) { // 329
	return (uint8_t)alifAtomic_exchangeInt8((int8_t*)_obj, (int8_t)_value);
}

static inline AlifIntT alifAtomic_exchangeInt(AlifIntT* _obj, AlifIntT _value) { // 350
	return (AlifIntT)alifAtomic_exchangeInt32((int32_t*)_obj, (int32_t)_value);
}

static inline uint64_t alifAtomic_exchangeUint64(uint64_t* _obj, uint64_t _value) { // 366
	return (uint64_t)alifAtomic_exchangeInt64((int64_t*)_obj, (int64_t)_value);
}

static inline uintptr_t alifAtomic_exchangeUintptr(uintptr_t* _obj, uintptr_t _value) { // 381
	return (uintptr_t)alifAtomic_exchangePtr((void**)_obj, (void*)_value);
}

static inline AlifSizeT alifAtomic_exchangeSize(AlifSizeT* _obj, AlifSizeT _value) { // 390
	return (AlifSizeT)alifAtomic_exchangePtr((void**)_obj, (void*)_value);
}

static inline uint32_t alifAtomic_andUint32(uint32_t* _obj, uint32_t _value) { // 414
	return (uint32_t)_InterlockedAnd((volatile long*)_obj, (long)_value);
}

static inline uint64_t alifAtomic_andUint64(uint64_t* _obj, uint64_t _value) { // 421
#if defined(_M_X64) or defined(_M_ARM64)
	return (uint64_t)_InterlockedAnd64((volatile __int64*)_obj, (__int64)_value);
#else
	uint64_t old_value = alifAtomic_loadUint64Relaxed(_obj);
	for (;;) {
		uint64_t new_value = old_value & _value;
		if (alifAtomic_compareExchangeUint64(_obj, &old_value, new_value)) {
			return old_value;
		}
	}
#endif
}

static inline uintptr_t alifAtomic_andUintptr(uintptr_t* _obj, uintptr_t _value) { // 438
#if SIZEOF_VOID_P == 8
	return (uintptr_t)alifAtomic_andUint64((uint64_t*)_obj, (uint64_t)_value);
#else
	return (uintptr_t)alifAtomic_andUint32((uint32_t*)_obj, (uint32_t)_value);
#endif
}


static inline uint32_t alifAtomic_orUint32(uint32_t* _obj, uint32_t _value) { // 469
	return (uint32_t)_InterlockedOr((volatile long*)_obj, (long)_value);
}

static inline uint64_t alifAtomic_orUint64(uint64_t* _obj, uint64_t _value) { // 476
#if defined(_M_X64) or defined(_M_ARM64)
	return (uint64_t)_InterlockedOr64((volatile __int64*)_obj, (__int64)_value);
#else
	uint64_t oldValue = alifAtomic_loadUint64Relaxed(_obj);
	for (;;) {
		uint64_t new_value = oldValue | _value;
		if (alifAtomic_compareExchangeUint64(_obj, &oldValue, new_value)) {
			return oldValue;
		}
	}
#endif
}


static inline uintptr_t alifAtomic_orUintptr(uintptr_t* _obj, uintptr_t _value) { // 494
#if SIZEOF_VOID_P == 8
	return (uintptr_t)alifAtomic_orUint64((uint64_t*)_obj, (uint64_t)_value);
#else
	return (uintptr_t)alifAtomic_orUint32((uint32_t*)_obj, (uint32_t)_value);
#endif
}


static inline uint8_t alifAtomic_loadUint8(const uint8_t* _obj) { // 511
#if defined(_M_X64) or defined(_M_IX86)
	return *(volatile uint8_t*)_obj;
#elif defined(_M_ARM64)
	return (uint8_t)__ldar8((unsigned __int8 volatile*)_obj);
#else
#  error "no implementation of alifAtomic_loadUint8"
#endif
}

static inline uint16_t alifAtomic_loadUint16(const uint16_t* _obj) { // 523
#if defined(_M_X64) or defined(_M_IX86)
	return *(volatile uint16_t*)_obj;
#elif defined(_M_ARM64)
	return (uint16_t)__ldar16((unsigned __int16 volatile*)_obj);
#else
#  error "no implementation of alifAtomic_loadUint16"
#endif
}

static inline uint32_t alifAtomic_loadUint32(const uint32_t* _obj) { // 535
#if defined(_M_X64) or defined(_M_IX86)
	return *(volatile uint32_t*)_obj;
#elif defined(_M_ARM64)
	return (uint32_t)__ldar32((unsigned __int32 volatile*)_obj);
#else
#  error "no implementation of alifAtomic_loadUint32"
#endif
}


static inline uint64_t alifAtomic_loadUint64(const uint64_t* _obj) { // 547
#if defined(_M_X64) or defined(_M_IX86)
	return *(volatile uint64_t*)_obj;
#elif defined(_M_ARM64)
	return (uint64_t)__ldar64((unsigned __int64 volatile*)_obj);
#else
#  error "no implementation of alifAtomic_loadUint64"
#endif
}


static inline void* alifAtomic_loadPtr(const void* _obj) { // 597
#if SIZEOF_VOID_P == 8
	return (void*)alifAtomic_loadUint64((const uint64_t*)_obj);
#else
	return (void*)alifAtomic_loadUint32((const uint32_t*)_obj);
#endif
}

static inline uintptr_t alifAtomic_loadUintptr(const uintptr_t* _obj) { // 614
	return (uintptr_t)alifAtomic_loadPtr((void*)_obj);
}

static inline AlifSizeT alifAtomic_loadSize(const AlifSizeT* _obj) { // 621
	return (AlifSizeT)alifAtomic_loadPtr((void*)_obj);
}

static inline int alifAtomic_loadIntRelaxed(const AlifIntT* _obj) {  // 633
	return *(volatile AlifIntT*)_obj;
}

static inline int8_t alifAtomic_loadInt8Relaxed(const int8_t* _obj) { // 638
	return *(volatile int8_t*)_obj;
}

static inline int16_t alifAtomic_loadInt16Relaxed(const int16_t* _obj) { // 644
	return *(volatile int16_t*)_obj;
}

static inline int32_t alifAtomic_loadInt32Relaxed(const int32_t* _obj) { // 649
	return *(volatile int32_t*)_obj;
}

static inline int64_t alifAtomic_loadInt64Relaxed(const int64_t* _obj) { // 655
	return *(volatile int64_t*)_obj;
}

static inline uint8_t alifAtomic_loadUint8Relaxed(const uint8_t* _obj) { // 667
	return *(volatile uint8_t*)_obj;
}

static inline uint32_t alifAtomic_loadUint32Relaxed(const uint32_t* _obj) { // 679
	return *(volatile uint32_t*)_obj;
}


static inline uint64_t alifAtomic_loadUint64Relaxed(const uint64_t* _obj) { // 685
	return *(volatile uint64_t*)_obj;
}



static inline AlifSizeT alifAtomic_loadSizeRelaxed(const AlifSizeT* _obj) { // 703
	return *(volatile AlifSizeT*)_obj;
}

static inline void* alifAtomic_loadPtrRelaxed(const void* _obj) { // 709
	return *(void* volatile*)_obj;
}




static inline void alifAtomic_storeInt(AlifIntT* _obj, AlifIntT _value) { // 724
	(void)alifAtomic_exchangeInt(_obj, _value);
}

static inline void alifAtomic_storeUint8(uint8_t* _obj, uint8_t _value) { // 760
	(void)alifAtomic_exchangeUint8(_obj, _value);
}

static inline void alifAtomic_storeUint64(uint64_t* _obj, uint64_t _value) { // 778
	(void)alifAtomic_exchangeUint64(_obj, _value);
}

static inline void alifAtomic_storeSize(AlifSizeT* _obj, AlifSizeT _value) { // 802
	(void)alifAtomic_exchangeSize(_obj, _value);
}




static inline void alifAtomic_storeIntRelaxed(AlifIntT* _obj, AlifIntT _value) { // 811
	*(volatile AlifIntT*)_obj = _value;
}

static inline void alifAtomic_storeInt8Relaxed(int8_t* _obj, int8_t _value) { // 818
	*(volatile int8_t*)_obj = _value;
}

static inline void alifAtomic_storeInt16Relaxed(int16_t* _obj, int16_t _value) { // 824
	*(volatile int16_t*)_obj = _value;
}

static inline void alifAtomic_storeInt32Relaxed(int32_t* _obj, int32_t _value) { // 830
	*(volatile int32_t*)_obj = _value;
}

static inline void alifAtomic_storeInt64Relaxed(int64_t* _obj, int64_t _value) { // 836
	*(volatile int64_t*)_obj = _value;
}

static inline void alifAtomic_storeUint8Relaxed(uint8_t* _obj, uint8_t _value) { // 847
	*(volatile uint8_t*)_obj = _value;
}


static inline void alifAtomic_storeUint32Relaxed(uint32_t* _obj, uint32_t _value) { // 859
	*(volatile uint32_t*)_obj = _value;
}




static inline void alifAtomic_storeUintptrRelaxed(uintptr_t* _obj, uintptr_t _value) { // 871
	*(volatile uintptr_t*)_obj = _value;
}

static inline void alifAtomic_storePtrRelaxed(void* _obj, void* _value) { // 883
	*(void* volatile*)_obj = _value;
}

static inline void alifAtomic_storeSizeRelaxed(AlifSizeT* _obj, AlifSizeT _value) { // 890
	*(volatile AlifSizeT*)_obj = _value;
}

static inline void alifAtomic_storePtrRelease(void* _obj, void* _value) { // 930
#if defined(_M_X64) or defined(_M_IX86)
	* (void* volatile*)_obj = _value;
#elif defined(_M_ARM64)
	__stlr64((unsigned __int64 volatile*)_obj, (uintptr_t)_value);
#else
#  error "no implementation of alifAtomic_storePtrRelease"
#endif
}

static inline void alifAtomic_storeIntRelease(AlifIntT* _obj, AlifIntT _value) { // 954
#if defined(_M_X64) or defined(_M_IX86)
	* (int volatile*)_obj = _value;
#elif defined(_M_ARM64)
	__stlr32((unsigned __int32 volatile*)_obj, (unsigned __int32)_value);
#else
#  error "no implementation of alifAtomic_storeIntRelease"
#endif
}


static inline AlifIntT alifAtomic_loadIntAcquire(const AlifIntT* _obj) { // 979
#if defined(_M_X64) or defined(_M_IX86)
	return *(int volatile*)_obj;
#elif defined(_M_ARM64)
	return (int)__ldar32((unsigned __int32 volatile*)_obj);
#else
#  error "no implementation of alifAtomic_loadIntAcquire"
#endif
}



static inline void alifAtomic_storeUint64Release(uint64_t* _obj, uint64_t _value) { // 1005
#if defined(_M_X64) or defined(_M_IX86)
	* (uint64_t volatile*)_obj = _value;
#elif defined(_M_ARM64)
	__stlr64((unsigned __int64 volatile*)_obj, (unsigned __int64)_value);
#else
#  error "no implementation of alifAtomic_storeUint64Release"
#endif
}


static inline uint64_t alifAtomic_loadUint64Acquire(const uint64_t* _obj) { // 1018
#if defined(_M_X64) or defined(_M_IX86)
	return *(uint64_t volatile*)_obj;
#elif defined(_M_ARM64)
	return (uint64_t)__ldar64((unsigned __int64 volatile*)_obj);
#else
#  error "no implementation of alifAtomic_loadUint64Acquire"
#endif
}


static inline AlifSizeT alifAtomic_loadSizeAcquire(const AlifSizeT* _obj) { // 1043
#if defined(_M_X64) or defined(_M_IX86)
	return *(AlifSizeT volatile*)_obj;
#elif defined(_M_ARM64)
	return (AlifSizeT)__ldar64((unsigned __int64 volatile*)_obj);
#else
#  error "no implementation of alifAtomic_loadSizeAcquire"
#endif
}


static inline void alifAtomic_fenceSeqCst(void) { // 1058
#if defined(_M_ARM64)
	__dmb(_ARM64_BARRIER_ISH);
#elif defined(_M_X64)
	__faststorefence();
#elif defined(_M_IX86)
	_mm_mfence();
#else
#  error "no implementation of alifAtomic_fenceSeqCst"
#endif
}
