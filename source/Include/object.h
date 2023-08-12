#pragma once

#include "alif.h"
#include "alifMemory.h"

#if defined(ALIF_DEBUG) && !defined(ALIF_REF_DEBUG)
#  define ALIF_REF_DEBUG
#endif

#if defined(ALIF_LIMITED_API) && defined(ALIF_TRACE_REFS)
#  error ALIF_LIMITED_API is incompatible with ALIF_TRACE_REFS
#endif

#ifdef ALIF_TRACE_REFS
#define ALIFObject_HEAD_EXTRA            \
    AlifObject *obNext;           \
    AlifObject *obPrev;

#define ALIFOBJECT_EXTRA_INIT ALIF_NULL, ALIF_NULL,

#else
#  define ALIFOBJECT_HEAD_EXTRA
#  define ALIFOBJECT_EXTRA_INIT
#endif

#define ALIFOBJECT_HEAD                   AlifObject obBase;

#if SIZEOF_VOID_P > 4

#define ALIF_IMMORTAL_REFCNT UINT_MAX

#else

#define ALIF_IMMORTAL_REFCNT (UINT_MAX >> 2)
#endif

#ifdef ALIF_BUILD_CORE
#define ALIFOBJECT_HEAD_INIT(type)    \
    {                               \
        ALIFOBJECT_EXTRA_INIT        \
        { ALIF_IMMORTAL_REFCNT },    \
        (type)                      \
    },
#else
#define ALIFOBJECT_HEAD_INIT(type) \
    {                            \
        ALIFOBJECT_EXTRA_INIT     \
        { 1 },                   \
        (type)                   \
    },
#endif 

#define ALIFVAROBJECT_HEAD_INIT(type, size) \
    {                                     \
        ALIFOBJECT_HEAD_INIT(type)          \
        (size)                            \
    },

#define ALIFOBJECT_VAR_HEAD      AlifVarObject obBase;
#define ALIF_INVALID_SIZE (long long)-1

class Object {
public:
	ALIFOBJECT_HEAD_EXTRA

#if (defined(__GNUC__) || defined(__clang__)) \
        && !(defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L)
		__extension__
#endif
#ifdef _MSC_VER
		__pragma(warning(push))
		__pragma(warning(disable: 4201))
#endif
		union {
		long long obRefcnt;
#if SIZEOF_VOID_P > 4
		ALIF_UINT32_T obRefcntSplit[2];
#endif
	};
#ifdef _MSC_VER
	__pragma(warning(pop))
#endif

		AlifTypeObject* obType;
};

#define ALIFOBJECT_CAST(op) ALIF_CAST(AlifObject*, (op))

class AlifVarObject {
public:
	AlifObject obBase;
	long long obSize; 
};

#define ALIFVAROBJECT_CAST(op) ALIF_CAST(AlifVarObject*, (op))

ALIFAPI_FUNC(int) alif_is(AlifObject* x, AlifObject* y);
#define ALIF_IS(x, y) ((x) == (y))


inline long long alif_Refcnt(AlifObject* ob) {
	return ob->obRefcnt;
}
#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 < 0x030b0000
#  define ALIF_REFCNT(ob) Alif_Refcnt(ALIFOBJECT_CAST(ob))
#endif

AlifTypeObject* alif_type(AlifObject* ob) {
	return ob->obType;
}
#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 < 0x030b0000
#  define ALIF_TYPE(ob) alif_type(ALIFOBJECT_CAST(ob))
#endif

