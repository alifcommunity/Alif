#pragma once


extern AlifTypeObject _alifCPPFunctionType_; // 14


typedef AlifObject* (*AlifCPPFunction)(AlifObject*, AlifObject*); // 19

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



class AlifCPPMethodObject { // 23
public:
	AlifCPPFunctionObject func{};
	AlifTypeObject* class_{};
};


extern AlifTypeObject _alifCPPMethodType_; // 32

#define ALIFCPPMETHOD_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifCPPMethodType_) // 34
#define ALIFCPPMETHOD_CHECK(_op) ALIFOBJECT_TYPECHECK((_op), &_alifCPPMethodType_)
