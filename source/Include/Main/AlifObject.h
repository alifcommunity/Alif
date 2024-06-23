#pragma once


#define ALIFOBJECT_HEAD		AlifObject _base_{};


//#if SIZEOF_VOID_P > 4

#define ALIF_IMMORTAL_REFCENT ALIF_CAST(int64_t, UINT_MAX)

//#else
//#define ALIF_IMMORTAL_REFCNT ALIF_CAST(int64_t, UINT_MAX >> 2)
//#endif


#define ALIFOBJECT_HEAD_INIT(_type)			\
    {										\
        ALIF_IMMORTAL_REFCENT ,		\
        (_type)                    \
    } \

#define ALIFVAROBJECT_HEAD_INIT(_type, _size)	 \
    {											 \
        ALIFOBJECT_HEAD_INIT(_type),   \
        (_size)                         \
    } \

#define ALIFOBJECT_VAR_HEAD		AlifVarObject _base_{};
#define Alif_INVALID_SIZE (int64_t) - 1

class AlifObject {
public:
	AlifUSizeT ref_{};
	AlifInitObject* type_{};
};

#define ALIFOBJECT_CAST(_op) ALIF_CAST(AlifObject*, (_op))


class AlifVarObject {
public:
	ALIFOBJECT_HEAD
	AlifSizeT size_{};
};

#define ALIFVAROBJECT_CAST(_op) ALIF_CAST(AlifVarObject*, (_op))

int alif_is(AlifObject*, AlifObject*);
#define ALIF_IS(_x, _y) ((_x) == (_y))


static inline int64_t alif_ref(AlifObject* _ob) {
	return _ob->ref_;
}
#  define ALIF_REFCNT(_ob) alif_ref(ALIFOBJECT_CAST(_ob))

static inline AlifTypeObject* alif_type(AlifObject* _ob) {
	return _ob->type_;
}
#  define ALIF_TYPE(_ob) alif_type(ALIFOBJECT_CAST(_ob))

extern AlifTypeObject alifIntegerType;
extern AlifTypeObject alifBoolType;

static inline int64_t alif_size(AlifObject* _ob) {

	return  ALIFVAROBJECT_CAST(_ob)->size_;
}

#define ALIF_SIZE(_ob) alif_size(ALIFOBJECT_CAST(_ob))

static inline int alif_istype(AlifObject* _ob, AlifTypeObject* _type) {
	return ALIF_TYPE(_ob) == _type;
}
#define ALIF_IS_TYPE(_ob, _type) alif_istype(ALIFOBJECT_CAST(_ob), (_type))

static inline int alif_isImmortal(AlifObject* _op)
{
#if SIZEOF_VOID_P > 4
	return (ALIF_CAST(int32_t, _op->ref_) < 0);
#else
	return (_op->ref_ == ALIF_IMMORTAL_REFCENT);
#endif
}
#define ALIFSUB_ISIMMORTAL(_op) alif_isImmortal(ALIFOBJECT_CAST(_op))

void alif_set_ref(AlifObject*, int64_t);

static inline void alif_set_ref(AlifObject* _ob, int64_t _refcnt) {

	if (ALIFSUB_ISIMMORTAL(_ob)) {
		return;
	}

	_ob->ref_ = _refcnt;

}
#define ALIFSET_REFCNT(_ob, _refcnt) alif_set_ref(ALIFOBJECT_CAST(_ob), (_refcnt))

static inline void alif_set_type(AlifObject* _ob, AlifTypeObject* _type) {
	_ob->type_ = _type;
}
#define ALIFSET_TYPE(_ob, _type) alif_set_type(ALIFOBJECT_CAST(_ob), _type)


static inline void alif_set_size(AlifVarObject* _ob, int64_t _size) {

	_ob->size_ = _size;
}
#define ALIFSET_SIZE(_ob, _size) alif_set_size(ALIFVAROBJECT_CAST(_ob), (_size))



extern AlifTypeObject _alifTypeType_;




typedef AlifObject* (*UnaryFunc)(AlifObject*);
typedef AlifObject* (*BinaryFunc)(AlifObject*, AlifObject*);
typedef AlifObject* (*TernaryFunc)(AlifObject*, AlifObject*, AlifObject*);
typedef int (*Inquiry)(AlifObject*);
typedef size_t(*LenFunc)(AlifObject*);
typedef AlifObject* (*SSizeArgFunc)(AlifObject*, size_t);
typedef AlifObject* (*SSizeSSizeArgFunc)(AlifObject*, size_t, size_t);
typedef int(*SSizeObjArgProc)(AlifObject*, size_t, AlifObject*);
typedef int(*SSizeSSizeObjArgProc)(AlifObject*, size_t, size_t, AlifObject*);
typedef int(*ObjObjArgProc)(AlifObject*, AlifObject*, AlifObject*);

