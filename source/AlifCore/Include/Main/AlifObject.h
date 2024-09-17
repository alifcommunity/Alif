#pragma once





#define ALIFOBJECT_HEAD		AlifObject objBase // 60


#define ALIFOBJECT_HEAD_INIT(_type)		\
    {									\
        0,								\
        0,								\
        { 0 },							\
        0,								\
        ALIF_IMMORTAL_REFCNT_LOCAL,		\
        0,								\
        (_type),						\
    }

// 89
#define ALIFVAROBJECT_HEAD_INIT(_type, _size)	\
    {											\
        ALIFOBJECT_HEAD_INIT(_type),			\
        (_size)									\
    }


#define ALIFOBJECT_VAR_HEAD		AlifVarObject objBase // 101


#define ALIF_UNOWNED_TID		0

class AlifObject { // 140
public:
	uintptr_t threadID{};
	uint16_t padding{};
	AlifMutex mutex{};
	uint8_t gcBits{};
	uint32_t refLocal{};
	AlifSizeT refShared{};
	AlifTypeObject* type{};
};

#define ALIFOBJECT_CAST(_op) ALIF_CAST(AlifObject*, _op) // 155

class AlifVarObject { // 157
public:
	AlifObject objBase{};
	AlifSizeT objSize{};
};

#define ALIFVAROBJECT_CAST(_op) ALIF_CAST(AlifVarObject*, (_op)) // 163

uintptr_t alif_getThreadLocalAddr(void); // 171

static inline uintptr_t alif_threadID(void) { // 173
	uintptr_t threadID;
#if defined(_MSC_VER) and defined(_M_X64)
	threadID = __readgsqword(48);
#elif defined(_MSC_VER) and defined(_M_IX86)
	threadID = __readfsdword(24);
#elif defined(_MSC_VER) and defined(_M_ARM64)
	threadID = __getReg(18);
#elif defined(__i386__)
	__asm__("movl %%gs:0, %0" : "=r" (threadID));  // 32-bit always uses GS
#elif defined(__MACH__) and defined(__x86_64__)
	__asm__("movq %%gs:0, %0" : "=r" (threadID));  // x86_64 macOSX uses GS
#elif defined(__x86_64__)
	__asm__("movq %%fs:0, %0" : "=r" (threadID));  // x86_64 Linux, BSD uses FS
#elif defined(__arm__)
	__asm__("mrc p15, 0, %0, c13, c0, 3\nbic %0, %0, #3" : "=r" (threadID));
#elif defined(__aarch64__) and defined(__APPLE__)
	__asm__("mrs %0, tpidrro_el0" : "=r" (threadID));
#elif defined(__aarch64__)
	__asm__("mrs %0, tpidr_el0" : "=r" (threadID));
#elif defined(__powerpc64__)
#if defined(__clang__) and ALIF_HAS_BUILTIN(__builtin_thread_pointer)
	threadID = (uintptr_t)__builtin_thread_pointer();
#else
	// r13 is reserved for use as system thread ID by the Power 64-bit ABI.
	register uintptr_t tp __asm__("r13");
	__asm__("" : "=r" (tp));
	threadID = tp;
#endif
#elif defined(__powerpc__)
#if defined(__clang__) and ALIF_HAS_BUILTIN(__builtin_thread_pointer)
	threadID = (uintptr_t)__builtin_thread_pointer();
#else
	// r2 is reserved for use as system thread ID by the Power 32-bit ABI.
	register uintptr_t tp __asm__("r2");
	__asm__("" : "=r" (tp));
	threadID = tp;
#endif
#elif defined(__s390__) and defined(__GNUC__)
	// Both GCC and Clang have supported __builtin_thread_pointer
	// for s390 from long time ago.
	threadID = (uintptr_t)__builtin_thread_pointer();
#elif defined(__riscv)
#if defined(__clang__) and ALIF_HAS_BUILTIN(__builtin_thread_pointer)
	threadID = (uintptr_t)__builtin_thread_pointer();
#else
	// tp is Thread Pointer provided by the RISC-V ABI.
	__asm__("mv %0, tp" : "=r" (threadID));
#endif
#else
	// Fallback to a portable implementation if we do not have a faster
	// platform-specific implementation.
	threadID = alif_getThreadLocalAddr();
#endif
	return threadID;
}


static inline ALIF_ALWAYS_INLINE AlifIntT
alif_isOwnedByCurrentThread(AlifObject* ob) { // 232
	return ob->threadID == alif_threadID();
}

