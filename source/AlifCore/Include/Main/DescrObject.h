#pragma once



typedef AlifObject* (*Getter)(AlifObject*, void*); // 8
typedef AlifIntT (*Setter)(AlifObject*, AlifObject*, void*);

class AlifGetSetDef { // 11
public:
	const char* name{};
	Getter get{};
	Setter set{};
	const char* doc{};
	void* closure{};
};


extern AlifTypeObject _alifClassMethodDescrType_; // 19
extern AlifTypeObject _alifGetSetDescrType_;
extern AlifTypeObject _alifMemberDescrType_; // 21
extern AlifTypeObject _alifWrapperDescrType_; // 23

AlifObject* alifDescr_newMethod(AlifTypeObject*, AlifMethodDef*); // 27
AlifObject* alifDescr_newClassMethod(AlifTypeObject*, AlifMethodDef*); // 28
AlifObject* alifDescr_newMember(AlifTypeObject*, AlifMemberDef*); // 29
AlifObject* alifDescr_newGetSet(AlifTypeObject*, AlifGetSetDef*); // 30

class AlifMemberDef { // 41
public:
	const char* name{};
	AlifIntT type{};
	AlifSizeT offset{};
	AlifIntT flags{};
};

// 52
/* Types */
#define ALIF_T_SHORT     0
#define ALIF_T_INT       1
#define ALIF_T_LONG      2
#define ALIF_T_FLOAT     3
#define ALIF_T_DOUBLE    4
#define ALIF_T_STRING    5
#define _ALIF_T_OBJECT   6  // Deprecated, use ALIF_T_OBJECT_EX instead
/* the ordering here is weird for binary compatibility */
#define ALIF_T_CHAR      7   /* 1-character string */
#define ALIF_T_BYTE      8   /* 8-bit signed int */
/* unsigned variants: */
#define ALIF_T_UBYTE     9
#define ALIF_T_USHORT    10
#define ALIF_T_UINT      11
#define ALIF_T_ULONG     12

#define ALIF_T_STRING_INPLACE    13

/* Added by Lillo: bools contained in the structure (assumed char) */
#define ALIF_T_BOOL      14

#define ALIF_T_OBJECT_EX 16
#define ALIF_T_LONGLONG  17
#define ALIF_T_ULONGLONG 18

#define ALIF_T_ALIFSIZET  19      /* AlifSizeT */
#define _ALIF_T_NONE     20 // Deprecated. Value is always None.


#define ALIF_READONLY	1 // 83
#define ALIF_AUDIT_READ   2 // 84

#define ALIF_RELATIVE_OFFSET  8 //86







/* ------------------------------------------------------------------------------------ */


typedef AlifObject* (*WrapperFunc)(AlifObject*, AlifObject*, void*); // 5


class WrapperBase { // 11
public:
	const char* name{};
	AlifIntT offset{};
	void* function{};
	WrapperFunc wrapper{};
	const char* doc{};
	AlifIntT flags{};
	AlifObject* nameStrObj{};
};

 // 21
/* Flags for above struct */
#define ALIFWRAPPERFLAG_KEYWORDS 1 /* wrapper function takes keyword args */


class AlifDescrObject { // 26
public:
	ALIFOBJECT_HEAD{};
	AlifTypeObject* type{};
	AlifObject* name{};
	AlifObject* qualname{};
};


#define ALIFDESCR_COMMON AlifDescrObject common // 33

#define ALIFDESCR_TYPE(_x) (((AlifDescrObject *)(_x))->type)
#define ALIFDESCR_NAME(_x) (((AlifDescrObject *)(_x))->name)

class AlifMethodDescrObject { // 38
public:
	ALIFDESCR_COMMON{};
	AlifMethodDef* method{};
	VectorCallFunc vectorCall{};
};

class AlifMemberDescrObject {
public:
	ALIFDESCR_COMMON;
	AlifMemberDef* member{};
};

class AlifGetSetDescrObject {
public:
	ALIFDESCR_COMMON;
	AlifGetSetDef* getSet{};
};


class AlifWrapperDescrObject { // 54
public:
	ALIFDESCR_COMMON;
	WrapperBase* base{};
	void* wrapped{}; /* This can be any function pointer */
};

AlifObject* alifDescr_newWrapper(AlifTypeObject*, WrapperBase*, void*); // 60

AlifIntT alifDescr_isData(AlifObject*); // 62
