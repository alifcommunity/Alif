#pragma once


typedef class AlifInitObject;
typedef class AlifMethodDef;



#define ALIFOBJECT_HEAD_INIT(type)    \
    {                               \
        { 0xffffffff },    \
        (type)                      \
    },

#define ALIFVAROBJECT_HEAD_INIT(type, size) \
    {                                     \
        ALIFOBJECT_HEAD_INIT(type)          \
        (size)                            \
    },

class AlifObj {
public:
	uint32_t ref{};
	AlifInitObject* type{};
};

class AlifObjVar {
public:
	AlifObj object;

	size_t size;
};

extern AlifObj alifNone;

#define ALIF_NONE &alifNone


// سيتم تغيير مكان هذا الماكرو في وقت لاحق الى مكان مناسب
#define POINTER_SIZE 8
#define ALIF_SIZE_ROUND_UP(n, a) (((size_t)(n) + \
        (size_t)((a) - 1)) & ~(size_t)((a) - 1)) // هذا الماكرو متوفر له دالة بديلة size_alignUp

void alif_dealloc(AlifObj* object);

void alif_incRef(AlifObj* object);

void alif_decRef(AlifObj* object);

void alif_setImmortal(AlifObj* object);

AlifObj* alifObject_init(AlifObj* object, AlifInitObject* type);
AlifObjVar* alifObject_varInit(AlifObjVar* object, AlifInitObject* type, size_t size);
AlifObj* alifNew_object(AlifInitObject* type);
AlifObjVar* alifNew_varObject(AlifInitObject* type, size_t numberItem);

size_t alifObject_hash(AlifObj* value);
size_t alifObject_hashNotImplemented(AlifObj* object);
AlifObj* alifObject_richCompare(AlifObj* v, AlifObj* w, int op);
int alifObject_richCompareBool(AlifObj* v, AlifObj* w, int op);
void new_reference(AlifObj* object);


typedef AlifObj* (*UnaryFunc)(AlifObj*);
typedef AlifObj* (*BinaryFunc)(AlifObj*, AlifObj*);
typedef AlifObj* (*TernaryFunc)(AlifObj*, AlifObj*, AlifObj*);
typedef int (*Inquiry)(AlifObj*);
typedef size_t(*LenFunc)(AlifObj*);
typedef AlifObj* (*SSizeArgFunc)(AlifObj*, size_t);
typedef AlifObj* (*SSizeSSizeArgFunc)(AlifObj*, size_t, size_t);
typedef int(*SSizeObjArgProc)(AlifObj*, size_t, AlifObj*);
typedef int(*SSizeSSizeObjArgProc)(AlifObj*, size_t, size_t, AlifObj*);
typedef int(*ObjObjArgProc)(AlifObj*, AlifObj*, AlifObj*);

typedef int (*ObjObjProc)(AlifObj*, AlifObj*);
typedef int (*VisitProc)(AlifObj*, void*);
typedef int (*TraverseProc)(AlifObj*, VisitProc, void*);

typedef void (*FreeFunc)(void*);
typedef void (*Destructor)(AlifObj*);
typedef AlifObj* (*GetAttrFunc)(AlifObj*, wchar_t*);
typedef AlifObj* (*GetAttroFunc)(AlifObj*, AlifObj*);
typedef int (*SetAttrFunc)(AlifObj*, wchar_t*, AlifObj*);
typedef int (*SetAttroFunc)(AlifObj*, AlifObj*, AlifObj*);
typedef AlifObj* (*ReprFunc)(AlifObj*);
typedef size_t(*HashFunc)(AlifObj*);
typedef AlifObj* (*RichCmpFunc) (AlifObj*, AlifObj*, int);
typedef AlifObj* (*GetIterFunc) (AlifObj*);
typedef AlifObj* (*IterNextFunc) (AlifObj*);
typedef AlifObj* (*DescrGetFunc) (AlifObj*, AlifObj*, AlifObj*);
typedef int (*DescrSetFunc) (AlifObj*, AlifObj*, AlifObj*);
typedef int (*InitProc)(AlifObj*, AlifObj*, AlifObj*);
typedef AlifObj* (*NewFunc)(AlifInitObject*, AlifObj*, AlifObj*);
typedef AlifObj* (*AllocFunc)(AlifInitObject*, size_t);

typedef AlifObj* (*VectorCallFunc)(AlifObj*, AlifObj* const*, size_t, AlifObj*);

class AlifNumberMethods {
public:
	BinaryFunc add;
	BinaryFunc subtract;
	BinaryFunc multiply;
	BinaryFunc remainder;
	BinaryFunc divmod;
	BinaryFunc power;
	UnaryFunc negative;
	UnaryFunc positive;
	UnaryFunc absolute;
	Inquiry boolean;
	UnaryFunc invert;
	BinaryFunc lshift;
	BinaryFunc rshift;
	BinaryFunc andLogin;
	BinaryFunc xorLogin;
	BinaryFunc orLogin;
	UnaryFunc intLogin;
	void* reserved;
	UnaryFunc floatLogin;

	BinaryFunc inplaceAdd;
	BinaryFunc inplaceSubtract;
	BinaryFunc inplaceMultiply;
	BinaryFunc inplaceRemainder;
	TernaryFunc inplacePower;
	BinaryFunc inplaceLShift;
	BinaryFunc inplaceRShift;
	BinaryFunc inplaceAnd;
	BinaryFunc inplaceXor;
	BinaryFunc inplaceOr;

	BinaryFunc floorDivide;
	BinaryFunc trueDivide;
	BinaryFunc inplaceFloorDivide;
	BinaryFunc inplaceTrueDivide;

