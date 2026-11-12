#pragma once



ALIFAPI_FUNC(void) alifErr_setObject(AlifObject*, AlifObject*); // 10
ALIFAPI_FUNC(void) alifErr_setString(AlifObject*, const char*); // 11


ALIFAPI_FUNC(AlifObject*) alifErr_occurred(void); // 15
ALIFAPI_FUNC(void) alifErr_clear(void); // 16
ALIFAPI_FUNC(void) alifErr_fetch(AlifObject**, AlifObject**, AlifObject**); // 17
ALIFAPI_FUNC(void) alifErr_restore(AlifObject*, AlifObject*, AlifObject*); // 18
ALIFAPI_FUNC(AlifObject*) alifErr_getRaisedException(void); // 19
ALIFAPI_FUNC(void) alifErr_setRaisedException(AlifObject*); // 20
ALIFAPI_FUNC(void) alifErr_setHandledException(AlifObject*); // 23

ALIFAPI_FUNC(void) ALIF_NO_RETURN alif_fatalError(const char* _msg); // 35

ALIFAPI_FUNC(AlifIntT) alifErr_givenExceptionMatches(AlifObject*, AlifObject*); // 38
ALIFAPI_FUNC(AlifIntT) alifErr_exceptionMatches(AlifObject*); // 39

ALIFAPI_FUNC(AlifIntT) alifException_setTraceback(AlifObject*, AlifObject*); // 43
ALIFAPI_FUNC(AlifObject*) alifException_getTraceback(AlifObject*); // 44

ALIFAPI_FUNC(AlifObject*) alifException_getCause(AlifObject*); // 47
ALIFAPI_FUNC(void) alifException_setCause(AlifObject*, AlifObject*); // 48

ALIFAPI_FUNC(AlifObject*) alifException_getContext(AlifObject*); // 51
ALIFAPI_FUNC(void) alifException_setContext(AlifObject*, AlifObject*); // 52


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

ALIFAPI_DATA(AlifObject*) _alifExcBaseException_; // 76
ALIFAPI_DATA(AlifObject*) _alifExcException_; // 77
ALIFAPI_DATA(AlifObject*) _alifExcBaseExceptionGroup_; // 77

ALIFAPI_DATA(AlifObject*) _alifExcStopAsyncIteration_; // 80

ALIFAPI_DATA(AlifObject*) _alifExcStopIteration_; // 82
ALIFAPI_DATA(AlifObject*) _alifExcAttributeError_; // 88
ALIFAPI_DATA(AlifObject*) _alifExcOSError_; // 92
ALIFAPI_DATA(AlifObject*) _alifExcImportError_; // 93

ALIFAPI_DATA(AlifObject*) _alifExcIndexError_; // 97
ALIFAPI_DATA(AlifObject*) _alifExcKeyError_; // 98



ALIFAPI_DATA(AlifObject*) _alifExcNameError_; // 101
ALIFAPI_DATA(AlifObject*) _alifExcOverflowError_; // 102
ALIFAPI_DATA(AlifObject*) _alifExcRuntimeError_; // 103
ALIFAPI_DATA(AlifObject*) _alifExcNotImplementedError; // 107
ALIFAPI_DATA(AlifObject*) _alifExcSyntaxError_; // 108
ALIFAPI_DATA(AlifObject*) _alifExcIndentationError_; // 109
ALIFAPI_DATA(AlifObject*) _alifExcSystemError_; // 112
ALIFAPI_DATA(AlifObject*) _alifExcSystemExit_; // 113
ALIFAPI_DATA(AlifObject*) _alifExcTypeError_; // 114

ALIFAPI_DATA(AlifObject*) _alifExcUnicodeEncodeError_; // 117

ALIFAPI_DATA(AlifObject*) _alifExcValueError_; // 120

ALIFAPI_DATA(AlifObject*) _alifExcBlockingIOError_; // 124


ALIFAPI_FUNC(AlifObject*) alifErr_setFromErrnoWithFilenameObject(AlifObject*, AlifObject*);
ALIFAPI_FUNC(AlifObject*) alifErr_setFromErrnoWithFilenameObjects(AlifObject*, AlifObject*, AlifObject*);


ALIFAPI_FUNC(AlifObject*) alifErr_format(AlifObject*, const char*, ...); // 180



#ifdef _WINDOWS // 192

ALIFAPI_FUNC(AlifObject*) alifErr_setExcFromWindowsErrWithFilenameObject(
	AlifObject*, AlifIntT, AlifObject*);
ALIFAPI_FUNC(AlifObject*) alifErr_setExcFromWindowsErrWithFilenameObjects(
	AlifObject*, AlifIntT, AlifObject*, AlifObject*);

#endif /* _WINDOWS */ // 210



ALIFAPI_FUNC(AlifObject*) alifErr_setImportError(AlifObject*, AlifObject*, AlifObject*); // 217


ALIFAPI_FUNC(AlifObject*) alifErr_newException(const char*, AlifObject*, AlifObject*); // 229



ALIFAPI_FUNC(AlifIntT) alifUnicodeEncodeError_setStart(AlifObject*, AlifSizeT); // 285

ALIFAPI_FUNC(AlifIntT) alifUnicodeEncodeError_setEnd(AlifObject*, AlifSizeT); // 297


ALIFAPI_FUNC(AlifIntT) alifUnicodeEncodeError_setReason(AlifObject*, const char*); // 308


// 320
ALIFAPI_FUNC(AlifIntT) alifOS_snprintf(char*, AlifUSizeT, const char*, ...)
ALIF_GCC_ATTRIBUTE((format(printf, 3, 4)));
ALIFAPI_FUNC(AlifIntT) alifOS_vsnprintf(char*, AlifUSizeT, const char*, va_list)
ALIF_GCC_ATTRIBUTE((format(printf, 3, 0)));



/* --------------------------------------------------------------------------------- */

 // 8
#define ALIFEXCEPTION_HEAD ALIFOBJECT_HEAD; AlifObject *dict{};\
             AlifObject *args{}; AlifObject *notes{}; AlifObject *traceback{};\
             AlifObject *context{}; AlifObject *cause{};\
             char suppressContext{};

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


class AlifUnicodeErrorObject { // 43
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* encoding{};
	AlifObject* object{};
	AlifSizeT start{};
	AlifSizeT end{};
	AlifObject* reason{};
};


class AlifSystemExitObject { // 52
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* code{};
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

class AlifNameErrorObject { // 74
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* name{};
};

class AlifAttributeErrorObject { // 79
public:
	ALIFEXCEPTION_HEAD;
	AlifObject* obj{};
	AlifObject* name{};
};




ALIFAPI_FUNC(void) _alifErr_chainExceptions1(AlifObject*); // 93


ALIFAPI_FUNC(void) alifErr_rangedSyntaxLocationObject(AlifObject*, AlifIntT,
	AlifIntT, AlifIntT, AlifIntT); // 112


ALIFAPI_FUNC(AlifObject*) alifErr_programTextObject(AlifObject*, AlifIntT); // 119
