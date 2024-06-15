#pragma once

class AlifGCHead {
public:
	AlifUSizeT gcNext{};
	AlifUSizeT gcPrev{};
};


static inline AlifGCHead* alifSub_asGC(AlifObject* _gc) {
	char* gc = ((char*)_gc) - sizeof(AlifGCHead);
	return (AlifGCHead*)gc;
}

static inline int alifSubObject_gc_isTracked(AlifObject* _op) {
#ifdef ALIF_GIL_DISABLED
	return (op->gcBits & ALIFSUBGC_BITS_TRACKED) != 0;
#else
	AlifGCHead* gc_ = alifSub_asGC(_op);
	return (gc_->gcNext != 0);
#endif
}

#define ALIFSUBOBJECT_GC_ISTRACKED(_op) alifSubObject_gc_isTracked(ALIF_CAST(AlifObject*, _op))


#define ALIFSUBGC_PREV_MASK_FINALIZED  1
#define ALIFSUBGC_PREV_MASK_COLLECTING 2

#define ALIFSUBGC_PREV_SHIFT           2
#define ALIFSUBGC_PREV_MASK            (((uintptr_t) -1) << ALIFSUBGC_PREV_SHIFT)



static inline AlifGCHead* alifSubGCHead_next(AlifGCHead* _gc) {
	uintptr_t next_ = _gc->gcNext & ALIFSUBGC_PREV_MASK;
	return (AlifGCHead*)next_;
}
static inline void alifSubGCHead_set_next(AlifGCHead* _gc, AlifGCHead* _next) {
	uintptr_t unext_ = (uintptr_t)_next;
	_gc->gcNext = (_gc->gcNext & ~ALIFSUBGC_PREV_MASK) | unext_;
}

static inline AlifGCHead* alifSubGCHead_prev(AlifGCHead* _gc) {
	uintptr_t prev_ = (_gc->gcPrev & ALIFSUBGC_PREV_MASK);
	return (AlifGCHead*)prev_;
}

static inline void alifSubGCHead_set_prev(AlifGCHead* _gc, AlifGCHead* _prev) {
	uintptr_t uprev_ = (uintptr_t)_prev;
	_gc->gcPrev = ((_gc->gcPrev & ~ALIFSUBGC_PREV_MASK) | uprev_);
}


#define NUM_GENERATIONS 3


class GCGeneration {
public:
	AlifGCHead head{};
	AlifIntT threshold{};
	AlifIntT count{};
};

class GCGenerationStats {
public:
	AlifSizeT collections{};
	AlifSizeT collected{};
	AlifSizeT uncollectable{};
};

class AlifGCDureRun {
public:
	AlifObject* trashDeleteLater{};
	AlifIntT trashDeleteNesting{};

	AlifIntT enabled{};

	GCGeneration young{};
	GCGeneration old[2];

	GCGeneration permanentGeneration{};
	GCGenerationStats generationStats[NUM_GENERATIONS]{};

	AlifIntT collecting{};
	AlifObject* garbage{};
	AlifObject* callbacks{};
	AlifSizeT heapSize{};
	AlifSizeT workToDo{};
	AlifIntT visitedSpace{};
};

void alifSubGC_initState(class AlifGCDureRun* );