	UnaryFunc index;

	BinaryFunc matrixMultiply;
	BinaryFunc inplaceMatrixMultiply;
};

class AlifSequenceMethods {
public:
	LenFunc length;
	BinaryFunc concat;
	SSizeArgFunc repeat;
	SSizeArgFunc item;
	void* wasSlice;
	SSizeObjArgProc assItem;
	void* wasAssSlice;
	ObjObjProc contains;

	BinaryFunc inplaceConcat;
	SSizeArgFunc inplaceRepeat;
};

class AlifMappingMethods {
public:
	LenFunc length;
	BinaryFunc subscript;
	ObjObjArgProc assSubscript;
};

typedef AlifObj* (*AlifCFunction)(AlifObj*, AlifObj*);

class AlifMemberDef {
public:
	const char* name;
	int type;
	size_t offset;
	int flags;
	const char* doc;
};

typedef AlifObj* (*Getter)(AlifObj*, void*);
typedef int (*Setter)(AlifObj*, AlifObj*, void*);

class AlifGetSetDef {
public:
	const char* name;
	Getter get;
	Setter set;
	const char* doc;
	void* closure;
};

class AlifInitObject {
public:

	//AlifObjVar object;
	const wchar_t* name;
	size_t basicSize, itemsSize;

	Destructor dealloc;
	size_t vectorCallOffset;
	GetAttrFunc getAttr;
	SetAttrFunc setAttr;

	ReprFunc repr;

	AlifNumberMethods* asNumber;
	AlifSequenceMethods* asSequence;
	AlifMappingMethods* asMapping;

	HashFunc hash;
	TernaryFunc call;
	ReprFunc str;
	GetAttroFunc getAttro;
	SetAttroFunc setAttro;

	unsigned long flags;

	const char* doc;

	TraverseProc traverse;

	Inquiry clear;

	RichCmpFunc richCompare;

	size_t weakListOffset;

	GetIterFunc iter;
	IterNextFunc iterNext;

	AlifMethodDef* methods;
	AlifMemberDef* members;
	AlifGetSetDef* getSet;

	AlifInitObject* base;
	AlifObj* dict;
	DescrGetFunc descrGet;
	DescrSetFunc descrSet;
	size_t dictoffset;
	InitProc init;
	AllocFunc alloc;
	NewFunc newObject;
	FreeFunc free;
	Inquiry isGC;
	AlifObj* bases;
	AlifObj* mro;
	AlifObj* cache;
	void* subclasses;
	AlifObj* weakList;
	Destructor del;

	unsigned int versionTag;

	Destructor finalize;
	VectorCallFunc vectorCall;

	unsigned char watched;

};

/* This struct is used by the specializer
 * It should be treated as an opaque blob
 * by code other than the specializer and interpreter. */
class SpecializationCache {
public:
	// In order to avoid bloating the bytecode with lots of inline caches, the
	// members of this structure have a somewhat unique contract. They are set
	// by the specialization machinery, and are invalidated by PyType_Modified.
	// The rules for using them are as follows:
	// - If getitem is non-NULL, then it is the same Python function that
	//   PyType_Lookup(cls, "__getitem__") would return.
	// - If getitem is NULL, then getitem_version is meaningless.
	// - If getitem->func_version == getitem_version, then getitem can be called
	//   with two positional arguments and no keyword arguments, and has neither
	//   *args nor **kwargs (as required by BINARY_SUBSCR_GETITEM):
	AlifObj* getitem;
	uint32_t getitem_version;
	AlifObj* init;
};

class AlifHeapTypeObject {
public:
	AlifInitObject type;
	AlifNumberMethods number;
	AlifMappingMethods mapping;
	AlifSequenceMethods sequence; /* as_sequence comes after as_mapping,
									  so that the mapping wins when both
									  the mapping and the sequence define
									  a given operator (e.g. __getitem__).
									  see add_operators() in typeobject.c . */
	//PyBufferProcs as_buffer;
	AlifObj* name, * slots, * qualname;
	//struct _dictkeysobject* ht_cached_keys;
	AlifObj* ht_module;
	char* typeName;  // Storage for "tp_name"; see PyType_FromModuleAndSpec
	SpecializationCache specCache; // For use by the specializer.
	/* here are optional user slots, followed by the members. */
} ;

/* comparison opcodes */
#define ALIF_LT 0
#define ALIF_LE 1
#define ALIF_EQ 2
#define ALIF_NE 3
#define ALIF_GT 4
#define ALIF_GE 5

//#define ALIF_RETURN_COMPARE(val1, val2, op)                               \
//    do {                                                                    \
//        switch (op) {                                                       \
//        case ALIF_EQ: if ((val1) == (val2)) ALIF_TRUE; ALIF_FALSE;  \
//        case ALIF_NE: if ((val1) != (val2)) ALIF_TRUE; ALIF_FALSE;  \
//        case ALIF_NE: if ((val1) != (val2)) ALIF_TRUE; ALIF_FALSE;  \
//        case ALIF_LT: if ((val1) < (val2)) ALIF_TRUE; ALIF_FALSE;   \
//        case ALIF_GT: if ((val1) > (val2)) ALIF_TRUE; ALIF_FALSE;   \
//        case ALIF_LE: if ((val1) <= (val2)) ALIF_TRUE; ALIF_FALSE;  \
//        case ALIF_GE: if ((val1) >= (val2)) ALIF_TRUE; ALIF_FALSE;  \
//        default:                                                            \
//            /* error */                                               \
//        }                                                                   \
//    } while (0)
