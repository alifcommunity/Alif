#pragma once

// alif
// 11
#define ALIF_COMMON_FIELDS() \
    AlifObject *globals{}; \
    AlifObject *builtins{}; \
    AlifObject *name{}; \
    AlifObject *qualname{}; \
    AlifObject *code{};         \
    AlifObject *defaults{};   \
    AlifObject *kwDefaults{};  \
    AlifObject *closure{};     

class AlifFrameConstructor { // 21
public:
	ALIF_COMMON_FIELDS()
};

class AlifFunctionObject{ // 36
public:
	ALIFOBJECT_HEAD{};
	ALIF_COMMON_FIELDS()
	AlifObject* doc{};
	AlifObject* dict{};
	AlifObject* weakRefList{};
	AlifObject* module{};
	AlifObject* annotations{};
	AlifObject* annotate{};
	AlifObject* typeParams{};
	VectorCallFunc vectorCall{};
	uint32_t version{};
};
