#pragma once

// 11
#define ALIF_COMMON_FIELDS(__prefix) \
    AlifObject *_prefix ## globals; \
    AlifObject *_prefix ## builtins; \
    AlifObject *_prefix ## name; \
    AlifObject *_prefix ## qualname; \
    AlifObject *_prefix ## code;         \
    AlifObject *_prefix ## defaults;   \
    AlifObject *_prefix ## kwdefaults;  \
    AlifObject *_prefix ## closure;     

class AlifFrameConstructor { // 21
public:
	ALIF_COMMON_FIELDS(fc_{})
};

class AlifFunctionObject{ // 36
public:
	ALIFOBJECT_HEAD;
	ALIF_COMMON_FIELDS(func_{})
	AlifObject* funcDoc{};         /* The __doc__ attribute, can be anything */
	AlifObject* funcDict{};        /* The __dict__ attribute, a dict or NULL */
	AlifObject* funcWeakRefList{}; /* List of weak references */
	AlifObject* funcModule{};      /* The __module__ attribute, can be anything */
	AlifObject* funcAnnotations{}; /* Annotations, a dict or NULL */
	AlifObject* funcAnnotate{};    /* Callable to fill the annotations dictionary */
	AlifObject* funcTypeParams{};  /* Tuple of active type variables or NULL */
	VectorCallFunc vectorCall{};
	uint32_t funcVersion{};
};
