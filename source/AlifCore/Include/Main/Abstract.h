#pragma once


ALIFAPI_FUNC(AlifObject*) alifObject_callOneArg(AlifObject*, AlifObject*); // 59

ALIFAPI_FUNC(AlifObject*) alifObject_callNoArgs(AlifObject*); // 189

ALIFAPI_FUNC(AlifObject*) alifObject_call(AlifObject*, AlifObject*, AlifObject*); // 201

ALIFAPI_FUNC(AlifObject*) alifObject_callObject(AlifObject*, AlifObject*); // 212

ALIFAPI_FUNC(AlifObject*) alifObject_callFunction(AlifObject*, const char*, ...); // 225

ALIFAPI_FUNC(AlifObject*) alifObject_callMethod(AlifObject*, const char*, const char*, ...); // 237

ALIFAPI_FUNC(AlifObject*) alifObject_callFunctionObjArgs(AlifObject* _callable, ...); // 249

ALIFAPI_FUNC(AlifObject*) alifObject_callMethodObjArgs(AlifObject*, AlifObject*, ...); // 260

ALIFAPI_FUNC(AlifObject*) alifVectorCall_call(AlifObject*, AlifObject*, AlifObject*); // 273

// 276
#define ALIF_VECTORCALL_ARGUMENTS_OFFSET \
    (ALIF_STATIC_CAST(AlifUSizeT, 1) << (8 * sizeof(AlifUSizeT) - 1))

ALIFAPI_FUNC(AlifObject*) alifObject_vectorCall(AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*); // 280


ALIFAPI_FUNC(AlifObject*) alifObject_vectorCallMethod(AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*); // 287


ALIFAPI_FUNC(AlifObject*) alifObject_type(AlifObject*); // 328

ALIFAPI_FUNC(AlifSizeT) alifObject_size(AlifObject*); // 337


#undef ALIFOBJECT_LENGTH
ALIFAPI_FUNC(AlifSizeT) alifObject_length(AlifObject*); // 342
#define ALIFOBJECT_LENGTH alifObject_size


ALIFAPI_FUNC(AlifObject*) alifObject_getItem(AlifObject*, AlifObject*); // 349


ALIFAPI_FUNC(AlifIntT) alifObject_setItem(AlifObject*, AlifObject*, AlifObject*); // 357

ALIFAPI_FUNC(AlifIntT) alifObject_delItem(AlifObject*, AlifObject*); // 369

ALIFAPI_FUNC(AlifObject*) alifObject_format(AlifObject*, AlifObject*); // 374

ALIFAPI_FUNC(AlifObject*) alifObject_getIter(AlifObject*); // 383

ALIFAPI_FUNC(AlifIntT) alifIter_check(AlifObject*); // 393

ALIFAPI_FUNC(AlifObject*) alifIter_next(AlifObject*); // 417


ALIFAPI_FUNC(AlifIntT) alifNumber_check(AlifObject*); // 438

ALIFAPI_FUNC(AlifObject*) alifNumber_add(AlifObject*, AlifObject*); // 443
ALIFAPI_FUNC(AlifObject*) alifNumber_subtract(AlifObject*, AlifObject*); // 448

ALIFAPI_FUNC(AlifObject*) alifNumber_multiply(AlifObject*, AlifObject*); // 453
ALIFAPI_FUNC(AlifObject*) alifNumber_floorDivide(AlifObject*, AlifObject*); // 464
ALIFAPI_FUNC(AlifObject*) alifNumber_trueDivide(AlifObject*, AlifObject*); // 470
ALIFAPI_FUNC(AlifObject*) alifNumber_remainder(AlifObject*, AlifObject*); // 475
ALIFAPI_FUNC(AlifObject*) alifNumber_divmod(AlifObject*, AlifObject*); // 482
ALIFAPI_FUNC(AlifObject*) alifNumber_power(AlifObject*, AlifObject*, AlifObject*); // 488


// this funcs in Abstract.cpp alter-line: 1361
ALIFAPI_FUNC(AlifObject*) alifNumber_negative(AlifObject*); // 494
ALIFAPI_FUNC(AlifObject*) alifNumber_positive(AlifObject*); // 499
ALIFAPI_FUNC(AlifObject*) alifNumber_absolute(AlifObject*); // 504
ALIFAPI_FUNC(AlifObject*) alifNumber_invert(AlifObject*); // 509
ALIFAPI_FUNC(AlifObject*) alifNumber_lshift(AlifObject*, AlifObject*); // 514
ALIFAPI_FUNC(AlifObject*) alifNumber_rshift(AlifObject*, AlifObject*); // 520
ALIFAPI_FUNC(AlifObject*) alifNumber_and(AlifObject*, AlifObject*); // 526
ALIFAPI_FUNC(AlifObject*) alifNumber_xor(AlifObject*, AlifObject*); // 531
ALIFAPI_FUNC(AlifObject*) alifNumber_or(AlifObject*, AlifObject*); // 537

ALIFAPI_FUNC(AlifIntT) alifIndex_check(AlifObject*); // 541
ALIFAPI_FUNC(AlifObject*) alifNumber_index(AlifObject*); // 545

