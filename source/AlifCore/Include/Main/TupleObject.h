#pragma once

extern AlifTypeObject _alifTupleType_; // 23

// 26
#define ALIFTUPLE_CHECK(_op) \
                 ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_TUPLE_SUBCLASS)
#define ALIFTUPLE_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifTupleType_)


AlifObject* alifTuple_new(AlifSizeT ); // 30
AlifSizeT alifTuple_size(AlifObject*); // 31
AlifObject* alifTuple_getItem(AlifObject*, AlifSizeT); // 32
AlifObject* alifTuple_pack(AlifSizeT , ...); // 35


/* ------------------------------------------------------------------------------------------------------------*/

class AlifTupleObject{ // 5
public:
	ALIFOBJECT_VAR_HEAD;
	AlifObject* item[1]{};
};


AlifIntT alifTuple_resize(AlifObject** , AlifSizeT ); // 13

// 16
#define ALIFTUPLE_CAST(_op) ALIF_CAST(AlifTupleObject*, (_op))


static inline AlifSizeT alifTuple_getSize(AlifObject* _op) { // 21
	AlifTupleObject* tuple = ALIFTUPLE_CAST(_op);
	return ALIF_SIZE(tuple);
}
#define ALIFTUPLE_GET_SIZE(_op) alifTuple_getSize(ALIFOBJECT_CAST(_op))

#define ALIFTUPLE_GET_ITEM(_op, _index) (ALIFTUPLE_CAST(_op)->item[(_index)]) // 27

static inline void alifTuple_setItem(AlifObject* _op, AlifSizeT _index, AlifObject* _value) { // 31
	AlifTupleObject* tuple = ALIFTUPLE_CAST(_op);
	tuple->item[_index] = _value;
}
#define ALIFTUPLE_SET_ITEM(_op, _index, _value) \
    alifTuple_setItem(ALIFOBJECT_CAST(_op), (_index), ALIFOBJECT_CAST(_value))
