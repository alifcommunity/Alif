#pragma once




static inline int32_t alifAtomic_addInt32(int32_t* _obj, int32_t _value) { // 25
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}


static inline int64_t alifAtomic_addInt64(int64_t* _obj, int64_t _value) { // 29
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}


static inline intptr_t alifAtomic_addIntptr(intptr_t* _obj, intptr_t _value) { // 33
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}


static inline AlifSizeT alifAtomic_addSize(AlifSizeT* _obj, AlifSizeT _value) { // 61
	return __atomic_fetch_add(_obj, _value, __ATOMIC_SEQ_CST);
}







static inline AlifIntT alifAtomic_compareExchangeInt64(int64_t* _obj,
	int64_t* _expected, int64_t _desired) { // 88
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






static inline uint32_t alifAtomic_loadUint32(const uint32_t* _obj) { // 278
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}


static inline uint64_t alifAtomic_loadUint64(const uint64_t* _obj) { // 282
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}

static inline AlifSizeT alifAtomic_loadSize(const AlifSizeT* _obj) { // 294
	return __atomic_load_n(_obj, __ATOMIC_SEQ_CST);
}

static inline void* alifAtomic_loadPtr(const void* _obj) { // 298
	return (void*)__atomic_load_n((void* const*)_obj, __ATOMIC_SEQ_CST);
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

static inline AlifSizeT alifAtomic_loadSizeRelaxed(const AlifSizeT* _obj) { // 353
	return __atomic_load_n(_obj, __ATOMIC_RELAXED);
}





static inline void alifAtomic_storeUint64(uint64_t* _obj, uint64_t _value) { // 404
	__atomic_store_n(_obj, _value, __ATOMIC_SEQ_CST);
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




static inline AlifSizeT alifAtomic_loadSizeAcquire(const AlifSizeT* obj) { // 535
	return __atomic_load_n(obj, __ATOMIC_ACQUIRE);
}
