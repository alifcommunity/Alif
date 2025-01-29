#pragma once


extern AlifTypeObject _alifDictType_; // 15


// 17
#define ALIFDICT_CHECK(_op) \
                 ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_DICT_SUBCLASS)
//19
#define ALIFDICT_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifDictType_)


AlifObject* alifDict_new(); // 21
AlifObject* alifDict_getItemWithError(AlifObject*, AlifObject*); // 23
AlifIntT alifDict_setItem(AlifObject*, AlifObject*, AlifObject*); // 24
AlifIntT alifDict_delItem(AlifObject*, AlifObject*); // 25
AlifIntT alifDict_next(AlifObject* , AlifSizeT* , AlifObject** , AlifObject** ); // 27

AlifObject* alifDict_keys(AlifObject*); // 29
AlifSizeT alifDict_size(AlifObject*); // 32
AlifObject* alifDict_copy(AlifObject*); // 33
AlifIntT alifDict_contains(AlifObject*, AlifObject*); // 34

AlifIntT alifDict_setItemString(AlifObject*, const char*, AlifObject*); // 58
AlifIntT alifDict_delItemString(AlifObject*, const char* ); // 59

AlifIntT alifDict_getItemRef(AlifObject* , AlifObject* , AlifObject** ); // 67

AlifIntT alifDict_getItemStringRef(AlifObject* , const char* , AlifObject** ); // 68


AlifObject* alifObject_genericGetDict(AlifObject*, void*); // 72




extern AlifTypeObject _alifDictKeysType_; // 77
extern AlifTypeObject _alifDictValuesType_;
extern AlifTypeObject _alifDictItemsType_;


extern AlifTypeObject _alifDictIterKeyType_; // 90

extern AlifTypeObject _alifDictIterItemType_; // 92

extern AlifTypeObject _alifDictRevIterKeyType_; // 94
extern AlifTypeObject _alifDictRevIterItemType_; // 95
extern AlifTypeObject _alifDictRevIterValueType_; // 96

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




AlifObject* _alifDict_getItemKnownHash(AlifObject*, AlifObject*, AlifHashT); // 38


AlifIntT alifDict_setDefaultRef(AlifObject*, AlifObject*, AlifObject*, AlifObject**); // 53

static inline AlifSizeT _alifDict_getSize(AlifObject* _op) { // 56
	AlifDictObject* mp_{};
	mp_ = ALIF_CAST(AlifDictObject*, _op);
	return alifAtomic_loadSizeRelaxed(&mp_->used);
}
#define ALIFDICT_GET_SIZE(_op) _alifDict_getSize(ALIFOBJECT_CAST(_op))


AlifIntT alifDict_containsString(AlifObject*, const char*); // 68

AlifIntT alifDict_pop(AlifObject*, AlifObject*, AlifObject**); // 72
AlifIntT alifDict_popString(AlifObject*, const char*, AlifObject**); // 73


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
