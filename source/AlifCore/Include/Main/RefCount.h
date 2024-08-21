#pragma once








#define ALIF_IMMORTAL_REFCNT_LOCAL UINT32_MAX // 58




void alif_mergeZeroLocalRefcount(AlifObject*); // 276


static inline void alif_decreaseRef(AlifObject* _op) { // 
	uint32_t local = alifAtomic_loadUint32Relaxed(&_op->refLocal);
	if (local == ALIF_IMMORTAL_REFCNT_LOCAL) {
		return;
	}

	if (alif_isOwnedByCurrentThread(_op)) {
		local--;
		alifAtomic_storeUint32Relaxed(&_op->refLocal, local);
		if (local == 0) {
			alif_mergeZeroLocalRefcount(_op);
		}
	}
	else {
		alif_decRefShared(_op);
	}
}
#define ALIF_DECREF(_op) alif_decreaseRef(__FILE__, __LINE__, ALIFOBJECT_CAST(_op)) // 316
















void alif_dealloc(AlifObject*); // 192
