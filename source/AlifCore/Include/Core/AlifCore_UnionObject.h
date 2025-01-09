#pragma once






extern AlifTypeObject _alifUnionType_; // 12

#define ALIFUNION_CHECK(_op) ALIF_IS_TYPE(_op, &_alifUnionType_) // 15


extern AlifObject* _alifUnion_args(AlifObject*); // 20
