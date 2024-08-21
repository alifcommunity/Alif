#pragma once













static inline uint32_t alifAtomic_loadUint32Relaxed(const uint32_t* obj) { // 679
	return *(volatile uint32_t*)obj;
}










static inline void alifAtomic_storeUint32Relaxed(uint32_t* obj, uint32_t value) { // 859
	*(volatile uint32_t*)obj = value;
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
