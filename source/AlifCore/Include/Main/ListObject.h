#pragma once

extern AlifTypeObject _alfiListType_; // 20


AlifObject* alifList_New(AlifSizeT ); // 28
// ------------------------------------------------------------------------------------------- //



class AlifListObject { //  5
public:
	ALIFOBJECT_VAR_HEAD;
		AlifObject** item;

	AlifSizeT allocated;
} ;
