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

class AlifFunctionObject { // 36
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










extern AlifTypeObject _alifFunctionType_; // 66

// 132
#define ALIF_FOREACH_FUNC_EVENT(_v) \
    _v(Create)                    \
    _v(Destroy)                   \
    _v(Modify_Code)               \
    _v(Modify_Defaults)           \
    _v(Modify_KWDefaults)

enum AlifFunctionWatchEvent { // 139
#define ALIF_DEF_EVENT(_Event) AlifFunction_Event_##_Event,
	ALIF_FOREACH_FUNC_EVENT(ALIF_DEF_EVENT)
#undef ALIF_DEF_EVENT
};



typedef AlifIntT (*AlifFunctionWatchCallback)(AlifFunctionWatchEvent,
	AlifFunctionObject* , AlifObject* ); // 160
