#pragma once

extern AlifInitObject _alifTupleType_;

AlifObject* alifNew_tuple(AlifSizeT );
AlifObject* alifTuple_getItem(AlifObject* , int64_t);
AlifObject* alifTuple_setItem(AlifObject* , int64_t);
AlifObject* alifTuple_getSlice(AlifObject*, int64_t, int64_t);
AlifObject* tuple_pack(size_t, ...);


class AlifTupleObject {
public:
	ALIFOBJECT_VAR_HEAD;

	AlifObject* items_[1]{};
};


#define ALIFTUPLE_CAST(_op) ALIF_CAST(AlifTupleObject*, (_op))

#define ALIFTUPLE_CHECK(_op) \
					 ALIF_TYPE(_op)
                 //ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIFTPFLAGS_TUPLE_SUBCLASS)
#define ALIFTUPLE_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifTupleType_)


static inline AlifSizeT alifTuple_getSize(AlifObject* _op) {
	AlifTupleObject* tuple = ALIFTUPLE_CAST(_op);
	return ALIF_SIZE(tuple);
}
#define ALIFTUPLE_GET_SIZE(_op) alifTuple_getSize(ALIFOBJECT_CAST(_op))

#define ALIFTUPLE_GET_ITEM(op, index) (ALIF_CAST(AlifTupleObject*, (op))->items_[(index)])

static inline void alifTuple_setItem(AlifObject* _op, int64_t _index, AlifObject* _value) {
	AlifTupleObject* tuple_ = (AlifTupleObject*)(_op);
	tuple_->items_[_index] = _value;
}
#define ALIFTUPLE_SET_ITEM(_op, _index, _value) \
    alifTuple_setItem(ALIFOBJECT_CAST(_op), (_index), ALIFOBJECT_CAST(_value))
