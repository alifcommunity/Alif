#pragma once


extern AlifTypeObject _alifSetType_; // 9
extern AlifTypeObject _alifFrozenSetType_; // 10

AlifIntT alifSet_add(AlifObject* , AlifObject* ); // 16
AlifIntT alifSet_contains(AlifObject* , AlifObject*); // 18


// 24
#define ALIFFROZENSET_CHECK(_ob) \
    (ALIF_IS_TYPE((_ob), &_alifFrozenSetType_) or \
      alifType_isSubType(ALIF_TYPE(_ob), &_alifFrozenSetType_))


// 30
#define ALIFANYSET_CHECK(_ob) \
(ALIF_IS_TYPE((_ob), &_alifSetType_) or ALIF_IS_TYPE((_ob), &_alifFrozenSetType_) or \
	alifType_isSubType(ALIF_TYPE(_ob), &_alifSetType_) or \
	alifType_isSubType(ALIF_TYPE(_ob), &_alifFrozenSetType_))

// 36
#define ALIFSET_CHECK(_ob) \
    (ALIF_IS_TYPE((_ob), &_alifSetType_) or \
    alifType_isSubType(ALIF_TYPE(_ob), &_alifSetType_))



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
