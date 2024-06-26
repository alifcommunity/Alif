#pragma once

extern AlifInitObject _alifDictType_;


#define ALIFDICT_CHECK(op) ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(op), ALIFTPFLAGS_DICT_SUBCLASS)
#define ALIFDICT_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifDictType_)


class AlifDictValues {
public:
	size_t hash{};
	//uint8_t kind;

	AlifObject* key{};
	AlifObject* value{};

};

class AlifDictObject{
public:
	ALIFOBJECT_HEAD

	AlifSizeT size_{};
	AlifSizeT capacity_{};
	AlifDictValues* items_{};
};



#define ALIFDICT_GET_SIZE(_op) (((AlifDictObject*)_op)->size_)



AlifObject* alifNew_dict();
AlifDictObject* deleteItem_fromIndex(AlifDictObject*, int64_t);
AlifIntT dict_deleteItem(AlifDictObject*, AlifObject*);
int alifDict_getItemRef(AlifObject*, AlifObject*, AlifObject**);
bool alifDict_next(AlifObject*, AlifSizeT*, AlifObject**, AlifObject**, AlifUSizeT*);
AlifObject* alifDict_keys(AlifObject*);
AlifIntT dict_setItem(AlifDictObject* , AlifObject*, AlifObject* );
AlifIntT dict_ass_sub(AlifDictObject*, AlifObject*, AlifObject*);
int dict_lookupItem(AlifDictObject*, AlifObject*, size_t, AlifObject** );
AlifObject* dict_getItem(AlifObject* , AlifObject* );
bool dict_contain(AlifObject* , AlifObject* ); 
AlifDictObject* dict_resize(AlifDictObject* );
AlifDictObject* deleteItem_fromIndex(AlifDictObject* , int64_t );