static inline AlifTypeObject* alif_type(AlifObject* _ob) { // 250
	return _ob->type;
}
#define ALIF_TYPE(_ob) alif_type(ALIFOBJECT_CAST(_ob))

//extern AlifTypeObject _alifLongType_; // 261
extern AlifTypeObject _alifBoolType_; // 262

static inline AlifSizeT alif_size(AlifObject* ob) { // 265
	return  ALIFVAROBJECT_CAST(ob)->objSize;
}
#define ALIF_SIZE(ob) alif_size(ALIFOBJECT_CAST(ob))

static inline AlifIntT alif_isType(AlifObject* _obj, AlifTypeObject* _type) { // 274
	return ALIF_TYPE(_obj) == _type;
}
#define ALIF_IS_TYPE(_obj, _type) alif_isType(ALIFOBJECT_CAST(_obj), (_type))


static inline void alif_setType(AlifObject* _obj, AlifTypeObject* _type) { // 282
	_obj->type = _type;
}
# define ALIF_SET_TYPE(_ob, _type) alif_setType(ALIFOBJECT_CAST(_ob), _type)


static inline void alif_setSize(AlifVarObject* _ob, AlifSizeT _size) { // 289
	alifAtomic_storeSizeRelaxed(&_ob->objSize, _size);
}
#  define ALIF_SET_SIZE(_ob, _size) alif_setSize(ALIFVAROBJECT_CAST(_ob), (_size)) // 300


typedef AlifObject* (*UnaryFunc)(AlifObject*); // 318
typedef AlifObject* (*BinaryFunc)(AlifObject*, AlifObject*); // 319
typedef AlifObject* (*TernaryFunc)(AlifObject*, AlifObject*, AlifObject*); // 320
typedef AlifIntT (*Inquiry)(AlifObject*); // 321
typedef AlifSizeT (*LenFunc)(AlifObject*); // 322
typedef AlifObject* (*SizeArgFunc)(AlifObject*, AlifSizeT); // 323
typedef AlifIntT (*SizeObjArgProc)(AlifObject*, AlifSizeT, AlifObject*); // 325
typedef AlifIntT (*ObjObjArgProc)(AlifObject*, AlifObject*, AlifObject*); // 327
typedef AlifIntT(*ObjObjProc)(AlifObject*, AlifObject*); // 329
typedef AlifIntT (*VisitProc)(AlifObject*, void*); // 330
typedef AlifIntT (*TraverseProc)(AlifObject*, VisitProc, void*); // 331

typedef void (*FreeFunc)(void*); // 334
typedef void (*Destructor)(AlifObject*); // 335
typedef AlifObject* (*GetAttrFunc)(AlifObject*, char*); // 336
typedef AlifObject* (*GetAttroFunc)(AlifObject*, AlifObject*); // 337
typedef AlifIntT (*SetAttrFunc)(AlifObject*, char*, AlifObject*); // 338
typedef AlifIntT(*SetAttroFunc)(AlifObject*, AlifObject*, AlifObject*); // 339
typedef AlifHashT (*HashFunc)(AlifObject*); // 341
typedef AlifObject* (*RichCmpFunc) (AlifObject*, AlifObject*, AlifIntT); // 342
typedef AlifIntT (*InitProc)(AlifObject*, AlifObject*, AlifObject*); // 347
typedef AlifObject* (*NewFunc)(AlifTypeObject*, AlifObject*, AlifObject*); // 348

typedef AlifObject* (*VectorCallFunc)(AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*); // 352



// 360
#define ALIF_XSETREF(_dst, _src) \
    do { \
        AlifObject **tmpDstPtr = ALIF_CAST(AlifObject**, &(_dst)); \
        AlifObject *tmpOldDst = (*tmpDstPtr); \
        AlifObject *tmpSrc = ALIFOBJECT_CAST(_src); \
        memcpy(tmpDstPtr, &tmpSrc, sizeof(AlifObject*)); \
        alif_xdecRef(tmpOldDst); \
    } while (0)





AlifIntT alifType_isSubType(AlifTypeObject*, AlifTypeObject*); // 396

static inline AlifIntT alifObject_typeCheck(AlifObject* _ob, AlifTypeObject* _type) { // 398
	return ALIF_IS_TYPE(_ob, _type) or alifType_isSubType(ALIF_TYPE(_ob), _type);
}
#define ALIFOBJECT_TYPECHECK(ob, type) alifObject_typeCheck(ALIFOBJECT_CAST(ob), (type))




