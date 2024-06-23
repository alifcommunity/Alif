#pragma once

#define ALIFTUPLE_ITEMS(object) ALIF_RVALUE(((AlifTupleObject*)(object))->items_)

AlifObject* alifSubTuple_fromArray(AlifObject* const* , size_t );
AlifObject* alifTuple_fromArraySteal(AlifObject* const*, AlifSizeT);
