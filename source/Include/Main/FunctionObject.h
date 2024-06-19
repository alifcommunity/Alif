#pragma once

#define COMMON_FIELDS(PREFIX) \
    AlifObject *PREFIX ## globals; \
    AlifObject *PREFIX ## builtins; \
    AlifObject *PREFIX ## name; \
    AlifObject *PREFIX ## qualname; \
    AlifObject *PREFIX ## code;        /* A code object, the __code__ attribute */ \
    AlifObject *PREFIX ## defaults;    /* nullptr or a tuple */ \
    AlifObject *PREFIX ## kwdefaults;  /* nullptr or a dict */ \
    AlifObject *PREFIX ## closure;     /* nullptr or a tuple of cell objects */

class AlifFrameConstructor {
public:
	COMMON_FIELDS(fc)
};


class AlifFunctionObject {
public:
    AlifObject* object;
    COMMON_FIELDS(func_)
    AlifObject* funcDict;        /* The __dict__ attribute, a dict or NULL */
    AlifObject* funcWeakRefList; /* List of weak references */
    AlifObject* funcModule;      /* The __module__ attribute, can be anything */
    AlifObject* funcTypeParams;  /* Tuple of active type_ variables or NULL */
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
     *     (func_closure may be NULL if alifCode_GetNumFree(func_code) == 0).
     */
} ;
