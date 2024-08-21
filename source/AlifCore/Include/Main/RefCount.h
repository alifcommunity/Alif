#pragma once







#define ALIF_IMMORTAL_REFCNT_LOCAL UINT32_MAX // 58


#define ALIF_REF_SHARED_SHIFT		2 // 65
#define ALIFREF_SHARED_FLAG_MASK	0x3 // 66

#define ALIF_REF_SHARED_INIT	0x0 // 69
#define ALIF_REF_MAYBE_WEAKREF	0x1
#define ALIF_REF_QUEUED			0x2
#define ALIF_REF_MERGED			0x3 // 72

#define ALIF_REF_SHARED(_refcnt, _flags)	\
	(((_refcnt) << ALIF_REF_SHARED_SHIFT) + (_flags)) // 75


static inline ALIF_ALWAYS_INLINE AlifIntT alif_isImmortal(AlifObject* op) { // 98
	return (alifAtomic_loadUint32Relaxed(&op->refLocal) ==
		ALIF_IMMORTAL_REFCNT_LOCAL);
}
#define ALIF_ISIMMORTAL(_op) alif_isImmortal(ALIFOBJECT_CAST(_op))


static inline void alifSet_refCount(AlifObject* ob, AlifSizeT refcnt) { // 115
	if (ALIF_ISIMMORTAL(ob)) {
		return;
	}

	if (alif_isOwnedByCurrentThread(ob)) {
		if ((size_t)refcnt > (size_t)UINT32_MAX) {
			// On overflow, make the object immortal
			ob->threadID = ALIF_UNOWNED_TID;
			ob->refLocal = ALIF_IMMORTAL_REFCNT_LOCAL;
			ob->refShared = 0;
		}
		else {
			ob->refLocal = ALIF_STATIC_CAST(uint32_t, refcnt);
			ob->refShared &= ALIFREF_SHARED_FLAG_MASK;
		}
	}
	else {
		ob->threadID = ALIF_UNOWNED_TID;
		ob->refLocal = 0;
		ob->refShared = ALIF_REF_SHARED(refcnt, ALIF_REF_MERGED);
	}
}
#define ALIF_SET_REFCNT(_ob, _refcnt) alifSet_refCount(ALIFOBJECT_CAST(_ob), (_refcnt))



void alif_decRefShared(AlifObject*); // 269
void alif_decRefSharedDebug(AlifObject*, const char*, AlifIntT); // 270

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
