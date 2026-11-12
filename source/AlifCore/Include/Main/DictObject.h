#pragma once


ALIFAPI_DATA(AlifTypeObject) _alifDictType_; // 15


// 17
#define ALIFDICT_CHECK(_op) \
                 ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_DICT_SUBCLASS)
//19
#define ALIFDICT_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifDictType_)


ALIFAPI_FUNC(AlifObject*) alifDict_new(); // 21
ALIFAPI_FUNC(AlifObject*) alifDict_getItem(AlifObject*, AlifObject*); // 22
ALIFAPI_FUNC(AlifObject*) alifDict_getItemWithError(AlifObject*, AlifObject*); // 23
ALIFAPI_FUNC(AlifIntT) alifDict_setItem(AlifObject*, AlifObject*, AlifObject*); // 24
ALIFAPI_FUNC(AlifIntT) alifDict_delItem(AlifObject*, AlifObject*); // 25
ALIFAPI_FUNC(AlifIntT) alifDict_next(AlifObject* , AlifSizeT* , AlifObject** , AlifObject** ); // 27

ALIFAPI_FUNC(AlifObject*) alifDict_keys(AlifObject*); // 29
ALIFAPI_FUNC(AlifSizeT) alifDict_size(AlifObject*); // 32
ALIFAPI_FUNC(AlifObject*) alifDict_copy(AlifObject*); // 33
ALIFAPI_FUNC(AlifIntT) alifDict_contains(AlifObject*, AlifObject*); // 34

ALIFAPI_FUNC(AlifIntT) alifDict_update(AlifObject*, AlifObject*); // 37

ALIFAPI_FUNC(AlifIntT) alifDict_setItemString(AlifObject*, const char*, AlifObject*); // 58
ALIFAPI_FUNC(AlifIntT) alifDict_delItemString(AlifObject*, const char* ); // 59

ALIFAPI_FUNC(AlifIntT) alifDict_getItemRef(AlifObject* , AlifObject* , AlifObject** ); // 67

ALIFAPI_FUNC(AlifIntT) alifDict_getItemStringRef(AlifObject* , const char* , AlifObject** ); // 68


ALIFAPI_FUNC(AlifObject*) alifObject_genericGetDict(AlifObject*, void*); // 72




ALIFAPI_DATA(AlifTypeObject) _alifDictKeysType_; // 77
ALIFAPI_DATA(AlifTypeObject) _alifDictValuesType_;
ALIFAPI_DATA(AlifTypeObject) _alifDictItemsType_;


ALIFAPI_DATA(AlifTypeObject) _alifDictIterKeyType_; // 90

ALIFAPI_DATA(AlifTypeObject) _alifDictIterItemType_; // 92

ALIFAPI_DATA(AlifTypeObject) _alifDictRevIterKeyType_; // 94
ALIFAPI_DATA(AlifTypeObject) _alifDictRevIterItemType_; // 95
ALIFAPI_DATA(AlifTypeObject) _alifDictRevIterValueType_; // 96

/* ---------------------------------------------------------------------------------------------------------------- */











typedef class DictKeysObject AlifDictKeysObject; // 5
typedef class DictValues AlifDictValues; // 6


class AlifDictObject { // 11
public:
	ALIFOBJECT_HEAD{};
	AlifSizeT used{};
	uint64_t watcherTag{};
	AlifDictKeysObject* keys{};
	AlifDictValues* values{};
};




ALIFAPI_FUNC(AlifObject*) _alifDict_getItemKnownHash(AlifObject*, AlifObject*, AlifHashT); // 38


ALIFAPI_FUNC(AlifIntT) alifDict_setDefaultRef(AlifObject*, AlifObject*, AlifObject*, AlifObject**); // 53

static inline AlifSizeT _alifDict_getSize(AlifObject* _op) { // 56
	AlifDictObject* mp_{};
	mp_ = ALIF_CAST(AlifDictObject*, _op);
	return alifAtomic_loadSizeRelaxed(&mp_->used);
}
#define ALIFDICT_GET_SIZE(_op) _alifDict_getSize(ALIFOBJECT_CAST(_op))


ALIFAPI_FUNC(AlifIntT) alifDict_containsString(AlifObject*, const char*); // 68

ALIFAPI_FUNC(AlifIntT) alifDict_pop(AlifObject*, AlifObject*, AlifObject**); // 72
ALIFAPI_FUNC(AlifIntT) alifDict_popString(AlifObject*, const char*, AlifObject**); // 73


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
