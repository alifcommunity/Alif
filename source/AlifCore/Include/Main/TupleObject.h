#pragma once

extern AlifTypeObject _alifTupleType_; // 23

// 26
#define ALIFTUPLE_CHECK(_op) \
                 ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_TUPLE_SUBCLASS)
#define ALIFTUPLE_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifTupleType_)


AlifObject* alifTuple_new(AlifSizeT ); // 30

/* ------------------------------------------------------------------------------------------------------------*/

class AlifTupleObject{ // 5
public:
	ALIFOBJECT_VAR_HEAD;
	AlifObject* item[1]{};
};
