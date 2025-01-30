#pragma once



void alifErr_setObject(AlifObject*, AlifObject*); // 10
void alifErr_setString(AlifObject*, const char*); // 11


AlifObject* alifErr_occurred(void); // 15



AlifIntT alifErr_exceptionMatches(AlifObject*); // 39


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

extern AlifObject* _alifExcBaseException_; // 76
extern AlifObject* _alifExcException_; // 77


extern AlifObject* _alifExcImportError_; // 93
extern AlifObject* _alifExcIndexError_; // 97



extern AlifObject* _alifExcOverflowError_; // 102

extern AlifObject* _alifExcSyntaxError_; // 108
extern AlifObject* _alifExcSystemError_; // 112
extern AlifObject* _alifExcTypeError_; // 114




AlifObject* alifErr_format(AlifObject*, const char*, ...); // 180






/* --------------------------------------------------------------------------------- */

 // 8
#define ALIFEXCEPTION_HEAD ALIFOBJECT_HEAD; AlifObject *dict;\
             AlifObject *args; AlifObject *notes; AlifObject *traceback;\
             AlifObject *context; AlifObject *cause;\
             char suppressContext;

class AlifBaseExceptionObject { // 13
public:
	ALIFEXCEPTION_HEAD
};



class AlifSyntaxErrorObject { // 23
public:
	ALIFEXCEPTION_HEAD
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
