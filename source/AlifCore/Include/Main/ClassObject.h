#pragma once







class AlifMethodObject { // 12
public:
	ALIFOBJECT_HEAD{};
	AlifObject* func{};   /* The callable object implementing the method */
	AlifObject* self{};   /* The instance it is bound to */
	AlifObject* weakRefList{}; /* List of weak references */
	VectorCallFunc vectorCall{};
};


extern AlifTypeObject _alifMethodType_; // 20
