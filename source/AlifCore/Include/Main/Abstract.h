#pragma once



AlifObject* alifStack_asDict(AlifObject* const*, AlifObject* );

VectorCallFunc alifVectorCall_function(AlifObject*);

AlifObject* alifObject_vectorCall(AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*);

AlifObject* alifObject_callOneArg(AlifObject*, AlifObject*);


AlifIntT alifMapping_getOptionalItem(AlifObject*, AlifObject*, AlifObject**);

AlifObject* alifObject_getIter(AlifObject*);

int alifObject_setItem(AlifObject*, AlifObject*, AlifObject* );

AlifObject* alifNumber_add(AlifObject*, AlifObject*);
AlifObject* alifNumber_subtract(AlifObject*, AlifObject*);
AlifObject* alifNumber_inPlaceAdd(AlifObject* _x, AlifObject* _y);
AlifObject* alifNumber_inPlaceSubtract(AlifObject* _x, AlifObject* _y);

AlifObject* alifInteger_inPlaceOr(AlifObject*, AlifObject*);

AlifObject* alifIter_next(AlifObject*);

AlifObject* alifInteger_float(AlifObject* );

AlifSizeT alifNumber_asSizeT(AlifObject*, AlifObject*);

AlifObject* alifSequence_getItem(AlifObject*, AlifSizeT);

int alifSequence_setItem(AlifObject*, int64_t , AlifObject*);

#define ALIF_VECTORCALL_ARGUMENTS_OFFSET ((size_t)1 << (8 * sizeof(size_t) - 1))

AlifObject* alifVectorCall_call(AlifObject* , AlifObject* , AlifObject* );
AlifObject* alifSequence_fast(AlifObject* , const wchar_t* );
int alifSequence_check(AlifObject* );

AlifIntT alifSequence_delItem(AlifObject*, AlifSizeT);

#define ALIFSEQUENCE_FAST_GETSIZE(o) (((AlifVarObject*)(o))->size_ ? ((AlifVarObject*)(o))->size_ : ((AlifVarObject*)(o))->size_)

#define ALIFSEQUENCE_FAST_ITEMS(sf) ((((AlifObject*)(sf))->type_ == &_alifListType_) ? ((AlifListObject *)(sf))->items_ : ((AlifTupleObject *)(sf))->items_)

static inline int64_t alifSubVectorcall_nArgs(size_t _n)
{
    return _n & ~ALIF_VECTORCALL_ARGUMENTS_OFFSET;
}
#define ALIFVECTORCALL_NARGS(_n) alifSubVectorcall_nArgs(_n)
