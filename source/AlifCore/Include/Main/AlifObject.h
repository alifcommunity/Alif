#pragma once





#define ALIFOBJECT_HEAD		AlifObject base // 60


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


#define ALIFOBJECT_VAR_HEAD		AlifVarObject base // 101


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

#define ALIFOBJECT_CAST(_op) ALIF_CAST(AlifObject*, (_op)) // 155

class AlifVarObject { // 157
public:
	AlifObject base{};
	AlifSizeT size{};
};

uintptr_t alif_getThreadLocalAddr(void); // 171

static inline uintptr_t alif_threadId(void) { // 173
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
	return ob->threadID == alif_threadId();
}

static inline AlifTypeObject* alif_type(AlifObject* _ob) { // 250
	return _ob->type;
}
#define ALIF_TYPE(ob) alif_type(ob)




static inline void alif_setType(AlifObject* _obj, AlifTypeObject* _type) { // 282
	_obj->type = _type;
}
# define ALIF_SET_TYPE(_ob, _type) alif_setType(ALIFOBJECT_CAST(_ob), _type)




typedef AlifIntT (*Inquiry)(AlifObject*); // 321

typedef AlifIntT (*VisitProc)(AlifObject*, void*); // 330
typedef AlifIntT (*TraverseProc)(AlifObject*, VisitProc, void*); // 331

typedef void (*FreeFunc)(void*); // 334
typedef void (*Destructor)(AlifObject*); // 335





extern AlifTypeObject _alifTypeType_; // 405





/* -------------------------------------------------------------------------------------------------------------- */







class AlifTypeObject { // 147
public:
	ALIFOBJECT_VAR_HEAD{};
	const char* name{};
	AlifSizeT basicSize{}, itemSize{};
	Destructor dealloc{};
};




class AlifHeapTypeObject { // 255
	AlifTypeObject type{};
	//AlifAsyncMethods async;
	//AlifNumberMethods number;
	//AlifMappingMethods mapping;
	//AlifSequenceMethods sequence;
	//AlifBufferProcs Buffer;
	AlifObject* name{}, * slots{}, * qualname{};
	AlifObject* Module{};
	AlifSizeT uniqueID{};
};








enum AlifRefTracerEvent_ {
	Alif_RefTracer_Create = 0,
	Alif_RefTracer_Destroy = 1,
};

typedef AlifIntT (*AlifRefTracer)(AlifObject*, AlifRefTracerEvent_ event, void*); // 526