ALIFAPI_FUNC(AlifSizeT) alifNumber_asSizeT(AlifObject*, AlifObject*); // 553

ALIFAPI_FUNC(AlifObject*) alifNumber_float(AlifObject* o); // 565

ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceAdd(AlifObject*, AlifObject*); // 574
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceSubtract(AlifObject*, AlifObject*); // 580
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceMultiply(AlifObject*, AlifObject*); // 586
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceFloorDivide(AlifObject*, AlifObject*); // 597
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceTrueDivide(AlifObject*, AlifObject*); // 604
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceRemainder(AlifObject*, AlifObject*); // 611
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceLshift(AlifObject*, AlifObject*); // 625
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceRshift(AlifObject*, AlifObject*); // 631
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceAnd(AlifObject*, AlifObject*); // 637
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceXor(AlifObject*, AlifObject*); // 643
ALIFAPI_FUNC(AlifObject*) alifNumber_inPlaceOr(AlifObject*, AlifObject*); // 649

ALIFAPI_FUNC(AlifObject*) alifNumber_toBase(AlifObject*, AlifIntT, AlifIntT); // 655

ALIFAPI_FUNC(AlifIntT) alifSequence_check(AlifObject*); // 664

ALIFAPI_FUNC(AlifSizeT) alifSequence_size(AlifObject*); // 667

ALIFAPI_FUNC(AlifObject*) alifSequence_getItem(AlifObject*, AlifSizeT); // 689

ALIFAPI_FUNC(AlifIntT) alifSequence_setItem(AlifObject*, AlifSizeT, AlifObject*); // 700

ALIFAPI_FUNC(AlifIntT) alifSequence_delItem(AlifObject*, AlifSizeT); // 705

ALIFAPI_FUNC(AlifObject*) alifSequence_tuple(AlifObject*); // 723

ALIFAPI_FUNC(AlifObject*) alifSequence_list(AlifObject*); // 727

ALIFAPI_FUNC(AlifObject*) alifSequence_fast(AlifObject*, const char*); // 736


// 740
#define ALIFSEQUENCE_FAST_GET_SIZE(o) \
    (ALIFLIST_CHECK(o) ? ALIFLIST_GET_SIZE(o) : ALIFTUPLE_GET_SIZE(o))


 // 750
#define ALIFSEQUENCE_FAST_ITEMS(sf) \
    (ALIFLIST_CHECK(sf) ? ((AlifListObject *)(sf))->item \
                      : ((AlifTupleObject *)(sf))->item)


ALIFAPI_FUNC(AlifIntT) alifSequence_contains(AlifObject*, AlifObject*); // 765


ALIFAPI_FUNC(AlifIntT) alifMapping_check(AlifObject*); // 806

ALIFAPI_FUNC(AlifSizeT) alifMapping_size(AlifObject*); // 810

#define ALIFMAPPING_DELITEM(_o, _k) alifObject_delItem(_o, _k) // 836

ALIFAPI_FUNC(AlifObject*) alifMapping_keys(AlifObject*); // 867

ALIFAPI_FUNC(AlifIntT) alifMapping_getOptionalItem(AlifObject*, AlifObject*, AlifObject**); // 895

ALIFAPI_FUNC(AlifIntT) alifMapping_setItemString(AlifObject*, const char*, AlifObject*); // 903

ALIFAPI_FUNC(AlifIntT) alifObject_isInstance(AlifObject*, AlifObject*); // 907

ALIFAPI_FUNC(AlifIntT) alifObject_isSubclass(AlifObject*, AlifObject*); // 910

/* ------------------------------------------------------------------------------------- */



ALIFAPI_FUNC(AlifObject*) _alifStack_asDict(AlifObject* const*, AlifObject*); // 24

static inline AlifSizeT _alifVectorCall_nArgs(AlifUSizeT _n) { // 32
	return _n & ~ALIF_VECTORCALL_ARGUMENTS_OFFSET;
}
#define ALIFVECTORCALL_NARGS(_n) _alifVectorCall_nArgs(_n)


ALIFAPI_FUNC(VectorCallFunc) alifVectorCall_function(AlifObject*); // 39

ALIFAPI_FUNC(AlifObject*) alifObject_vectorCallDict(AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*); // 53


static inline AlifObject* alifObject_callMethodNoArgs(AlifObject* _self, AlifObject* _name) { // 61
	AlifUSizeT nargsf = 1 | ALIF_VECTORCALL_ARGUMENTS_OFFSET;
	return alifObject_vectorCallMethod(_name, &_self, nargsf, nullptr);
}

static inline AlifObject* alifObject_callMethodOneArg(AlifObject* _self,
	AlifObject* _name, AlifObject* _arg) { // 68
	AlifObject* args[2] = { _self, _arg };
	AlifUSizeT nargsf = 2 | ALIF_VECTORCALL_ARGUMENTS_OFFSET;
	return alifObject_vectorCallMethod(_name, args, nargsf, nullptr);
}


ALIFAPI_FUNC(AlifSizeT) alifObject_lengthHint(AlifObject*, AlifSizeT); // 80
