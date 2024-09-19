#pragma once

extern AlifTypeObject _alifListType_; // 20


AlifObject* alifList_new(AlifSizeT); // 28



/* -------------------------------------------------------------------------------------- */



class AlifListObject { //  5
public:
	ALIFOBJECT_VAR_HEAD{};
	AlifObject** item{};

	AlifSizeT allocated{};
} ;
