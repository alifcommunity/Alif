#pragma once

#define COMMON_FIELDS(PREFIX) \
    AlifObject *PREFIX ## Globals; \
    AlifObject *PREFIX ## Builtins; \
    AlifObject *PREFIX ## Name; \
    AlifObject *PREFIX ## Qualname; \
    AlifObject *PREFIX ## Code;        /* A code object, the __code__ attribute */ \
    AlifObject *PREFIX ## Defaults;    /* nullptr or a tuple */ \
    AlifObject *PREFIX ## Kwdefaults;  /* nullptr or a dict */ \
    AlifObject *PREFIX ## Closure;     /* nullptr or a tuple of cell objects */

class AlifFrameConstructor {
public:
	COMMON_FIELDS(fc)
};


class AlifFunctionObject {
public:
    ALIFOBJECT_HEAD;
	COMMON_FIELDS(func)
	AlifObject* funcDoc;         /* The __doc__ attribute, can be anything */
    AlifObject* funcDict;        /* The __dict__ attribute, a dict or nullptr */
    AlifObject* funcWeakRefList; /* List of weak references */
    AlifObject* funcModule;      /* The __module__ attribute, can be anything */
    AlifObject* funcTypeParams;  /* Tuple of active type_ variables or nullptr */
    VectorCallFunc vectorCall;
    /* Version number for use by specializer.
     * Can set to non-zero when we want to specialize.
     * Will be set to zero if any of these change:
     *     defaults
     *     kwdefaults (only if the object changes, not the contents of the dict)
     *     code
     *     annotations
     *     vectorcall function pointer */
    uint32_t funcVersion;

    /* Invariant:
     *     func_closure contains the bindings for func_code->co_freevars, so
     *     alifTuple_Size(func_closure) == alifCode_GetNumFree(func_code)
     *     (func_closure may be nullptr if alifCode_GetNumFree(func_code) == 0).
     */
} ;


extern AlifTypeObject _alifFunctionType_;


#define ALIF_FOREACH_FUNC_EVENT(_v) \
    _v(Create)                    \
    _v(Destroy)                   \
    _v(Modify_Code)               \
    _v(Modify_Defaults)           \
    _v(Modify_Kwdefaults)

enum AlifFunctionWatchEvent {
#define ALIF_DEF_EVENT(EVENT) AlifFunction_Event_##EVENT,
	ALIF_FOREACH_FUNC_EVENT(ALIF_DEF_EVENT)
#undef ALIF_DEF_EVENT
};
