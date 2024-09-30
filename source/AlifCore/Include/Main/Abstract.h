#pragma once


 // 276
#define ALIF_VECTORCALL_ARGUMENTS_OFFSET \
    (ALIF_STATIC_CAST(AlifUSizeT, 1) << (8 * sizeof(AlifUSizeT) - 1))

AlifSizeT alifObject_size(AlifObject*); // 328


#undef ALIFOBJECT_LENGTH
AlifSizeT alifObject_length(AlifObject*); // 342
#define ALIFOBJECT_LENGTH alifObject_size


AlifIntT alifObject_setItem(AlifObject*, AlifObject*, AlifObject*); // 357


AlifObject* alifObject_getIter(AlifObject*); // 383

AlifObject* alifIter_next(AlifObject* ); // 417


AlifIntT alifSequence_check(AlifObject*); // 664


AlifObject* alifSequence_tuple(AlifObject*); // 723



AlifIntT alifMapping_check(AlifObject*); // 806

AlifSizeT alifMapping_size(AlifObject*); // 810



AlifIntT alifMapping_getOptionalItem(AlifObject*, AlifObject*, AlifObject**); // 895




/* ------------------------------------------------------------------------------------- */





static inline AlifSizeT _alifVectorCall_nArgs(AlifUSizeT _n) { // 32
	return _n & ~ALIF_VECTORCALL_ARGUMENTS_OFFSET;
}
#define ALIFVECTORCALL_NARGS(_n) _alifVectorCall_nArgs(_n)





AlifSizeT alifObject_lengthHint(AlifObject*, AlifSizeT); // 80
