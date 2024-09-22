#pragma once


extern AlifTypeObject _alifDictType_; // 15


// 17
#define ALIFDICT_CHECK(_op) \
                 ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_DICT_SUBCLASS)
//19
#define ALIFDICT_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifDictType_)


AlifObject* alifDict_new(); // 21

AlifIntT alifDict_setItem(AlifObject*, AlifObject*, AlifObject*); // 24
AlifIntT alifDict_delItem(AlifObject*, AlifObject*); // 25
AlifIntT alifDict_contains(AlifObject*, AlifObject*); // 34

AlifIntT alifDict_getItemRef(AlifObject* , AlifObject* , AlifObject** ); // 67


AlifObject* alifObject_genericGetDict(AlifObject*, void*); // 72


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


AlifIntT alifDict_setDefaultRef(AlifObject*, AlifObject*, AlifObject*, AlifObject**); // 53

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
