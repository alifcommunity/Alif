#pragma once




#define ALIF_IMMORTAL_REFCNT ALIF_CAST(AlifSizeT, UINT_MAX) // 37



#define ALIF_IMMORTAL_REFCNT_LOCAL UINT32_MAX // 58


#define ALIF_REF_SHARED_SHIFT		2 // 65
#define ALIFREF_SHARED_FLAG_MASK	0x3 // 66

#define ALIF_REF_SHARED_INIT	0x0 // 69
#define ALIF_REF_MAYBE_WEAKREF	0x1
#define ALIF_REF_QUEUED			0x2
#define ALIF_REF_MERGED			0x3 // 72

#define ALIF_REF_SHARED(_refcnt, _flags)	\
	(((_refcnt) << ALIF_REF_SHARED_SHIFT) + (_flags)) // 75

static inline AlifSizeT _alif_refCnt(AlifObject* _ob) { // 80
	uint32_t local = alifAtomic_loadUint32Relaxed(&_ob->refLocal);
	if (local == ALIF_IMMORTAL_REFCNT_LOCAL) {
		return ALIF_IMMORTAL_REFCNT;
	}
	AlifSizeT shared = alifAtomic_loadSizeRelaxed(&_ob->refShared);
	return ALIF_STATIC_CAST(AlifSizeT, local) +
		ALIF_ARITHMETIC_RIGHT_SHIFT(AlifSizeT, shared, ALIF_REF_SHARED_SHIFT);
}
#define ALIF_REFCNT(_ob) _alif_refCnt(ALIFOBJECT_CAST(_ob))

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

void alif_dealloc(AlifObject*); // 196

static inline ALIF_ALWAYS_INLINE void alif_incRef(AlifObject* op) { // 211
	uint32_t local = alifAtomic_loadUint32Relaxed(&op->refLocal);
	uint32_t newLocal = local + 1;
	if (newLocal == 0) {
		// do nothing
		return;
	}
	if (alif_isOwnedByCurrentThread(op)) {
		alifAtomic_storeUint32Relaxed(&op->refLocal, newLocal);
	}
	else {
		alifAtomic_addSize(&op->refShared, (1 << ALIF_REF_SHARED_SHIFT));
	}

}
#define ALIF_INCREF(_op) alif_incRef(ALIFOBJECT_CAST(_op))



void alif_decRefShared(AlifObject*); // 269
void alif_decRefSharedDebug(AlifObject*, const char*, AlifIntT); // 270

void alif_mergeZeroLocalRefcount(AlifObject*); // 276


static inline void alif_decreaseRef(AlifObject* _op) { // 319
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
#define ALIF_DECREF(_op) alif_decreaseRef(ALIFOBJECT_CAST(_op)) // 337

// 432
#define ALIF_CLEAR(_op) \
    do { \
        AlifObject **tmpOpPtr = ALIF_CAST(AlifObject**, &(_op)); \
        AlifObject *tmpOldOp = (*tmpOpPtr); \
        if (tmpOldOp != nullptr) { \
            AlifObject *nullPtr = nullptr; \
            memcpy(tmpOpPtr, &nullPtr, sizeof(AlifObject*)); \
            ALIF_DECREF(tmpOldOp); \
        } \
    } while (0)



static inline void alif_xincRef(AlifObject* _op) { // 446
	if (_op != nullptr) {
		ALIF_INCREF(_op);
	}
}
#define ALIF_XINCREF(_op) alif_xincRef(ALIFOBJECT_CAST(_op))


static inline void alif_xdecRef(AlifObject* _op) { // 456
	if (_op != nullptr) {
		ALIF_DECREF(_op);
	}
}
#define ALIF_XDECREF(op) alif_xdecRef(ALIFOBJECT_CAST(op))




static inline AlifObject* alif_newRef(AlifObject* _obj) { // 473
	ALIF_INCREF(_obj);
	return _obj;
}


static inline AlifObject* alif_xnewRef(AlifObject* _obj) { // 479
	ALIF_XINCREF(_obj);
	return _obj;
}



#define ALIF_NEWREF(_obj) alif_newRef(ALIFOBJECT_CAST(_obj)) // 489
#define ALIF_XNEWREF(_obj) alif_xnewRef(ALIFOBJECT_CAST(_obj)) // 490


