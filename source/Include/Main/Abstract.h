#pragma once


AlifObject* alifObject_getIter(AlifObject*);

AlifObject* alifNumber_add(AlifObject*, AlifObject*);

AlifObject* alifInteger_inPlaceOr(AlifObject*, AlifObject*);

AlifObject* alifIter_next(AlifObject*);

AlifObject* alifInteger_float(AlifObject* );

#define ALIF_VECTORCALL_ARGUMENTS_OFFSET ((size_t)1 << (8 * sizeof(size_t) - 1))

AlifObject* alifVectorCall_call(AlifObject* , AlifObject* , AlifObject* );
AlifObject* alifSequence_fast(AlifObject* , const wchar_t* );
int alifSequence_check(AlifObject* );

#define ALIFSEQUENCE_FAST_GETSIZE(o) (((AlifVarObject*)(o))->size_ ? ((AlifVarObject*)(o))->size_ : ((AlifVarObject*)(o))->size_)

#define ALIFSEQUENCE_FAST_ITEMS(sf) ((((AlifObject*)(sf))->type_ == &typeList) ? ((AlifListObject *)(sf))->items : ((AlifTupleObject *)(sf))->items)

static inline int64_t alifSubVectorcall_NArgs(size_t _n)
{
    return _n & ~ALIF_VECTORCALL_ARGUMENTS_OFFSET;
}
#define ALIFVECTORCALL_NARGS(_n) alifSubVectorcall_NArgs(_n)
