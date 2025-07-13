#pragma once



void alifErr_setObject(AlifObject*, AlifObject*); // 10
void alifErr_setString(AlifObject*, const char*); // 11


AlifObject* alifErr_occurred(void); // 15
void alifErr_clear(void); // 16
void alifErr_fetch(AlifObject**, AlifObject**, AlifObject**); // 17
void alifErr_restore(AlifObject*, AlifObject*, AlifObject*); // 18
AlifObject* alifErr_getRaisedException(void); // 19
void alifErr_setRaisedException(AlifObject*); // 20
void alifErr_setHandledException(AlifObject*); // 23

AlifIntT alifErr_givenExceptionMatches(AlifObject*, AlifObject*); // 38
AlifIntT alifErr_exceptionMatches(AlifObject*); // 39

AlifIntT alifException_setTraceback(AlifObject*, AlifObject*); // 43
AlifObject* alifException_getTraceback(AlifObject*); // 44


AlifObject* alifException_getContext(AlifObject*); // 51
void alifException_setContext(AlifObject*, AlifObject*); // 52


 // 60
#define ALIFEXCEPTIONCLASS_CHECK(_x)  (ALIFTYPE_CHECK(_x)	\
			and ALIFTYPE_FASTSUBCLASS((AlifTypeObject*)(_x), ALIF_TPFLAGS_BASE_EXC_SUBCLASS))

 // 64
#define ALIFEXCEPTIONINSTANCE_CHECK(_x)                    \
    ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_x), ALIF_TPFLAGS_BASE_EXC_SUBCLASS)

#define ALIFEXCEPTIONINSTANCE_CLASS(_x) ALIFOBJECT_CAST(ALIF_TYPE(_x))

// 71
#define ALIFBASEEXCEPTIONGROUP_CHECK(x)                   \
    ALIFOBJECT_TYPECHECK((x), (AlifTypeObject*)_alifExcBaseExceptionGroup_)

extern AlifObject* _alifExcBaseException_; // 76
extern AlifObject* _alifExcException_; // 77
extern AlifObject* _alifExcBaseExceptionGroup_; // 77

extern AlifObject* _alifExcStopAsyncIteration_; // 80

extern AlifObject* _alifExcStopIteration_; // 82
extern AlifObject* _alifExcOSError_; // 92
extern AlifObject* _alifExcImportError_; // 93
extern AlifObject* _alifExcIndexError_; // 97



extern AlifObject* _alifExcOverflowError_; // 102

extern AlifObject* _alifExcSyntaxError_; // 108
extern AlifObject* _alifExcIndentationError_; // 109
extern AlifObject* _alifExcSystemError_; // 112
extern AlifObject* _alifExcTypeError_; // 114

extern AlifObject* _alifExcValueError_; // 120

extern AlifObject* _alifExcBlockingIOError_; // 124


AlifObject* alifErr_setFromErrnoWithFilenameObject(AlifObject*, AlifObject*);
AlifObject* alifErr_setFromErrnoWithFilenameObjects(AlifObject*, AlifObject*, AlifObject*);


AlifObject* alifErr_format(AlifObject*, const char*, ...); // 180



#ifdef _WINDOWS // 192

AlifObject* alifErr_setExcFromWindowsErrWithFilenameObject(
	AlifObject*, AlifIntT, AlifObject*);
AlifObject* alifErr_setExcFromWindowsErrWithFilenameObjects(
	AlifObject*, AlifIntT, AlifObject*, AlifObject*);

#endif /* _WINDOWS */ // 210




/* --------------------------------------------------------------------------------- */

 // 8
#define ALIFEXCEPTION_HEAD ALIFOBJECT_HEAD; AlifObject *dict;\
             AlifObject *args; AlifObject *notes; AlifObject *traceback;\
             AlifObject *context; AlifObject *cause;\
             char suppressContext;

class AlifBaseExceptionObject { // 13
public:
	ALIFEXCEPTION_HEAD;
};

class AlifBaseExceptionGroupObject { // 17
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* msg{};
	AlifObject* excs{};
};

class AlifSyntaxErrorObject { // 23
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* msg{};
	AlifObject* filename{};
	AlifObject* lineno{};
	AlifObject* offset{};
	AlifObject* endLineno{};
	AlifObject* endOffset{};
	AlifObject* text{};
	AlifObject* printFileAndLine{};
};



class AlifImportErrorObject { // 35
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* msg{};
	AlifObject* name{};
	AlifObject* path{};
	AlifObject* nameFrom{};
};


class AlifOSErrorObject { // 57
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* myErrno{};
	AlifObject* strError{};
	AlifObject* fileName{};
	AlifObject* fileName2{};
#ifdef _WINDOWS
	AlifObject* winError{};
#endif
	AlifSizeT written{};
};


class AlifStopIterationObject { // 69
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* value{};
};






void _alifErr_chainExceptions1(AlifObject*); // 93
