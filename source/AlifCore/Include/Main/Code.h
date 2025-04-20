#pragma once



class AlifCoCached { // 28
public:
	AlifObject* code{};
	AlifObject* varNames{};
	AlifObject* cellVars{};
	AlifObject* freeVars{};
};


class AlifExecutorArray { // 44
public:
	AlifIntT size{};
	AlifIntT capacity{};
	class AlifExecutorObject* executors[1]{};
};

 // 73
#define ALIFCODE_DEF(_size) {                                                    \
public:																			\
    ALIFOBJECT_VAR_HEAD;                                                         \
    AlifObject *consts{};           /* list (constants used) */                 \
    AlifObject *names{};            /* list of strings (names used) */          \
    AlifObject *exceptiontable{};												\
    AlifIntT flags{};															\
                                                                               \
    /* The rest are not so impactful on performance. */                        \
    AlifIntT argCount{};              /* #arguments, except *args */               \
    AlifIntT posOnlyArgCount{};       /* #positional only arguments */             \
    AlifIntT kwOnlyArgCount{};        /* #keyword only arguments */                \
    AlifIntT stackSize{};             /* #entries needed for evaluation stack */   \
    AlifIntT firstLineno{};           /* first source line number */               \
                                                                               \
    AlifIntT nLocalsPlus{};           /* number of local + cell + free variables */ \
    AlifIntT frameSize{};             /* Size of frame in words */                 \
    AlifIntT nLocals{};               /* number of local variables */              \
    AlifIntT nCellVars{};             /* total number of cell variables */         \
    AlifIntT nFreeVars{};             /* number of free variables */               \
    uint32_t version{};          /* version number */                         \
                                                                               \
    AlifObject *localsPlusNames{}; /* tuple mapping offsets to names */         \
    AlifObject *localsPlusKinds{};												\
    AlifObject *filename{};        /* unicode (where it was loaded from) */     \
    AlifObject *name{};            /* unicode (name, for reference) */          \
    AlifObject *qualname{};        /* unicode (qualname, for reference) */      \
    AlifObject *lineTable{};       /* bytes object that holds location info */  \
    AlifObject *weakRefList{};     /* to support weakrefs to code objects */    \
    AlifExecutorArray *executors{};      /* executors from optimizer */        \
    AlifCoCached *cached{};      /* cached co_* attributes */                 \
    uintptr_t instrumentationVersion{}; /* current instrumentation version */ \
    /*AlifCoMonitoringData *monitoring{};*/ /* Monitoring data */                 \
    AlifIntT firstTraceable{};       /* index of first traceable instruction */   \
    void *extra{};                                                            \
    char codeAdaptive[(_size)]{};                                             \
}

/* Bytecode object */
class AlifCodeObject ALIFCODE_DEF(1); // 140

 // 143
#define CO_OPTIMIZED    0x0001
#define CO_NEWLOCALS    0x0002
#define CO_VARARGS      0x0004
#define CO_VARKEYWORDS  0x0008
#define CO_NESTED       0x0010
#define CO_GENERATOR    0x0020

#define CO_COROUTINE            0x0080
#define CO_ITERABLE_COROUTINE   0x0100
#define CO_ASYNC_GENERATOR      0x0200


 // 160
#define CO_FUTURE_DIVISION      0x20000
#define CO_FUTURE_ABSOLUTE_IMPORT 0x40000 /* do absolute imports by default */
#define CO_FUTURE_WITH_STATEMENT  0x80000
#define CO_FUTURE_PRINT_FUNCTION  0x100000
#define CO_FUTURE_UNICODE_LITERALS 0x200000

#define CO_FUTURE_BARRY_AS_BDFL  0x400000
#define CO_FUTURE_GENERATOR_STOP  0x800000
#define CO_FUTURE_ANNOTATIONS    0x1000000

#define CO_NO_MONITORING_EVENTS 0x2000000



#define MAXBLOCKS 21

extern AlifTypeObject _alifCodeType_; // 179
#define ALIFCODE_CHECK(_op) ALIF_IS_TYPE((_op), &_alifCodeType_)



static inline AlifIntT alifUnstableCode_getFirstFree(AlifCodeObject* _op) { // 188
	return _op->nLocalsPlus - _op->nFreeVars;
}


AlifIntT alifCode_addr2Line(AlifCodeObject*, AlifIntT); // 243


 // 247
#define ALIF_FOREACH_CODE_EVENT(_v) \
    _v(Create)                 \
    _v(Destroy)

enum AlifCodeEvent { // 274
#define ALIF_DEF_EVENT(_op) Alif_Code_Event_##_op,
	ALIF_FOREACH_CODE_EVENT(ALIF_DEF_EVENT)
#undef ALIF_DEF_EVENT
};



typedef AlifIntT (*AlifCodeWatchCallback)(AlifCodeEvent _event, AlifCodeObject* _co); // 263


class Opaque {
public:
	AlifIntT computedLine{};
	const uint8_t* loNext{};
	const uint8_t* limit{};
};

class AlifCodeAddressRange { // 294
public:
	AlifIntT start{};
	AlifIntT end{};
	AlifIntT line{};
	Opaque opaque{};
};
typedef AlifCodeAddressRange LineOffsets;


AlifIntT _alifCode_checkLineNumber(AlifIntT, AlifCodeAddressRange*); // 304


AlifObject* alifCode_constantKey(AlifObject*); // 309

enum AlifCodeLocationInfoKind { // 340
	AlifCode_Location_Info_Short = 0,
	AlifCode_Location_Info_One_Line0 = 10,
	AlifCode_Location_Info_One_Line1 = 11,
	AlifCode_Location_Info_One_Line2 = 12,
	AlifCode_Location_Info_No_Columns = 13,

	AlifCode_Location_Info_Long = 14,
	AlifCode_Location_Info_None = 15

};
