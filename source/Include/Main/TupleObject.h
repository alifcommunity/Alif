#pragma once

class AlifTupleObject {
public:
    AlifVarObject object{};

     AlifObject* items[1];
};

extern AlifInitObject typeTuple;

#define ALIFTUPLE_CHECK(_op) \
                 ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIFTPFLAGS_TUPLE_SUBCLASS)
#define PyTuple_CheckExact(_op) ALIF_IS_TYPE((_op), &typeTuple)

AlifObject* alifNew_tuple(SSIZE_T );
AlifObject* tuple_pack(size_t, ...);


#define ALIFTUPLE_GET_ITEM(op, index) (ALIF_CAST(AlifTupleObject*, (op))->items[(index)])
