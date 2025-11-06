#pragma once


class AlifGCHead { // 12
public:
	uintptr_t gcNext{};
	uintptr_t gcPrev{};
};


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

static inline AlifIntT alifObject_gcMayBeTracked(AlifObject* _obj) { // 94
	if (!alifObject_isGC(_obj)) {
		return 0;
	}
	if (ALIFTUPLE_CHECKEXACT(_obj)) {
		return ALIFOBJECT_GC_IS_TRACKED(_obj);
	}
	return 1;
}

static inline AlifIntT alifObject_gcIsShared(AlifObject* _op) { // 112
	return alifObject_hasGCBits(_op, ALIFGC_BITS_SHARED);
}
#define ALIFOBJECT_GC_IS_SHARED(_op) alifObject_gcIsShared(ALIF_CAST(AlifObject*, _op))

static inline void alifObject_gcSetShared(AlifObject* _op) { // 117
	alifObject_setGCBits(_op, ALIFGC_BITS_SHARED);
}
#define ALIFOBJECT_GC_SET_SHARED(_op) alifObject_gcSetShared(ALIF_CAST(AlifObject*, _op))

static inline AlifIntT alifObject_gcIsSharedInline(AlifObject* _op) { // 133
	return alifObject_hasGCBits(_op, ALIFGC_BITS_SHARED_INLINE);
}
#define ALIFOBJECT_GC_IS_SHARED_INLINE(_op) \
    alifObject_gcIsSharedInline(ALIF_CAST(AlifObject*, _op))


#define ALIFGC_PREV_SHIFT           2 // 162 
#define ALIFGC_PREV_MASK            (((uintptr_t) -1) << ALIFGC_PREV_SHIFT) // 163

static inline void alifGCHead_setNext(AlifGCHead* _gc, AlifGCHead* _next) { // 191
	uintptr_t unext = (uintptr_t)_next;
	_gc->gcNext = (_gc->gcNext & ~ALIFGC_PREV_MASK) | unext;
}

static inline void alifGCHead_setPerv(AlifGCHead* _gc, AlifGCHead* _prev) { // 203
	uintptr_t uprev = (uintptr_t)_prev;
	_gc->gcPrev = ((_gc->gcPrev & ~ALIFGC_PREV_MASK) | uprev);
}

static inline int _alifGC_finalized(AlifObject* _op) {
	return alifObject_hasGCBits(_op, ALIFGC_BITS_FINALIZED);
}
static inline void _alifGC_setFinalized(AlifObject* _op) {
	alifObject_setGCBits(_op, ALIFGC_BITS_FINALIZED);
}

class GCGeneration { // 280
public:
	AlifGCHead head{};
	AlifIntT threshold{};
	AlifIntT count{};
};

class GCGenerationStats { // 296
public:
	AlifSizeT collections{};
	AlifSizeT collected{};
	AlifSizeT uncollectable{};
};

class GCRuntimeState { // 305
public:
	AlifObject* trashDeleteLater{};
	AlifIntT trashDeleteNesting{};

	AlifIntT enabled{};
	AlifIntT debug{};
	GCGeneration young{};
	GCGeneration old[2]{};
	GCGeneration permanentGeneration{};
	GCGenerationStats generationStats[3]{};
	AlifIntT collecting{};
	AlifObject* garbage{};
	AlifObject* callbacks{};

	AlifSizeT heapSize{};
	AlifSizeT workToDo{};
	AlifIntT visitedSpace{};

	AlifSizeT longLivedTotal{};

	AlifSizeT longLivedPending{};
	AlifIntT immortalize{};
};

class GCThreadState { // 357
public:
	AlifSizeT allocCount{};
};




extern void alifGC_initState(GCRuntimeState*); // 364



extern void alif_scheduleGC(AlifThread*); // 381
