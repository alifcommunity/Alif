#pragma once


extern AlifTypeObject _alifCPPFunctionType_; // 14


typedef AlifObject* (*AlifCPPFunction)(AlifObject*, AlifObject*); // 19
typedef AlifObject* (*AlifCPPFunctionWithKeywords)(AlifObject*, AlifObject*, AlifObject*); // 21
typedef AlifObject* (*AlifCPPFunctionFastWithKeywords)(AlifObject*, AlifObject* const*, AlifSizeT, AlifObject*); // 23

// 52
#define ALIF_CPPFUNCTION_CAST(_func) \
    ALIF_CAST(AlifCPPFunction, ALIF_CAST(void(*)(void), (_func)))


class AlifMethodDef { // 59
public:
	const char* name{};
	AlifCPPFunction method{};
	AlifIntT flags{};
};


// 78
#define ALIFCPPFUNCTION_NEWEX(_ml, _self, _mod) alifCPPMethod_new((_ml), (_self), (_mod), nullptr)
AlifObject* alifCPPMethod_new(AlifMethodDef*, AlifObject*, AlifObject*, AlifTypeObject*);



#define METHOD_VARARGS  0x0001 // 86
#define METHOD_KEYWORDS 0x0002 // 87

#define METHOD_NOARGS   0x0004 // 89
#define METHOD_O        0x0008 // 90

#define METHOD_CLASS    0x0010 // 95
#define METHOD_STATIC   0x0020


#define METHOD_FASTCALL 0x0080 // 106


#define METHOD_METHOD 0x0200 // 124





/* ------------------------------------------------------------------------------------------------------- */




class AlifCPPFunctionObject { // 7
public:
	ALIFOBJECT_HEAD;
	AlifMethodDef* ml_{};
	AlifObject* self{};
	AlifObject* module_{};
	AlifObject* weakRefList{};
	VectorCallFunc vectorCall{};
};

 // 16
#define ALIFCPPFUNCTIONOBJECT_CAST(_func) ALIF_CAST(AlifCPPFunctionObject*, (_func))


class AlifCPPMethodObject { // 23
public:
	AlifCPPFunctionObject func{};
	AlifTypeObject* class_{};
};


extern AlifTypeObject _alifCPPMethodType_; // 32

#define ALIFCPPMETHOD_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifCPPMethodType_) // 34
#define ALIFCPPMETHOD_CHECK(_op) ALIFOBJECT_TYPECHECK((_op), &_alifCPPMethodType_)



static inline AlifCPPFunction alifCPPFunction_getFunction(AlifObject* _func) { // 40
	return ALIFCPPFUNCTIONOBJECT_CAST(_func)->ml_->method;
}
#define ALIFCPPFUNCTION_GET_FUNCTION(_func) alifCPPFunction_getFunction(ALIFOBJECT_CAST(_func))



static inline AlifObject* alifCPPFunction_getSelf(AlifObject* _funcObj) { // 45
	AlifCPPFunctionObject* func = ALIFCPPFUNCTIONOBJECT_CAST(_funcObj);
	if (func->ml_->flags & METHOD_STATIC) {
		return nullptr;
	}
	return func->self;
}
#define ALIFCPPFUNCTION_GET_SELF(_func) alifCPPFunction_getSelf(ALIFOBJECT_CAST(_func))
