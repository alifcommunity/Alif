#pragma once

class AlifTupleObject {
public:

	ALIFOBJECT_VAR_HEAD

     AlifObject* items_[1];
};

extern AlifInitObject _alifTupleType_;

#define ALIFTUPLE_CHECK(_op) \
					 ALIF_TYPE(_op)
                 //ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIFTPFLAGS_TUPLE_SUBCLASS)
#define ALIFTUPLE_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifTupleType_)

AlifObject* alifNew_tuple(SSIZE_T );
AlifObject* tuple_pack(size_t, ...);


#define ALIFTUPLE_GET_ITEM(op, index) (ALIF_CAST(AlifTupleObject*, (op))->items_[(index)])

static inline void alifTuple_set_item(AlifObject* _op, int64_t _index, AlifObject* _value) {
	AlifTupleObject* tuple_ = (AlifTupleObject*)(_op);
	tuple_->items_[_index] = _value;
}
#define ALIFTUPLE_SET_ITEM(_op, _index, _value) \
    alifTuple_set_item(ALIFSUBOBJECT_CAST(_op), (_index), ALIFSUBOBJECT_CAST(_value))