typedef int (*ObjObjProc)(AlifObject*, AlifObject*);
typedef int (*VisitProc)(AlifObject*, void*);
typedef int (*TraverseProc)(AlifObject*, VisitProc, void*);

typedef void (*FreeFunc)(void*);
typedef void (*Destructor)(AlifObject*);
typedef AlifObject* (*GetAttrFunc)(AlifObject*, wchar_t*);
typedef AlifObject* (*GetAttroFunc)(AlifObject*, AlifObject*);
typedef int (*SetAttrFunc)(AlifObject*, wchar_t*, AlifObject*);
typedef int (*SetAttroFunc)(AlifObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*ReprFunc)(AlifObject*);
typedef size_t(*HashFunc)(AlifObject*);
typedef AlifObject* (*RichCmpFunc) (AlifObject*, AlifObject*, int);
typedef AlifObject* (*GetIterFunc) (AlifObject*);
typedef AlifObject* (*IterNextFunc) (AlifObject*);
typedef AlifObject* (*DescrGetFunc) (AlifObject*, AlifObject*, AlifObject*);
typedef int (*DescrSetFunc) (AlifObject*, AlifObject*, AlifObject*);
typedef int (*InitProc)(AlifObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*NewFunc)(AlifInitObject*, AlifObject*, AlifObject*);
typedef AlifObject* (*AllocFunc)(AlifInitObject*, size_t);

typedef AlifObject* (*VectorCallFunc)(AlifObject*, AlifObject* const*, size_t, AlifObject*);


AlifObject* alifObject_richCompare(AlifObject*, AlifObject*, int);
int alifObject_richCompareBool(AlifObject*, AlifObject*, int);

int64_t alifObject_hash(AlifObject*);
int64_t alifObject_hashNotImplemented(AlifObject*);

#define ALIFSUBTPFLAGS_STATIC_BUILTIN (1 << 1)

#define ALIFTPFLAGS_INLINE_VALUES (1 << 2)

#define ALIFTPFLAGS_MANAGED_WEAKREF (1 << 3)

#define ALIFTPFLAGS_MANAGED_DICT (1 << 4)

#define ALIFTPFLAGS_PREHEADER (ALIFTPFLAGS_MANAGED_WEAKREF | ALIFTPFLAGS_MANAGED_DICT)

#define ALIFTPFLAGS_SEQUENCE (1 << 5)

#define ALIFTPFLAGS_MAPPING (1 << 6)

#define ALIFTPFLAGS_DISALLOW_INSTANTIATION (1UL << 7)

#define ALIFTPFLAGS_IMMUTABLETYPE (1UL << 8)

#define ALIFTPFLAGS_HEAPTYPE (1UL << 9)

#define ALIFTPFLAGS_BASETYPE (1UL << 10)

#if !defined(ALIF_LIMITED_API) || ALIF_LIMITED_API+0 >= 0x030C0000
#define ALIFTPFLAGS_HAVE_VECTORCALL (1UL << 11)
#ifndef ALIF_LIMITED_API
#define ALIFSUBTPFLAGS_HAVE_VECTORCALL ALIFTPFLAGS_HAVE_VECTORCALL
#endif
#endif

#define ALIFTPFLAGS_READY (1UL << 12)

#define ALIFTPFLAGS_READYING (1UL << 13)

#define ALIFTPFLAGS_HAVE_GC (1UL << 14)

#ifdef STACKLESS
#define ALIFTPFLAGS_HAVE_STACKLESS_EXTENSION (3UL << 15)
#else
#define ALIFTPFLAGS_HAVE_STACKLESS_EXTENSION 0
#endif

#define ALIFTPFLAGS_METHOD_DESCRIPTOR (1UL << 17)

#define ALIFTPFLAGS_VALID_VERSION_TAG  (1UL << 19)

#define ALIFTPFLAGS_IS_ABSTRACT (1UL << 20)

#define ALIFSUBTPFLAGS_MATCH_SELF (1UL << 22)

#define ALIFTPFLAGS_ITEMS_AT_END (1UL << 23)

#define ALIFTPFLAGS_LONG_SUBCLASS        (1UL << 24)
#define ALIFTPFLAGS_LIST_SUBCLASS        (1UL << 25)
#define ALIFTPFLAGS_TUPLE_SUBCLASS       (1UL << 26)
#define ALIFTPFLAGS_BYTES_SUBCLASS       (1UL << 27)
#define ALIFTPFLAGS_USTR_SUBCLASS     (1UL << 28)
#define ALIFTPFLAGS_DICT_SUBCLASS        (1UL << 29)
#define ALIFTPFLAGS_BASE_EXC_SUBCLASS    (1UL << 30)
#define ALIFTPFLAGS_TYPE_SUBCLASS        (1UL << 31)

