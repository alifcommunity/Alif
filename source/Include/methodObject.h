#pragma once
#include "alif.h"

typedef AlifObject* (*AlifCFunctionFunc)(AlifObject*, AlifObject*);

ALIFAPI_FUNC(AlifCFunctionFunc)AlifCFunction_getFunction(AlifObject* op);

class AlifMethodDef {
public:
	const char* mlName;
	AlifCFunctionFunc mlMeth;
	int mlFlags;
	const char* mlDoc;
};

#define ALIFCFUNCTION_CAST(func) ALIF_CAST(AlifCFunctionFunc, ALIF_CAST(void(*)(void), (func)))

class AlifCFunctionObject {
public:
	ALIFOBJECT_HEAD
	AlifMethodDef* mMl; /* Description of the C function to call */
	AlifObject* mSelf; /* Passed as 'self' arg to the C func, can be NULL */
	AlifObject* mModule; /* The __module__ attribute, can be anything */
	AlifObject* mWeakRefList; /* List of weak references */
	VectorCallFunc vectorCall;
};

#define ALIFCFUNCTIONOBJECT_CAST(func) ALIF_CAST(AlifCFunctionObject*, (func))

inline AlifCFunctionFunc alifCFunction_get_function(AlifObject* func) {
	return ALIFCFUNCTIONOBJECT_CAST(func)->mMl->mlMeth;
}
#define ALIFCFUNCTION_GET_FUNCTION(func) alifCFunction_get_function(ALIFOBJECT_CAST(func))
