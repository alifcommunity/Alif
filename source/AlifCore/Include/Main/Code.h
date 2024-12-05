#pragma once






 // 73
#define ALIFCODE_DEF(_size) {                                                    \
public:																			\
    ALIFOBJECT_VAR_HEAD;                                                         \
                                                                               \
    /* Note only the following fields are used in hash and/or comparisons      \
     *                                                                         \
     * - name                                                               \
     * - argcount                                                           \
     * - posonlyargcount                                                    \
     * - kwonlyargcount                                                     \
     * - nlocals                                                            \
     * - stacksize                                                          \
     * - flags                                                              \
     * - firstlineno                                                        \
     * - consts                                                             \
     * - names                                                              \
     * - localsplusnames                                                    \
     * This is done to preserve the name and line number for tracebacks        \
     * and debuggers; otherwise, constant de-duplication would collapse        \
     * identical functions/lambdas defined on different lines.                 \
     */                                                                        \
                                                                               \
    /* These fields are set with provided values on new code objects. */       \
                                                                               \
    /* The hottest fields (in the eval loop) are grouped here at the top. */   \
    AlifObject *consts{};           /* list (constants used) */                 \
    AlifObject *names{};            /* list of strings (names used) */          \
    AlifObject *exceptiontable{};   /* Byte string encoding exception handling  \
                                      table */                                 \
    AlifIntT flags{};                  /* CO_..., see below */                     \
                                                                               \
    /* The rest are not so impactful on performance. */                        \
    AlifIntT argCount{};              /* #arguments, except *args */               \
    AlifIntT posOnlyArgCount{};       /* #positional only arguments */             \
    AlifIntT kwOnlyArgCount{};        /* #keyword only arguments */                \
    AlifIntT stackSize{};             /* #entries needed for evaluation stack */   \
    AlifIntT firstLineno{};           /* first source line number */               \
                                                                               \
    /* redundant values (derived from co_localsplusnames and                   \
       co_localspluskinds) */                                                  \
    AlifIntT nLocalsPlus{};           /* number of local + cell + free variables */ \
    AlifIntT frameSize{};             /* Size of frame in words */                 \
    AlifIntT nLocals{};               /* number of local variables */              \
    AlifIntT nCellVars{};             /* total number of cell variables */         \
    AlifIntT nFreeVars{};             /* number of free variables */               \
    uint32_t version{};          /* version number */                         \
                                                                               \
    AlifObject *localsPlusNames{}; /* tuple mapping offsets to names */         \
    AlifObject *localsPlusKinds{}; /* Bytes mapping to local kinds (one byte    \
                                     per variable) */                          \
    AlifObject *filename{};        /* unicode (where it was loaded from) */     \
    AlifObject *name{};            /* unicode (name, for reference) */          \
    AlifObject *qualname{};        /* unicode (qualname, for reference) */      \
    AlifObject *lineTable{};       /* bytes object that holds location info */  \
    AlifObject *weakRefList{};     /* to support weakrefs to code objects */    \
    /*AlifExecutorArray *executors{};*/      /* executors from optimizer */        \
    /*AlifCoCached *_cached{};*/      /* cached co_* attributes */                 \
    uintptr_t _instrumentationVersion{}; /* current instrumentation version */ \
    /*AlifCoMonitoringData *_monitoring{};*/ /* Monitoring data */                 \
    AlifIntT _firsttraceable{};       /* index of first traceable instruction */   \
    /* Scratch space for extra data relating to the code object.               \
       Type is a void* to keep the format private in codeobject.c to force     \
       people to go through the proper APIs. */                                \
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





AlifObject* alifCode_constantKey(AlifObject*); // 309
