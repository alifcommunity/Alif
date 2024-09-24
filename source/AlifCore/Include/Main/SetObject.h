#pragma once


AlifTypeObject _alifSetType_; // 9
AlifTypeObject _alifFrozenSetType_; // 10


// 30
#define ALIFANYSET_CHECK(_ob) \
(ALIF_IS_TYPE((_ob), &_alifSetType_) or ALIF_IS_TYPE((_ob), &_alifFrozenSetType_) or \
	alifType_isSubType(ALIF_TYPE(_ob), &_alifSetType_) or \
	alifType_isSubType(ALIF_TYPE(_ob), &_alifFrozenSetType_))




/*-------------------------------------------------------------------------------------------------------------------------------------*/


#define ALIFSET_MINSIZE 8 // 18

class SetEntry{ // 20
public:
	AlifObject* key{};
	AlifHashT hash{};         
};


class AlifSetObject{ // 36
public:
	ALIFOBJECT_HEAD;
	AlifSizeT fill{};          
	AlifSizeT used{};
	AlifSizeT mask{};
	SetEntry* table{};
	AlifHashT hash{};
	AlifSizeT finger{};
	SetEntry smallTable[ALIFSET_MINSIZE]{};
	AlifObject* weakreList{};
};
