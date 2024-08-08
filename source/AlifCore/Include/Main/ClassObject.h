#pragma once




class AlifMethodObject {
public:
	ALIFOBJECT_HEAD
	AlifObject* func; 
	AlifObject* self; 
	AlifObject* weakRefList;
	VectorCallFunc vectorCall;
};

extern AlifTypeObject _alifMethodType_;
