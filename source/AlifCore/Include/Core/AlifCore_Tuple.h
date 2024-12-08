#pragma once









#define ALIFTUPLE_ITEMS(_op) ALIF_RVALUE(ALIFTUPLE_CAST(_op)->item) // 21


extern AlifObject* alifTuple_fromArray(AlifObject* const*, AlifSizeT); // 23