ALIFAPI_DATA(AlifTypeObject) alifLong_Type;
ALIFAPI_DATA(AlifTypeObject) alifBool_Type;

 inline long long alif_size(AlifObject* ob) {
	AlifVarObject* varOb = ALIFVAROBJECT_CAST(ob);
	return varOb->obSize;
}
#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 < 0x030b0000
#  define ALIF_SIZE(ob) ALIF_SIZE(ALIFOBJECT_CAST(ob))
#endif

 inline ALIF_ALWAYS_INLINE int alif_isImmortal(AlifObject* op)
{
#if SIZEOF_VOID_P > 4
	return ALIF_CAST(ALIF_INT32_T, op->obRefcnt) < 0;
#else
	return op->obRefcnt == ALIF_IMMORTAL_REFCNT;
#endif
}
#define AALIF_ISIMMORTAL(op) alif_isImmortal(ALIFOBJECT_CAST(op))

 inline int alif_isType(AlifObject* ob, AlifTypeObject* type) {
	return ALIF_TYPE(ob) == type;
}
#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 < 0x030b0000
#  define ALIF_IS_TYPE(ob, type) alif_isType(ALIFOBJECT_CAST(ob), (type))
#endif


 inline void alif_set_refcnt(AlifObject* ob, long long refcnt) {
	// This immortal check is for code that is unaware of immortal objects.
	// The runtime tracks these objects and we should avoid as much
	// as possible having extensions inadvertently change the refcnt
	// of an immortalized object.
	if (alif_isImmortal(ob)) {
		return;
	}
	ob->obRefcnt = refcnt;
}
#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 < 0x030b0000
#  define ALIF_SET_REFCNT(ob, refcnt) alif_set_refcnt(ALIFPBJECT_CAST(ob), (refcnt))
#endif


 inline void alif_set_type(AlifObject* ob, AlifTypeObject* type) {
	ob->obType = type;
}
#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 < 0x030b0000
#  define ALIF_SET_TYPE(ob, type) alif_set_refcnt(ALIFOBJECT_CAST(ob), type)
#endif

 inline void alif_set_size(AlifVarObject* ob, long long size) {

	ob->obSize = size;
}
#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 < 0x030b0000
#  define ALIF_SET_SIZE(ob, size) alif_set_refcnt(ALIFVAROBJECT_CAST(ob), (size))
#endif


#include "alif.h"

typedef void (*Destructor)(AlifObject*);
//typedef int (*GetBuffer)(AlifObject*, Buffer*, int);
//typedef void (*ReleaseBuffer)(AlifObject*, Buffer*);
typedef AlifObject* (*ReprFunc)(AlifObject*);
typedef AlifObject* (*GetAttrFunc)(AlifObject*, char*);
typedef AlifObject* (*SetAttrFunc)(AlifObject*, char*, AlifObject*);
typedef AlifObject* (*GetAttroFunc)(AlifObject*, AlifObject*);
typedef AlifObject* (*SetAttroFunc)(AlifObject*, AlifObject*);
typedef AlifObject* (*RichCompFunc)(AlifObject*, AlifObject*, int);
typedef int (*Inquiry)(AlifObject*);
typedef AlifObject* (*GetIterFunc)(AlifObject*);
typedef AlifObject* (*IterNextFunc)(AlifObject*);
typedef AlifObject* (*CFunctionFunc)(AlifObject*, AlifObject*);
typedef AlifObject* (*Getter)(AlifObject*, void*);
typedef AlifObject* (*Setteer)(AlifObject*, AlifObject*, void*);
typedef AlifObject* (*DescrGetFunc)(AlifObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*DescrSetFunc)(AlifObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*NewFunc)(AlifObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*InitProc)(AlifObject*, AlifObject*);
typedef AlifObject* (*AllocFunc)(AlifObject*, long long);
typedef void (*FreeFunc)(MemoryState*, void*);
typedef AlifObject* (*VectorCallFunc)(AlifObject* callable, AlifObject* const* args, size_t nargsf, AlifObject* kwnames);
typedef AlifObject* (*BinaryFunc)( AlifObject*, AlifObject*);
typedef AlifObject* (*TernaryFunc)( AlifObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*UnaryFunc)(AlifObject*);
typedef size_t(*HashFunc)(AlifObject*);
typedef size_t(*LenFunc)(AlifObject*);
typedef AlifObject* (*SizeArgFunc)(AlifObject*, size_t);
typedef AlifObject* (*SizeObjArgProcFunc)(AlifObject*, size_t, AlifObject*);
typedef AlifObject* (*ObjObjProcFunc)(AlifObject*, AlifObject*);
typedef AlifObject* (*ObjObjArgProcFunc)(AlifObject*, AlifObject*, AlifObject*);

