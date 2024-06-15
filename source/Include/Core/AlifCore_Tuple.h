#pragma once
// in file alifMacro.h line 165
#define ALIF_RVALUE(EXPR) ((void)0, (EXPR))

#define ALIFTUPLE_ITEMS(object) ALIF_RVALUE(((AlifTupleObject*)(object))->items_)

AlifObject* alifSubTuple_fromArray(AlifObject* const* , size_t );
