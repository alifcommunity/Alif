#pragma once









#define ALIFTUPLE_ITEMS(_op) ALIF_RVALUE(ALIFTUPLE_CAST(_op)->item) // 21


extern AlifObject* alifTuple_fromArray(AlifObject* const*, AlifSizeT); // 23
AlifObject* alifTuple_fromStackRefSteal(const AlifStackRef* , AlifSizeT); // 24


class AlifTupleIterObject { // 27
public:
	ALIFOBJECT_HEAD{};
	AlifSizeT index{};
	AlifTupleObject* seq{};
};