extern AlifTypeObject _alifTypeType_; // 405
extern AlifTypeObject _alifBaseObjectType_; // 406 /* built-in 'object' */

AlifIntT alifType_ready(AlifTypeObject*); // 411

AlifIntT alifObject_richCompareBool(AlifObject* , AlifObject* , AlifIntT ); // 424

AlifIntT alifObject_setAttrString(AlifObject*, const char*, AlifObject*); // 426

AlifIntT alifObject_setAttr(AlifObject*, AlifObject*, AlifObject*); // 434

AlifHashT alifObject_hash(AlifObject*); // 447
AlifHashT alifObject_hashNotImplemented(AlifObject*); // 448
AlifIntT alifObject_isTrue(AlifObject*); // 449


// 491
#define ALIF_TPFLAGS_STATIC_BUILTIN (1 << 1)
#define ALIF_TPFLAGS_INLINE_VALUES (1 << 2)
#define ALIF_TPFLAGS_MANAGED_WEAKREF (1 << 3)
#define ALIF_TPFLAGS_MANAGED_DICT (1 << 4)
#define ALIF_TPFLAGS_PREHEADER (ALIF_TPFLAGS_MANAGED_WEAKREF | ALIF_TPFLAGS_MANAGED_DICT)
#define ALIF_TPFLAGS_SEQUENCE (1 << 5)
#define ALIF_TPFLAGS_MAPPING (1 << 6)
#define ALIF_TPFLAGS_DISALLOW_INSTANTIATION (1UL << 7)
#define ALIF_TPFLAGS_IMMUTABLETYPE (1UL << 8)
#define ALIF_TPFLAGS_HEAPTYPE (1UL << 9)
#define ALIF_TPFLAGS_BASETYPE (1UL << 10)
#define ALIF_TPFLAGS_HAVE_VECTORCALL (1UL << 11)
#define _ALIF_TPFLAGS_HAVE_VECTORCALL ALIF_TPFLAGS_HAVE_VECTORCALL
#define ALIF_TPFLAGS_READY (1UL << 12)
#define ALIF_TPFLAGS_READYING (1UL << 13)
#define ALIF_TPFLAGS_HAVE_GC (1UL << 14)
#define ALIF_TPFLAGS_HAVE_STACKLESS_EXTENSION 0
#define ALIF_TPFLAGS_METHOD_DESCRIPTOR (1UL << 17)
#define ALIF_TPFLAGS_VALID_VERSION_TAG  (1UL << 19)
#define ALIF_TPFLAGS_IS_ABSTRACT (1UL << 20)
#define _ALIF_TPFLAGS_MATCH_SELF (1UL << 22)
#define ALIF_TPFLAGS_ITEMS_AT_END (1UL << 23)
#define ALIF_TPFLAGS_LONG_SUBCLASS        (1UL << 24)
#define ALIF_TPFLAGS_LIST_SUBCLASS        (1UL << 25)
#define ALIF_TPFLAGS_TUPLE_SUBCLASS       (1UL << 26)
#define ALIF_TPFLAGS_BYTES_SUBCLASS       (1UL << 27)
#define ALIF_TPFLAGS_UNICODE_SUBCLASS     (1UL << 28)
#define ALIF_TPFLAGS_DICT_SUBCLASS        (1UL << 29)
#define ALIF_TPFLAGS_BASE_EXC_SUBCLASS    (1UL << 30)
#define ALIF_TPFLAGS_TYPE_SUBCLASS        (1UL << 31)
#define ALIF_TPFLAGS_DEFAULT  (ALIF_TPFLAGS_HAVE_STACKLESS_EXTENSION | 0)


#define _ALIF_TPFLAGS_MATCH_SELF (1UL << 22) // 566


extern AlifObject _alifNoneClass_; // 623

#define ALIF_NONE (&_alifNoneClass_) // 628


extern AlifObject _alifNotImplementedClass_;  // 642

#define ALIF_NOTIMPLEMENTED (&_alifNotImplementedClass_) // 647


// 654
#define ALIF_LT 0
#define ALIF_LE 1
#define ALIF_EQ 2
#define ALIF_NE 3
#define ALIF_GT 4
#define ALIF_GE 5


/* -------------------------------------------------------------------------------------------------------------- */


void alif_newReference(AlifObject*); // 5
void alif_newReferenceNoTotal(AlifObject*); // 6


