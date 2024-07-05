#pragma once

extern AlifInitObject _alifDictType_;


#define ALIFDICT_CHECK(op) ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(op), ALIFTPFLAGS_DICT_SUBCLASS)
#define ALIFDICT_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifDictType_)

AlifObject* alifNew_dict();
AlifObject* alifDict_getItem(AlifObject* , AlifObject* );
int alifDict_getItemRef(AlifObject* , AlifObject* , AlifObject** );
int alifDict_setItem(AlifObject*, AlifObject*, AlifObject* );
int alifDict_next(AlifObject* , int64_t* , AlifObject** , AlifObject** );
AlifObject* alifDict_keys(AlifObject* );


typedef class DictKeysObject AlifDictKeysObject;
typedef class DictValues AlifDictValues;

class AlifDictObject {
public:
	ALIFOBJECT_HEAD

	int64_t used;

	uint64_t versionTag;

	AlifDictKeysObject* keys;

	AlifDictValues* values;
} ;

AlifObject* alifDictGetItem_knownHash(AlifObject* , AlifObject* , size_t );

#define ALIFDICT_GET_SIZE(_op) (((AlifDictObject*)_op)->used)

int alifDict_containsString(AlifObject*, const wchar_t*);

#define ALIF_FOREACH_DICT_EVENT(V) \
    V(ADDED)                     \
    V(MODIFIED)                  \
    V(DELETED)                   \
    V(CLONED)                    \
    V(CLEARED)                   \
    V(DEALLOCATED)

 enum AlifDictWatchEvent {
#define ALIF_DEF_EVENT(EVENT) AlifDict_Even_##EVENT,
	ALIF_FOREACH_DICT_EVENT(ALIF_DEF_EVENT)
#undef ALIF_DEF_EVENT
} ;

//AlifObject* alifNew_dict();
//AlifDictObject* deleteItem_fromIndex(AlifDictObject*, int64_t);
//AlifIntT dict_deleteItem(AlifDictObject*, AlifObject*);
//int alifDict_getItemRef(AlifObject*, AlifObject*, AlifObject**);
//bool alifDict_next(AlifObject*, AlifSizeT*, AlifObject**, AlifObject**, AlifUSizeT*);
//AlifObject* alifDict_keys(AlifObject*);
//AlifIntT dict_setItem(AlifDictObject* , AlifObject*, AlifObject* );
//AlifIntT dict_ass_sub(AlifDictObject*, AlifObject*, AlifObject*);
//int dict_lookupItem(AlifDictObject*, AlifObject*, size_t, AlifObject** );
//AlifObject* dict_getItem(AlifObject* , AlifObject* );
//bool dict_contain(AlifObject* , AlifObject* ); 
//AlifDictObject* dict_resize(AlifDictObject* );
//AlifDictObject* deleteItem_fromIndex(AlifDictObject* , int64_t );
