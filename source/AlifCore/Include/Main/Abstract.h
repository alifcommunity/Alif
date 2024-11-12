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


AlifObject* alifNumber_add(AlifObject*, AlifObject*); // 443
AlifObject* alifNumber_subtract(AlifObject*, AlifObject*); // 448

AlifObject* alifNumber_multiply(AlifObject*, AlifObject*); // 453

AlifObject* alifNumber_trueDivide(AlifObject*, AlifObject*); // 470


AlifObject* alifNumber_divmod(AlifObject*, AlifObject*); // 482

AlifObject* alifNumber_power(AlifObject*, AlifObject*, AlifObject*); // 488


// this funcs in Abstract.cpp alter-line: 1361
AlifObject* alifNumber_negative(AlifObject*); // 494
AlifObject* alifNumber_positive(AlifObject*); // 499
AlifObject* alifNumber_absolute(AlifObject*); // 504
AlifObject* alifNumber_invert(AlifObject*); // 509
AlifObject* alifNumber_sqrt(AlifObject*); // alif
AlifObject* alifNumber_lshift(AlifObject*, AlifObject*); // 514
AlifObject* alifNumber_rshift(AlifObject*, AlifObject*); // 520
AlifObject* alifNumber_and(AlifObject*, AlifObject*); // 526
AlifObject* alifNumber_xor(AlifObject*, AlifObject*); // 531
AlifObject* alifNumber_or(AlifObject*, AlifObject*); // 537

AlifObject* alifNumber_index(AlifObject*); // 545

AlifSizeT alifNumber_asSizeT(AlifObject*, AlifObject*); // 553

AlifIntT alifSequence_check(AlifObject*); // 664

AlifObject* alifSequence_getItem(AlifObject*, AlifSizeT); // 689

AlifIntT alifSequence_setItem(AlifObject*, AlifSizeT, AlifObject*); // 700

AlifObject* alifSequence_tuple(AlifObject*); // 723



AlifIntT alifMapping_check(AlifObject*); // 806

AlifSizeT alifMapping_size(AlifObject*); // 810



AlifIntT alifMapping_getOptionalItem(AlifObject*, AlifObject*, AlifObject**); // 895




/* ------------------------------------------------------------------------------------- */





static inline AlifSizeT _alifVectorCall_nArgs(AlifUSizeT _n) { // 32
	return _n & ~ALIF_VECTORCALL_ARGUMENTS_OFFSET;
}
#define ALIFVECTORCALL_NARGS(_n) _alifVectorCall_nArgs(_n)


VectorCallFunc alifVectorCall_function(AlifObject*); // 39


AlifSizeT alifObject_lengthHint(AlifObject*, AlifSizeT); // 80
