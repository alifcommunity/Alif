#pragma once

#include <stdbool.h>
#include "AlifCore_gc.h"     
#include "AlifCore_AlifState.h"

#define ALIFSUBOBJECT_HEAD_INIT(_type)       \
    {                                        \
         ALIF_IMMORTAL_REFCENT,    \
         (_type),                 \
    } \

#define ALIFSUBVAROBJECT_HEAD_INIT(_type, _size)    \
    {                                               \
        ._base_ = ALIFSUBOBJECT_HEAD_INIT(_type),   \
        .size_ = _size                              \
    } \                                              


static inline void alifSub_refcntAdd(AlifObject* op, int64_t n)
{
	if (ALIFSUB_ISIMMORTAL(op)) {
		return;
	}
#ifdef ALIF_REF_DEBUG
	alifSub_AddRefTotal(alifInterpreterState_get(), n);
#endif
#if !defined(ALIF_GIL_DISABLED)
	op->ref_ += n;
#else
	if (alifSub_IsOwnedByCurrentThread(op)) {
		uint32_t local = op->ob_ref_local;
		int64_t refcnt = (int64_t)local + n;
#  if LLONG_MAX > UINT32_MAX
		if (refcnt > (int64_t)UINT32_MAX) {
			// Make the object immortal if the 32-bit local reference count
			// would overflow.
			refcnt = ALIFSUB_IMMORTAL_REFCNT_LOCAL;
		}
#  endif
		alifSub_atomic_store_uint32_relaxed(&op->ob_ref_local, (uint32_t)refcnt);
	}
	else {
		alifSub_atomic_add_ssize(&op->oref_shared, (n << alifSub_REF_SHARED_SHIFT));
	}
#endif
}
#define ALIFSUB_REFCNTADD(op, n) alifSub_refcntAdd(ALIFSUBOBJECT_CAST(op), n)

extern void alifSub_setImmortal(AlifObject*);
extern void alifSub_setImmortalUntracked(AlifObject*);
static inline void alifSub_setMortal(AlifObject* _op, int64_t _ref)
{
    if (_op) {
        _op->ref_ = _ref;
    }
}

static inline void alifSub_clearImmortal(AlifObject* _op)
{
    if (_op) {
        alifSub_setMortal(_op, 1);
        ALIF_DECREF(_op);
    }
}
#define ALIFSUB_CLEARIMMORTAL(_op) \
    do { \
        alifSub_clearImmortal(ALIFSUBOBJECT_CAST(op)); \
        op = nullptr; \
    } while (0); \

static inline void alifSub_decref_specialized(AlifObject* _op, const Destructor _destruct)
{
    if (ALIFSUB_ISIMMORTAL(_op)) {
        return;
    }

    if (--_op->ref_ != 0) {
    }
    else {
        _destruct(_op);
    }
}

static inline int
alifSubType_hasFeature(AlifTypeObject* _type, unsigned long _feature) {
    return ((_type->flags_ & _feature) != 0);
}

static inline void alifSubObject_init(AlifObject* _op, AlifTypeObject* _typeObj)
{
    ALIFSET_TYPE(_op, _typeObj);
    ALIF_INCREF(_typeObj);
    alifSub_newReference(_op);
}

static inline void alifSubObject_initVar(AlifVarObject* _op, AlifTypeObject* _typeObj, int64_t _size)
{
    alifSubObject_init((AlifObject*)_op, _typeObj);
    ALIFSET_SIZE(_op, _size);
}


static inline void alifSubObject_gc_track(
#ifndef NDEBUG
	const char* _filename, int _lineno,
#endif
	AlifObject* _op)
{
#ifdef ALIF_GIL_DISABLED
	op->ob_gc_bits |= alifSubGC_BITS_TRACKED;
#else
	AlifGCHead* gc_ = alifSub_asGC(_op);

	AlifInterpreter* interp_ = alifInterpreter_get();
	AlifGCHead* generation0_ = &interp_->gc.young.head;
	AlifGCHead* last_ = (AlifGCHead*)(generation0_->gcPrev);
	alifSubGCHead_set_next(last_, gc_);
	alifSubGCHead_set_prev(gc_, last_);
	gc_->gcNext = ((uintptr_t)generation0_) | interp_->gc.visitedSpace;
	generation0_->gcPrev = (uintptr_t)gc_;
#endif
}

static inline void alifSubObject_gc_unTrack(
#ifndef NDEBUG
	const char* _filename, int _lineno,
#endif
	AlifObject* _op)
{
#ifdef ALIF_GIL_DISABLED
	op->ob_gc_bits |= alifSubGC_BITS_TRACKED;
#else
	AlifGCHead* gc_ = alifSub_asGC(_op);
	AlifGCHead* prev_ = alifSubGCHead_prev(gc_);
	AlifGCHead* next_ = alifSubGCHead_next(gc_);
	alifSubGCHead_set_next(prev_, next_);
	alifSubGCHead_set_prev(next_, prev_);
	gc_->gcNext = 0;
	gc_->gcPrev &= ALIFSUBGC_PREV_MASK_FINALIZED;
#endif
}

#ifdef NDEBUG
#  define ALIFSUBObject_GC_TRACK(op) \
        alifSubObject_gc_track(ALIFSUBOBJECT_CAST(op))
#  define ALIFSUBObject_GC_UNTRACK(op) \
        alifSubObject_gc_unTrack(ALIFSUBOBJECT_CAST(op))
#else
#  define ALIFSUBObject_GC_TRACK(op) \
        alifSubObject_gc_track(__FILE__, __LINE__, ALIFSUBOBJECT_CAST(op))
#  define ALIFSUBObject_GC_UNTRACK(op) \
        alifSubObject_gc_unTrack(__FILE__, __LINE__, ALIFSUBOBJECT_CAST(op))
#endif

#define ALIFSUBType_IS_GC(t) alifSubType_hasFeature((t), ALIFTPFLAGS_HAVE_GC)

static inline size_t  alifSubType_preHeaderSize(AlifTypeObject* _tp)
{
	return (
#ifndef ALIF_GIL_DISABLED
		ALIFSUBType_IS_GC(_tp) * sizeof(AlifGCHead) +
#endif
		alifSubType_hasFeature(_tp, ALIFTPFLAGS_PREHEADER) * 2 * sizeof(AlifObject*)
		);
}

void alifSubObjectGC_link(AlifObject*); // 642