#define ALIFTPFLAGS_DEFAULT  ( \
                 ALIFTPFLAGS_HAVE_STACKLESS_EXTENSION | \
                0)


void alifSub_dealloc(AlifObject*);

AlifObject* alif_newRef(AlifObject*);

AlifObject* alif_xNewRef(AlifObject*);

static inline void alif_incref(AlifObject* _op)
{
	if (ALIFSUB_ISIMMORTAL(_op)) {
		return;
	}
	_op->ref_++;

}

#define ALIF_INCREF(_op) alif_incref(ALIFOBJECT_CAST(_op))

static inline void alif_decref(AlifObject* _op)
{
	if (ALIFSUB_ISIMMORTAL(_op)) {
		return;
	}
	if (--_op->ref_ == 0) {
		alifSub_dealloc(_op);
	}
}

#define ALIF_DECREF(_op) alif_decref(ALIFOBJECT_CAST(_op))

#define ALIF_CLEAR(_op) \
    do { \
        AlifObject **tmpOpPtr = ALIF_CAST(AlifObject**, &(_op)); \
        AlifObject *tmpOldOp = (*tmpOpPtr); \
        if (tmpOldOp != nullptr) { \
            AlifObject *nullPtr = nullptr; \
            memcpy(tmpOpPtr, &nullPtr, sizeof(AlifObject*)); \
            ALIF_DECREF(tmpOldOp); \
        } \
    } while (0) \

static inline void alif_xIncref(AlifObject* _op)
{
	if (_op != nullptr) {
		ALIF_INCREF(_op);
	}
}

#  define ALIF_XINCREF(_op) alif_xIncref(ALIFOBJECT_CAST(_op))


static inline void alif_xDecref(AlifObject* _op)
{
	if (_op != nullptr) {
		ALIF_DECREF(_op);
	}
}

#define ALIF_XDECREF(_op) alif_xDecref(ALIFOBJECT_CAST(_op))

static inline AlifObject* alifSub_newRef(AlifObject* _obj)
{
	ALIF_INCREF(_obj);
	return _obj;
}

static inline AlifObject* alifSub_xNewRef(AlifObject* _obj)
{
	ALIF_XINCREF(_obj);
	return _obj;
}

#define ALIF_NEWREF(_obj) alifSub_newRef(ALIFOBJECT_CAST(_obj))
#define ALIF_XNEWREF(_obj) alifSub_xNewRef(ALIFOBJECT_CAST(_obj))

extern AlifObject _alifNoneStruct_;
#define ALIF_NONE (&_alifNoneStruct_)

int alif_isNone(AlifObject*);
#define ALIF_ISNONE(_x) ALIF_IS((_x), ALIF_NONE)

#define ALIF_RETURN_NONE return ALIF_NONE

extern AlifObject _alifNotImplemented_;
#define ALIF_NOTIMPLEMENTED (&_alifNotImplemented_)

/* comparison opcodes */
#define ALIF_LT 0
#define ALIF_LE 1
#define ALIF_EQ 2
#define ALIF_NE 3
#define ALIF_GT 4
#define ALIF_GE 5

#define ALIF_RETURN_RICHCOMPARE(val1, val2, op)                               \
    do {                                                                    \
        switch (op) {                                                       \
        case ALIF_EQ: if ((val1) == (val2)) return ALIF_TRUE; return ALIF_FALSE;  \
        case ALIF_NE: if ((val1) != (val2)) return ALIF_TRUE; return ALIF_FALSE;  \
        case ALIF_LT: if ((val1) < (val2)) return ALIF_TRUE; return ALIF_FALSE;   \
        case ALIF_GT: if ((val1) > (val2)) return ALIF_TRUE; return ALIF_FALSE;   \
        case ALIF_LE: if ((val1) <= (val2)) return ALIF_TRUE; return ALIF_FALSE;  \
        case ALIF_GE: if ((val1) >= (val2)) return ALIF_TRUE; return ALIF_FALSE;  \
        }                                                                   \
    } while (0)                                                      \


#define ALIFTYPE_FASTSUBCLASS(_type, _flag) alifType_hasFeature((_type), (_flag))

void alifSub_newReference(AlifObject*);

class AlifIdentifier {
public:
	const wchar_t* string_;
	int64_t index_;
	class {
		uint8_t v_;
	} mutex_;
};


class AlifNumberMethods {
public:
	BinaryFunc add_;
	BinaryFunc subtract_;
	BinaryFunc multiply_;
	BinaryFunc remainder_;
	BinaryFunc divmod_;
	BinaryFunc power_;
	UnaryFunc negative_;
	UnaryFunc positive_;
	UnaryFunc absolute_;
	Inquiry boolean_;
	UnaryFunc invert_;
	BinaryFunc lshift_;
	BinaryFunc rshift_;
	BinaryFunc andLogic;
	BinaryFunc xorLogic;
	BinaryFunc orLogic;
	UnaryFunc intLogic;
	void* reserved_;
	UnaryFunc float_;

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

