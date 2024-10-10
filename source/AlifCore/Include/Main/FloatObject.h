#pragma once



extern AlifTypeObject _alifFloatType_; // 14


AlifObject* alifFloat_fromDouble(double); // 39







/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

class AlifFloatObject { // 5
public:
	ALIFOBJECT_HEAD;
	double val{};
} ;
