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



static inline intptr_t alifAtomic_addIntptr(intptr_t* _obj, intptr_t _value) { // 101
#if SIZEOF_VOID_P == 8
	return (intptr_t)alifAtomic_addInt64((int64_t*)_obj, (int64_t)_value);
#else
	return (intptr_t)alifAtomic_addInt32((int32_t*)obj, (int32_t)value);
#endif
}

static inline AlifSizeT alifAtomic_addSize(AlifSizeT* _obj, AlifSizeT _value) { // 120
	return (AlifSizeT)alifAtomic_addIntptr((intptr_t*)_obj, (intptr_t)_value);
}

static inline AlifIntT alifAtomic_compareExchangeInt64(int64_t* _obj, int64_t* _expected, int64_t _value) { // 175
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

static inline AlifIntT alifAtomic_compareExchangePtr(void* _obj, void* _expected, void* _value) { // 190
	void* initial = _InterlockedCompareExchangePointer((void**)_obj, _value, *(void**)_expected);
	if (initial == *(void**)_expected) {
		return 1;
	}
	*(void**)_expected = initial;
	return 0;
}


static inline AlifIntT alifAtomic_compareExchangeSize(AlifSizeT* _obj,
	AlifSizeT* _expected, AlifSizeT value) { // 273
	return alifAtomic_compareExchangePtr((void**)_obj,
		(void**)_expected,
		(void*)value);
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

static inline AlifSizeT alifAtomic_loadSize(const AlifSizeT* _obj) { // 621
	return (AlifSizeT)alifAtomic_loadPtr((void*)_obj);
}




static inline int64_t alifAtomic_loadInt64Relaxed(const int64_t* _obj) { // 655
	return *(volatile int64_t*)_obj;
}


static inline uint32_t alifAtomic_loadUint32Relaxed(const uint32_t* _obj) { // 679
	return *(volatile uint32_t*)_obj;
}




static inline AlifSizeT alifAtomic_loadSizeRelaxed(const AlifSizeT* obj) { // 703
	return *(volatile AlifSizeT*)obj;
}





static inline void alifAtomic_storeUint32Relaxed(uint32_t* _obj, uint32_t _value) { // 859
	*(volatile uint32_t*)_obj = _value;
}




static inline void alifAtomic_storeUintptrRelaxed(uintptr_t* _obj, uintptr_t _value) { // 871
	*(volatile uintptr_t*)_obj = _value;
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
