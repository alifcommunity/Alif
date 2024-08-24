#pragma once


class AlifGCHead { // 12
public:
	uintptr_t gcNext;
	uintptr_t gcPrev;
} ;


static inline AlifGCHead* alif_asGC(AlifObject* _op) { // 26
	char* gc_ = ((char*)_op) - sizeof(AlifGCHead);
	return (AlifGCHead*)gc_;
}

static inline AlifObject* alif_fromGC(AlifGCHead* _gc) { // 32
	char* op_ = ((char*)_gc) + sizeof(AlifGCHead);
	return (AlifObject*)op_;
}

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

#define ALIFGC_PREV_SHIFT           2

#define ALIFGC_PREV_MASK            (((uintptr_t) -1) << ALIFGC_PREV_SHIFT) // 163

static inline void alifGCHead_set_next(AlifGCHead* _gc, AlifGCHead* _next) {
	uintptr_t unext = (uintptr_t)_next;
	_gc->gcNext = (_gc->gcNext & ~ALIFGC_PREV_MASK) | unext;
}

static inline void alifGCHead_set_perv(AlifGCHead* _gc, AlifGCHead* _prev) { // 203
	uintptr_t uprev = (uintptr_t)_prev;
	_gc->gcPrev = ((_gc->gcPrev & ~ALIFGC_PREV_MASK) | uprev);
}

class GCGeneration { // 280
public:
	AlifGCHead head;
	int threshold; 
	int count;
};

class GCGenerationStats { // 296
public:
	AlifSizeT collections;
	AlifSizeT collected;
	AlifSizeT uncollectable;
};

class AlifGCDureRun {
public:
	AlifObject* trashDeleteLater;
	int trashDeleteNesting;

	int enabled;
	int debug;
	class GCGeneration young;
	class GCGeneration old[2];
	class GCGeneration permanentGeneration;
	class GCGenerationStats generationStats[3];
	int collecting;
	AlifObject* garbage;
	AlifObject* callbacks;

	AlifSizeT heapSize;
	AlifSizeT workToDo;
	int visited_space;

#ifdef ALIF_GIL_DISABLED
	AlifSizeT longLivedTotal;

	AlifSizeT longLivedPending;
	int immortalize;
#endif
};

#ifdef ALIF_GIL_DISABLED
class GCThreadState {
public:
	AlifSizeT allocCount;
};
#endif


