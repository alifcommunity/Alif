#pragma once







ALIFAPI_FUNC(AlifObject*) alifMarshal_readObjectFromString(const char*, AlifSizeT); // 12
ALIFAPI_FUNC(AlifObject*) alifMarshal_writeObjectToString(AlifObject*, AlifIntT); // 14


#define ALIF_MARSHAL_VERSION 5
