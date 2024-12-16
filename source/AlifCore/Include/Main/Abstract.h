#pragma once


 // 276
#define ALIF_VECTORCALL_ARGUMENTS_OFFSET \
    (ALIF_STATIC_CAST(AlifUSizeT, 1) << (8 * sizeof(AlifUSizeT) - 1))

AlifSizeT alifObject_size(AlifObject*); // 328


#undef ALIFOBJECT_LENGTH
AlifSizeT alifObject_length(AlifObject*); // 342
#define ALIFOBJECT_LENGTH alifObject_size


AlifIntT alifObject_setItem(AlifObject*, AlifObject*, AlifObject*); // 357

AlifObject* alifObject_format(AlifObject*, AlifObject*); // 374

AlifObject* alifObject_getIter(AlifObject*); // 383

AlifIntT alifIter_check(AlifObject*); // 393

AlifObject* alifIter_next(AlifObject* ); // 417


AlifObject* alifNumber_add(AlifObject*, AlifObject*); // 443
AlifObject* alifNumber_subtract(AlifObject*, AlifObject*); // 448

AlifObject* alifNumber_multiply(AlifObject*, AlifObject*); // 453
AlifObject* alifNumber_floorDivide(AlifObject*, AlifObject*); // 464
AlifObject* alifNumber_trueDivide(AlifObject*, AlifObject*); // 470
AlifObject* alifNumber_remainder(AlifObject*, AlifObject*); // 475
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


AlifObject* alifNumber_inPlaceSubtract(AlifObject*, AlifObject*); // 580
AlifObject* alifNumber_inPlaceFloorDivide(AlifObject*, AlifObject*); // 597
AlifObject* alifNumber_inPlaceTrueDivide(AlifObject*, AlifObject*); // 604
AlifObject* alifNumber_inPlaceRemainder(AlifObject*, AlifObject*); // 611
AlifObject* alifNumber_inPlaceLshift(AlifObject*, AlifObject*); // 625
AlifObject* alifNumber_inPlaceRshift(AlifObject*, AlifObject*); // 631
AlifObject* alifNumber_inPlaceAnd(AlifObject*, AlifObject*); // 637
AlifObject* alifNumber_inPlaceXor(AlifObject*, AlifObject*); // 643
AlifObject* alifNumber_inPlaceOr(AlifObject*, AlifObject*); // 649


AlifIntT alifSequence_check(AlifObject*); // 664

AlifObject* alifSequence_getItem(AlifObject*, AlifSizeT); // 689

AlifIntT alifSequence_setItem(AlifObject*, AlifSizeT, AlifObject*); // 700

AlifIntT alifSequence_delItem(AlifObject*, AlifSizeT); // 705

AlifObject* alifSequence_tuple(AlifObject*); // 723

AlifObject* alifSequence_list(AlifObject*); // 727

AlifObject* alifSequence_fast(AlifObject*, const char*); // 736


 // 740
#define ALIFSEQUENCE_FAST_GET_SIZE(o) \
    (ALIFLIST_CHECK(o) ? ALIFLIST_GET_SIZE(o) : ALIFTUPLE_GET_SIZE(o))


 // 750
#define ALIFSEQUENCE_FAST_ITEMS(sf) \
    (ALIFLIST_CHECK(sf) ? ((AlifListObject *)(sf))->item \
                      : ((AlifTupleObject *)(sf))->item)

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
