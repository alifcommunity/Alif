#pragma once


static inline AlifIntT alifAtomic_addInt(AlifIntT* _obj, AlifIntT _value) { // 13
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}


static inline int16_t alifAtomic_addInt16(int16_t* _obj, int16_t _value) { // 21
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}


static inline int32_t alifAtomic_addInt32(int32_t* _obj, int32_t _value) { // 25
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}


static inline int64_t alifAtomic_addInt64(int64_t* _obj, int64_t _value) { // 29
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}


static inline intptr_t alifAtomic_addIntptr(intptr_t* _obj, intptr_t _value) { // 33
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uint16_t alifAtomic_addUint16(uint16_t* _obj, uint16_t _value) { // 45
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uint64_t alifAtomic_addUint64(uint64_t* _obj, uint64_t _value) { // 53
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uintptr_t alifAtomic_addUintptr(uintptr_t* _obj, uintptr_t _value) { // 57
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline AlifSizeT alifAtomic_addSize(AlifSizeT* _obj, AlifSizeT _value) { // 61
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}



static inline AlifIntT alifAtomic_compareExchangeInt(AlifIntT* _obj,
	AlifIntT* _expected, AlifIntT _desired) { // 68
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline AlifIntT alifAtomic_compareExchangeInt8(int8_t* _obj,
	int8_t* _expected, int8_t _desired) { // 73
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline AlifIntT alifAtomic_compareExchangeInt32(int32_t* _obj,
	int32_t* _expected, int32_t _desired) { // 83
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}


static inline AlifIntT alifAtomic_compareExchangeInt64(int64_t* _obj,
	int64_t* _expected, int64_t _desired) { // 88
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}


static inline AlifIntT alifAtomic_compareExchangeUint8(uint8_t* _obj,
	uint8_t* _expected, uint8_t _desired) { // 103
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline AlifIntT alifAtomic_compareExchangeUint64(uint64_t* _obj,
	uint64_t* _expected, uint64_t _desired) { // 118
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline AlifIntT alifAtomic_compareExchangeUintptr(uintptr_t* _obj,
	uintptr_t* _expected, uintptr_t _desired) { // 123
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}


static inline AlifIntT alifAtomic_compareExchangeSize(AlifSizeT* _obj,
	AlifSizeT* _expected, AlifSizeT _desired) { // 128
	return __atomic_compare_exchange_n(_obj, _expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}



static inline AlifIntT alifAtomic_compareExchangePtr(void* _obj,
	void* _expected, void* _desired) { // 133
	return __atomic_compare_exchange_n((void**)_obj, (void**)_expected, _desired, 0,
		__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}



static inline AlifIntT alifAtomic_exchangeInt(AlifIntT* _obj, AlifIntT _value) { // 141
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline int8_t alifAtomic_exchangeInt8(int8_t* _obj, int8_t _value) { // 145
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline int32_t alifAtomic_exchangeInt32(int32_t* _obj, int32_t _value) { // 153
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline int64_t alifAtomic_exchangeInt64(int64_t* _obj, int64_t _value) { // 157
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uint8_t alifAtomic_exchangeUint8(uint8_t* _obj, uint8_t _value) { // 169
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uint64_t alifAtomic_exchangeUint64(uint64_t* _obj, uint64_t _value) { // 181
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uintptr_t AlifAtomic_exchangeUintptr(uintptr_t* _obj, uintptr_t _value) { // 185
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline AlifSizeT alifAtomic_exchangeSize(AlifSizeT* _obj, AlifSizeT _value) { // 189
	return __atomic_exchange_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline void* alifAtomic_exchangeptr(void* _obj, void* _value) { // 193
	return __atomic_exchange_n((void**)_obj, _value, __ATOMIC_SEQ_CST);
}




static inline uint32_t alifAtomic_andUint32(uint32_t* _obj, uint32_t _value) { // 208
	return __atomic_fetch_and(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uint64_t alifAtomic_andUint64(uint64_t* _obj, uint64_t _value) { // 212
	return __atomic_fetch_and(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uintptr_t alifAtomic_andUintptr(uintptr_t* _obj, uintptr_t _value) { // 216
	return __atomic_fetch_and(_obj, _value, __ATOMIC_SEQ_CST);
}



static inline uint32_t alifAtomic_orUint32(uint32_t* _obj, uint32_t _value) { // 231
	return __atomic_fetch_or(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uint64_t alifAtomic_orUint64(uint64_t* _obj, uint64_t _value) { // 235
	return __atomic_fetch_or(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline uintptr_t alifAtomic_orUintptr(uintptr_t* _obj, uintptr_t _value) { // 239
	return __atomic_fetch_or(_obj, _value, __ATOMIC_SEQ_CST);
}



static inline uint8_t alifAtomic_loadUint8(const uint8_t* _obj) { // 270
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}

static inline uint16_t alifAtomic_loadUint16(const uint16_t* _obj) { // 274
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}

static inline uint32_t alifAtomic_loadUint32(const uint32_t* _obj) { // 278
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}


static inline uint64_t alifAtomic_loadUint64(const uint64_t* _obj) { // 282
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}

static inline uintptr_t alifAtomic_loaduintptr(const uintptr_t* _obj) { // 286
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}

static inline AlifSizeT alifAtomic_loadSize(const AlifSizeT* _obj) { // 294
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}

static inline void* alifAtomic_loadPtr(const void* _obj) { // 298
	return (void*)__atomic_load_n((void* const*)_obj, __ATOMIC_SEQ_CST);
}





static inline AlifIntT AlifAtomic_loadIntRelaxed(const AlifIntT* _obj) { // 305
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline int8_t alifAtomic_loadInt8Relaxed(const int8_t* _obj) { // 309
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline int16_t alifAtomic_loadInt16Relaxed(const int16_t* _obj) { // 313
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline int32_t alifAtomic_loadInt32Relaxed(const int32_t* _obj) { // 317
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline int64_t alifAtomic_loadInt64Relaxed(const int64_t* _obj) { // 321
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}


static inline uint8_t alifAtomic_loadUint8Relaxed(const uint8_t* _obj) { // 329
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}


static inline uint32_t alifAtomic_loadUint32Relaxed(const uint32_t* _obj) { // 337
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline uint64_t alifAtomic_loadUint64Relaxed(const uint64_t* _obj) { // 341
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline uintptr_t alifAtomic_loadUintptrRelaxed(const uintptr_t* _obj) { // 345
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline AlifSizeT alifAtomic_loadSizeRelaxed(const AlifSizeT* _obj) { // 353
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}

static inline void* alifAtomic_loadPtrRelaxed(const void* _obj) { // 357
	return (void*)__atomic_load_n((void* const*)_obj, __ATOMIC_RELAXED);
}




static inline void alifAtomic_storeInt(AlifIntT* _obj, AlifIntT _value) { // 368
	__atomic_store_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline void alifAtomic_storeUint8(uint8_t* _obj, uint8_t _value) { // 392
	__atomic_store_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline void alifAtomic_storeUint64(uint64_t* _obj, uint64_t _value) { // 404
	__atomic_store_n(_obj, _value, __ATOMIC_SEQ_CST);
}

static inline void alifAtomic_storeSize(AlifSizeT* _obj, AlifSizeT _value) { // 420
	__atomic_store_n(_obj, _value, __ATOMIC_SEQ_CST);
}




static inline void alifAtomic_storeIntRelaxed(AlifIntT* _obj, AlifIntT _value) { // 427
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeInt8Relaxed(int8_t* _obj, int8_t _value) { // 431
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeInt16Relaxed(int16_t* _obj, int16_t _value) { // 435
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeInt32Relaxed(int32_t* _obj, int32_t _value) { // 439
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeInt64Relaxed(int64_t* _obj, int64_t _value) { // 443
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeUint8Relaxed(uint8_t* _obj, uint8_t _value) { // 451
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeUint32Relaxed(uint32_t* _obj, uint32_t _value) { // 459
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeUint64Relaxed(uint64_t* _obj, uint64_t _value) { // 463
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeUintptrRelaxed(uintptr_t* _obj, uintptr_t _value) { // 467
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storePtrRelaxed(void* _obj, void* _value) { // 475
	__atomic_store_n((void**)_obj, _value, __ATOMIC_RELAXED);
}

static inline void alifAtomic_storeSizeRelaxed(AlifSizeT* _obj, AlifSizeT _value) { // 479
	__atomic_store_n(_obj, _value, __ATOMIC_RELAXED);
}




static inline void* alifAtomic_loadPtrAcquire(const void* _obj) { // 491
	return (void*)__atomic_load_n((void* const*)_obj, __ATOMIC_ACQUIRE);
}

static inline void alifAtomic_storeIntRelease(AlifIntT* _obj, AlifIntT _value) { // 507
	__atomic_store_n(_obj, _value, __ATOMIC_RELEASE);
}

static inline void alifAtomic_storeSizeRelease(AlifSizeT* _obj, AlifSizeT _value) { // 511
	__atomic_store_n(_obj, _value, __ATOMIC_RELEASE);
}

static inline AlifIntT alifAtomic_loadIntAcquire(const AlifIntT* _obj) { // 515
	return __atomic_load_n(_obj, __ATOMIC_ACQUIRE);
}

static inline void alifAtomic_storeUint64Release(uint64_t* _obj, uint64_t _value) { // 523
	__atomic_store_n(_obj, _value, __ATOMIC_RELEASE);
}

static inline uint64_t alifAtomic_loadUint64Acquire(const uint64_t* _obj) { // 527
	return __atomic_load_n(_obj, __ATOMIC_ACQUIRE);
}

static inline uint32_t alifAtomic_loadUint32Acquire(const uint32_t* _obj) { // 531
	return __atomic_load_n(_obj, __ATOMIC_ACQUIRE);
}

static inline AlifSizeT alifAtomic_loadSizeAcquire(const AlifSizeT* _obj) { // 535
	return __atomic_load_n(_obj, __ATOMIC_ACQUIRE);
}

static inline void alifAtomic_fenceSeqCst() { // 541
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
}

static inline void alifAtomic_fenceAcquire() { // 545
	__atomic_thread_fence(__ATOMIC_ACQUIRE);
}
