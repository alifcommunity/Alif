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

#define ALIFMETHOD_CHECK(_op) ALIF_IS_TYPE((_op), &_alifMethodType_) // 22

AlifObject* alifMethod_new(AlifObject*, AlifObject*); // 24

// 29
#define ALIFMETHOD_CAST(_meth) \
    ALIF_CAST(AlifMethodObject*, _meth)


static inline AlifObject* _alifMethod_getFunction(AlifObject* _meth) { // 34
	return ALIFMETHOD_CAST(_meth)->func;
}
#define ALIFMETHOD_GET_FUNCTION(_meth) _alifMethod_getFunction(ALIFOBJECT_CAST(_meth))

static inline AlifObject* _alifMethod_getSelf(AlifObject* _meth) { // 39
	return ALIFMETHOD_CAST(_meth)->self;
}
#define ALIFMETHOD_GET_SELF(_meth) _alifMethod_getSelf(ALIFOBJECT_CAST(_meth))
