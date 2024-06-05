#pragma once

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


extern AlifInitObject typeDict;

AlifObject* alifNew_dict();
AlifDictObject* deleteItem_fromIndex(AlifDictObject*, int64_t);
AlifIntT dict_deleteItem(AlifDictObject*, AlifObject*);
int alifDict_getItemRef(AlifObject*, AlifObject*, AlifObject**);
bool alifDict_next(AlifObject*, int64_t*, AlifObject**, AlifObject**, size_t*);
AlifObject* alifDict_keys(AlifObject*);
AlifIntT dict_setItem(AlifDictObject* , AlifObject*, AlifObject* );
AlifIntT dict_ass_sub(AlifDictObject*, AlifObject*, AlifObject*);
int dict_lookupItem(AlifDictObject*, AlifObject*, size_t, AlifObject** );
AlifObject* dict_getItem(AlifObject* , AlifObject* );
bool dict_contain(AlifObject* , AlifObject* ); 
AlifDictObject* dict_resize(AlifDictObject* );
AlifDictObject* deleteItem_fromIndex(AlifDictObject* , int64_t );
