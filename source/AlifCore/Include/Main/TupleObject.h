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




// 16
#define ALIFTUPLE_CAST(_op) ALIF_CAST(AlifTupleObject*, (_op))


static inline AlifSizeT alifTuple_getSize(AlifObject* _op) { // 21
	AlifTupleObject* tuple = ALIFTUPLE_CAST(_op);
	return ALIF_SIZE(tuple);
}
#define ALIFTUPLE_GET_SIZE(_op) alifTuple_getSize(ALIFOBJECT_CAST(_op))

#define ALIFTUPLE_GET_ITEM(_op, _index) (ALIFTUPLE_CAST(_op)->item[(_index)]) // 27
