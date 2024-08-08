#pragma once

typedef AlifObject* (*AlifCFunction)(AlifObject*, AlifObject*);
typedef AlifObject* (*AlifCFunctionFast) (AlifObject*, AlifObject* const*, int64_t);
typedef AlifObject* (*AlifCFunctionWithKeywords)(AlifObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*AlifCFunctionFastWithKeywords) (AlifObject*, AlifObject* const*, int64_t, AlifObject*);
typedef AlifObject* (*AlifCMethod)(AlifObject*, AlifInitObject*, AlifObject* const*, size_t, AlifObject*);

class AlifMethodDef{
public:
    const wchar_t* name{};
    AlifCFunction method{};
    int flags{};
};

extern AlifInitObject _alifCppMethodType_;
extern AlifInitObject _alifCppFunctionType_;

#define ALIFCPPFUNCTION_CHECKEXACT(op) ALIF_IS_TYPE((op), &_alifCppFunctionType_)
#define ALIFCPPFUNCTION_CHECK(op) ALIFOBJECT_TYPECHECK((op), &_alifCppFunctionType_)

#define ALIFCFUNCTION_NEWEX(_ml, _self, _mod) alifNew_cMethod((_ml), (_self), (_mod), nullptr)
AlifObject* alifNew_cMethod(AlifMethodDef*, AlifObject*, AlifObject*, AlifInitObject*);
AlifObject* alifNew_cFunction(AlifMethodDef*, AlifObject*);


#define METHOD_VARARGS  0x0001
#define METHOD_KEYWORDS 0x0002
#define METHOD_NOARGS   0x0004
#define METHOD_O        0x0008
#define METHOD_CLASS    0x0010
#define METHOD_STATIC   0x0020
#define METHOD_COEXIST   0x0040
#  define METHOD_FASTCALL  0x0080
#ifdef STACKLESS
#  define METHOD_STACKLESS 0x0100
#else
#  define METHOD_STACKLESS 0x0000
#endif
#define METHOD_METHOD 0x0200

#define ALIF_CPPFUNCTION_CAST(func) ((AlifCFunction)((void(*)(void))(func)))


class AlifCFunctionObject {
public:
	ALIFOBJECT_HEAD;
	AlifMethodDef* method{};
	AlifObject* self{};
	AlifObject* module{};
	AlifObject* weakRefList{};
	VectorCallFunc vectorCall{};
};

class AlifCMethodObject {
public:
    AlifCFunctionObject func;
    AlifInitObject* mMClass; 
};
