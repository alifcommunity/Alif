#pragma once


extern AlifTypeObject _alifDictType_; // 15

AlifObject* alifDict_new(); // 21





/* ---------------------------------------------------------------------------------------------------------------- */











typedef class DictKeysObject AlifDictKeysObject; // 5
typedef class DictValues AlifDictValues; // 6


class AlifDictObject { // 11
public:
	ALIFOBJECT_HEAD{};

	AlifSizeT used{};
	uint64_t versionTag{};

	AlifDictKeysObject* keys{};

	AlifDictValues* values{};
};



// 78
#define ALIF_FOREACH_DICT_EVENT(V) \
    V(Added)                     \
    V(Modified)                  \
    V(Deleted)                   \
    V(Cloned)                    \
    V(Cleared)                   \
    V(Deallocated)

enum AlifDictWatchEvent_ { // 86
#define ALIF_DEF_EVENT(_event) AlifDict_Event_##_event,
	ALIF_FOREACH_DICT_EVENT(ALIF_DEF_EVENT)
#undef ALIF_DEF_EVENT
};


typedef AlifIntT(*AlifDictWatchCallback)(AlifDictWatchEvent_ _event,
	AlifObject* _dict, AlifObject* _key, AlifObject* _newValue); // 95