class AlifNumberMethods {

	BinaryFunc nbAdd;
	BinaryFunc nbSubtract;
	BinaryFunc nbMultiply;
	BinaryFunc nbRemainder;
	BinaryFunc nbDivmod;
	TernaryFunc nbPower;
	UnaryFunc nbNegative;
	UnaryFunc nbPositive;
	UnaryFunc nbAbsolute;
	Inquiry nbBool;
	UnaryFunc nbInvert;
	BinaryFunc nbLshift;
	BinaryFunc nbRshift;
	BinaryFunc nbAnd;
	BinaryFunc nbXor;
	BinaryFunc nbOr;
	UnaryFunc nbInt;
	void* nbReserved;  /* the slot formerly known as nb_long */
	UnaryFunc nbFloat;

	BinaryFunc nbInplaceAdd;
	BinaryFunc nbInplaceSubtract;
	BinaryFunc nbInplaceMultiply;
	BinaryFunc nbInplaceRemainder;
	//ternaryfunc nbInplacePower;
	BinaryFunc nbInplaceLshift;
	BinaryFunc nbInplaceRshift;
	BinaryFunc nbInplaceAnd;
	BinaryFunc nbInplaceXor;
	BinaryFunc nbInplaceOr;

	BinaryFunc nbFloorDivide;
	BinaryFunc nbTrueDivide;
	BinaryFunc nbInplaceFloorDivide;
	BinaryFunc nbInplaceTrueDivide;

	UnaryFunc nbIndex;

	BinaryFunc nbMatrixMultiply;
	BinaryFunc nbInplaceMatrixMultiply;
};

class AlifSequenceMethods {
	LenFunc sqLength;
	BinaryFunc sqConcat;
	SizeArgFunc sqRepeat;
	SizeArgFunc sqItem;
	void* wasSqSlice;
	SizeObjArgProcFunc sqAssItem;
	void* wasSqAssSlice;
	ObjObjProcFunc sqContains;

	BinaryFunc sqInplaceConcat;
	SizeArgFunc sqInplaceRepeat;
} ;

class AlifMappingMethods {
	LenFunc mpLength;
	BinaryFunc mpSubscript;
	ObjObjArgProcFunc mpAssSubscript;
} ;

class TypeObject {
public:

	const char* typeName; 
	long long typeBasicSize, typeItemSize;

	Destructor typeDealloc;
	long long typeVectorCallOffset;
	GetAttrFunc typeGetAttr;
	SetAttrFunc typeSetAttr;
	//AsyncMethods* typeAsAsync;

	ReprFunc typeRepr;

	AlifNumberMethods* typeAsNumber;
	AlifSequenceMethods* typeAsSequence;
	AlifMappingMethods* typeAsMapping;

	HashFunc typeHash;
	TernaryFunc typeCall;
	ReprFunc tp_str;
	GetAttroFunc tp_getattro;
	SetAttroFunc tp_setattro;

	//BufferProcs* typeAsBuffer;

	unsigned long typeFlags;

	const char* typeDoc; 

	Inquiry typeClear;

	RichCompFunc typeRichCompare;

	long long typeweakListOffset;

	GetIterFunc typeIter;
	IterNextFunc typeIterNext;

	//PyMethodDef* typeMethods;
	//PyMemberDef* tyepMembers;
	//PyGetSetDef* typeGetSet;

	TypeObject* typeBase;
	AlifObject* typeDict;
	DescrGetFunc typeDescrGet;
	DescrSetFunc typeDescrSet;
	long long typeDictOffset;
	InitProc typeInit;
	AllocFunc typeAlloc;
	NewFunc typeNew;
	FreeFunc typeFree; 
	Inquiry typeIsGc; 
	AlifObject* typeBases;
	AlifObject* typeMro; 
	AlifObject* typeCache;
	void* typeSubClasses;  
	AlifObject* typeWeakList; 
	Destructor typeDel;

	unsigned int typeVersionTag;

	Destructor typeFinalize;
	VectorCallFunc typeVectorcall;

	char typeWatched;

};