	UnaryFunc index_;

	BinaryFunc matrixMultiply;
	BinaryFunc inplaceMatrixMultiply;
};

class AlifSequenceMethods {
public:
	LenFunc length_;
	BinaryFunc concat_;
	SSizeArgFunc repeat_;
	SSizeArgFunc item_;
	void* wasSlice;
	SSizeObjArgProc assItem;
	void* wasSqeAssSlice;
	ObjObjProc contains_;

	BinaryFunc inplaceConcat;
	SSizeArgFunc inplaceRepeat;
};

class AlifMappingMethods {
public:
	LenFunc length_;
	BinaryFunc subscript_;
	ObjObjArgProc assSubScript;
};

class AlifBufferProcs {
public:
	GetBufferProc getBuffer{};
	ReleaseBufferProc releaseBuffer{};
};

typedef AlifObject* (*AlifCFunction)(AlifObject*, AlifObject*);

class AlifInitObject {
public:

	ALIFOBJECT_VAR_HEAD;
	const wchar_t* name_;
	size_t basicSize, itemsSize;

	Destructor dealloc_;
	size_t vectorCallOffset;
	GetAttrFunc getAttr;
	SetAttrFunc setAttr;

	ReprFunc repr_;

	AlifNumberMethods* asNumber;
	AlifSequenceMethods* asSequence;
	AlifMappingMethods* asMapping;

	HashFunc hash_;
	TernaryFunc call_;
	ReprFunc str_;
	GetAttroFunc getAttro;
	SetAttroFunc setAttro;

	AlifBufferProcs* asBuffer;


	unsigned long flags_;

	const wchar_t* doc_;

	TraverseProc traverse_;

	Inquiry clear_;

	RichCmpFunc richCompare;

	size_t weakListOffset;

	GetIterFunc iter_;
	IterNextFunc iterNext;

	AlifMethodDef* methods_;
	AlifMemberDef* members_;
	AlifGetSetDef* getSet;

	AlifTypeObject* base_;
	AlifObject* dict_;
	DescrGetFunc descrGet;
	DescrSetFunc descrSet;
	size_t dictOffset;
	InitProc init_;
	AllocFunc alloc_;
	NewFunc newObject;
	FreeFunc free_;
	Inquiry isGC;
	AlifObject* bases_;
	AlifObject* mro_;
	AlifObject* cache_;
	void* subclasses_;
	AlifObject* weakList;
	Destructor del_;

	unsigned int versionTag;

	Destructor finalize;
	VectorCallFunc vectorCall;

	unsigned char watched_;

};

class SpecializationCache {
public:
	AlifObject* getItem;
	uint32_t getItemVersion;
	AlifObject* init_;
};

class AlifDictKeysObject {
public:
	AlifSizeT refCnt{};
	uint8_t log2Size{};
	uint8_t log2IndexBytes{};
	uint8_t kind{};
	uint32_t version{};
	AlifSizeT usable{};
	AlifSizeT nentries{};
	char indices[];
};

class AlifHeapTypeObject {
public:
	AlifInitObject type_;
	AlifNumberMethods number_;
	AlifMappingMethods mapping_;
	AlifSequenceMethods sequence_;
	AlifBufferProcs buffer_;
	AlifObject* name_, * slots_, * qualname_;
	AlifDictKeysObject* cachedKeys{};
	AlifObject* module_;
	char* typeName;
	SpecializationCache specCache;
};

#define ALIF_SETREF(_dst, _src) \
    do { \
        AlifObject **tmpDstPtr = ((AlifObject**)(&_dst)); \
        AlifObject *tmpOldDst = (*tmpDstPtr); \
        AlifObject *tmpSrc = (AlifObject*)_src; \
        memcpy(tmpDstPtr, &tmpSrc, sizeof(AlifObject*)); \
    } while (0) \

#define ALIF_XSETREF(_dst, _src) \
    do { \
        AlifObject **tmpDstPtr =  ((AlifObject**)(&_dst)); \
        AlifObject *tmpOldDst = (*tmpDstPtr); \
        AlifObject *tmpSrc = (AlifObject*)_src; \
        memcpy(tmpDstPtr, &tmpSrc, sizeof(AlifObject*)); \
    } while (0) \

#define ALIFTPFLAGS_USTR_SUBCLASS (1UL << 28)



static inline int alifType_hasFeature(AlifTypeObject* _type, unsigned long _feature) {
	unsigned long flags_ = _type->flags_;
	//#ifdef ALIF_LIMITED_API
	//	flags_ = alifType_getFlags(type);
	//#else
	//	flags_ = _type->flags_;
	//#endif
	return ((flags_ & _feature) != 0);
}
