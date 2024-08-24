#pragma once


// 48
#define ALIFGC_BITS_TRACKED        (1) 
#define ALIFGC_BITS_FINALIZED      (2)
#define ALIFGC_BITS_UNREACHABLE    (4)
#define ALIFGC_BITS_FROZEN         (8)
#define ALIFGC_BITS_SHARED         (16)
#define ALIFGC_BITS_SHARED_INLINE  (32)
#define ALIFGC_BITS_DEFERRED       (64) 




static inline void alifObject_setGCBits(AlifObject* _op, uint8_t _newBits) { // 59
	uint8_t bits = alifAtomic_loadUint8Relaxed(&_op->gcBits);
	alifAtomic_storeUint8Relaxed(&_op->gcBits, bits | _newBits);
}

static inline AlifIntT alifObject_hasGCBits(AlifObject* _op, uint8_t _bits) { // 66
	return (alifAtomic_loadUint8Relaxed(&_op->gcBits) & _bits) != 0;
}

static inline void alifObject_clearGCBits(AlifObject* op, uint8_t bits_to_clear) { // 72
	uint8_t bits = alifAtomic_loadUint8Relaxed(&op->gcBits);
	alifAtomic_storeUint8Relaxed(&op->gcBits, bits & ~bits_to_clear);
}



static inline AlifIntT alifObjectGC_isTracked(AlifObject* _op) { // 82
	return alifObject_hasGCBits(_op, ALIFGC_BITS_TRACKED);
}
#define ALIFOBJECT_GC_IS_TRACKED(_op) alifObjectGC_isTracked(ALIF_CAST(AlifObject*, _op))
