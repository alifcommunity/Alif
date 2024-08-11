#include "alif.h"

#include "AlifCore_Call.h"   
#include "AlifCore_AlifEval.h" 
#include "AlifCore_Object.h"
#include "AlifCore_AlifState.h" 








AlifTypeObject _alifMethodType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"method",
	sizeof(AlifMethodObject),
	0,
	0,
	offsetof(AlifMethodObject, vectorCall),
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0, // alifVectorCall_Call,
	0, //method_getAttro,
	alifObject_genericSetAttr,
	0,
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC | ALIFTPFLAGS_HAVE_VECTORCALL,
	0,
	0, // (traverseproc)method_traverse,
	0, // method_richcompare,
	0,
	offsetof(AlifMethodObject, weakRefList),
};
