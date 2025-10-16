#pragma once

#include "AlifCore_Frame.h"










#define ALIFGENOBJECT_HEAD(prefix)                                           \
    ALIFOBJECT_HEAD;                                                           \
    /* List of weak reference. */                                           \
    AlifObject *prefix##WeakRefList;                                         \
    /* Name of the generator. */                                            \
    AlifObject *prefix##Name;                                                \
    /* Qualified name of the generator. */                                  \
    AlifObject *prefix##Qualname;                                            \
    AlifErrStackItem prefix##ExcState;                                    \
    AlifObject *prefix##OriginOrFinalizer;                                 \
    char prefix##HooksInited;                                             \
    char prefix##Closed;                                                   \
    char prefix##RunningAsync;                                            \
    /* The frame */                                                         \
    int8_t prefix##FrameState;                                            \
    AlifInterpreterFrame prefix##IFrame;                             \


class AlifGenObject {
public:
	ALIFGENOBJECT_HEAD(gi)
};
class AlifCoroObject {
public:
	ALIFGENOBJECT_HEAD(cr)
};
class AlifAsyncGenObject {
public:
	ALIFGENOBJECT_HEAD(ag)
};



static inline AlifGenObject* _alifGen_getGeneratorFromFrame(AlifInterpreterFrame* _frame) {
	AlifUSizeT offsetInGen = offsetof(AlifGenObject, giIFrame);
	return (AlifGenObject*)(((char*)_frame) - offsetInGen);
}




AlifIntT _alifGen_setStopIterationValue(AlifObject*); // 59