class AlifNumberMethods{ // 60
public:
	BinaryFunc add_{};
	BinaryFunc subtract{};
	BinaryFunc multiply{};
	BinaryFunc remainder{};
	BinaryFunc divmod{};
	TernaryFunc power{};
	UnaryFunc negative{};
	UnaryFunc positive{};
	UnaryFunc absolute{};
	Inquiry bool_{};
	UnaryFunc invert{};
	BinaryFunc lshift{};
	BinaryFunc rshift{};
	BinaryFunc and_{};
	BinaryFunc xor_{};
	BinaryFunc or_{};
	UnaryFunc int_{};
	void* reserved{};
	UnaryFunc float_{};

	BinaryFunc inplaceAdd{};
	BinaryFunc inplaceSubtract{};
	BinaryFunc inplaceMultiply{};
	BinaryFunc inplaceRemainder{};
	TernaryFunc inplacePower{};
	BinaryFunc inplaceLshift{};
	BinaryFunc inplaceRshift{};
	BinaryFunc inplaceAnd{};
	BinaryFunc inplaceXor{};
	BinaryFunc inplaceOr{};

	BinaryFunc floorDivide{};
	BinaryFunc trueDivide{};
	BinaryFunc inplaceFloorDivide{};
	BinaryFunc inplaceTrueDivide{};

	UnaryFunc index{};

	BinaryFunc matrixMultiply{};
	BinaryFunc inplaceMatrixMultiply{};
};

class AlifSequenceMethods{ // 107
public:

	LenFunc length{};
	BinaryFunc concat{};
	SizeArgFunc repeat{};
	SizeArgFunc item{};
	void* wasSlice{};
	SizeObjArgProc assItem{};
	void* wasAssSlice{};
	ObjObjProc contains{};

	BinaryFunc inplaceConcat{};
	SizeArgFunc inplaceRepeat{};
};

class AlifMappingMethods{ // 121
public:
	LenFunc length{};
	BinaryFunc subscript{};
	ObjObjArgProc assSubscript{};
};


class AlifTypeObject { // 147
public:
	ALIFOBJECT_VAR_HEAD{};
	const char* name{};
	AlifSizeT basicSize{}, itemSize{};

	Destructor dealloc{};
	GetAttrFunc getAttr{};
	SetAttrFunc setAttr{};

	AlifNumberMethods* asNumber{};
	AlifSequenceMethods* asSequence{};
	AlifMappingMethods* asMapping{};

	HashFunc hash{};

	GetAttroFunc getAttro{};
	SetAttroFunc setAttro{};

	unsigned long flags{};

	RichCmpFunc richCompare{};


	AlifTypeObject* base{};
	AlifObject* dict{};
	InitProc init{};
	NewFunc new_{};
	FreeFunc free{};
	Inquiry isGC{};
	AlifObject* methResOrder{}; // mro
	void* subclasses{};
};




class AlifHeapTypeObject { // 255
public:
	AlifTypeObject type{};
	//AlifAsyncMethods async;
	//AlifNumberMethods number;
	//AlifMappingMethods mapping;
	//AlifSequenceMethods sequence;
	//AlifBufferProcs Buffer;
	AlifObject* name{}, * slots{}, * qualname{};
	class DictKeysObject* cachedKeys{};
	AlifObject* module_{};
	AlifSizeT uniqueID{};
};


// 338
#define ALIF_SETREF(_dst, _src) \
    do { \
        AlifObject **tmpDstPtr = ALIF_CAST(AlifObject**, &(_dst)); \
        AlifObject *tmpOldDst = (*tmpDstPtr); \
        AlifObject *tmpSrc = ALIFOBJECT_CAST(_src); \
        memcpy(tmpDstPtr, &tmpSrc, sizeof(AlifObject*)); \
        ALIF_DECREF(tmpOldDst); \
    } while (0)




enum AlifRefTracerEvent_ {
	Alif_RefTracer_Create = 0,
	Alif_RefTracer_Destroy = 1,
};

typedef AlifIntT (*AlifRefTracer)(AlifObject*, AlifRefTracerEvent_ event, void*); // 526









/* -------------------------------------------------------------------------------------------------------------------------------- */






static inline AlifIntT alifType_hasFeature(AlifTypeObject* _type, unsigned long _feature) { // 749
	unsigned long flags{};
	flags = ALIFATOMIC_LOAD_ULONG_RELAXED(&_type->flags);
	return ((flags & _feature) != 0);
}


#define ALIFTYPE_FASTSUBCLASS(_type, _flag) alifType_hasFeature((_type), (_flag)) // 766
