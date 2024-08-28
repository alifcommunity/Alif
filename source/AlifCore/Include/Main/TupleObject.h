#pragma once

extern AlifTypeObject _alifTupleType_; // 23


AlifObject* alifTuple_new(AlifSizeT ); //30

/* ------------------------------------------------------------------------------------------------------------*/

class AlifTupleObject{ // 5
public:
	ALIFOBJECT_VAR_HEAD;
	AlifObject* item[1]{};
};
