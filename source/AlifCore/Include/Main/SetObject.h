#pragma once


ALIFAPI_DATA(AlifTypeObject) _alifSetType_; // 9
ALIFAPI_DATA(AlifTypeObject) _alifFrozenSetType_; // 10


ALIFAPI_FUNC(AlifObject*) alifSet_new(AlifObject*); // 13
ALIFAPI_FUNC(AlifObject*) alifFrozenSet_new(AlifObject*); // 14

ALIFAPI_FUNC(AlifIntT) alifSet_add(AlifObject* ,AlifObject* ); // 16
ALIFAPI_FUNC(AlifIntT) alifSet_contains(AlifObject* ,AlifObject*); // 18
ALIFAPI_FUNC(AlifIntT) alifSet_discard(AlifObject* ,AlifObject* ); // 19
ALIFAPI_FUNC(AlifObject*) alifSet_pop(AlifObject*); // 20
ALIFAPI_FUNC(AlifSizeT) alifSet_size(AlifObject*); // 21


#define ALIFFROZENSET_CHECKEXACT(_ob) ALIF_IS_TYPE((_ob), &_alifFrozenSetType_) // 23
// 24
#define ALIFFROZENSET_CHECK(_ob) \
    (ALIF_IS_TYPE((_ob), &_alifFrozenSetType_) or \
      alifType_isSubType(ALIF_TYPE(_ob), &_alifFrozenSetType_))

//28
#define ALIFANYSET_CHECKEXACT(_ob) \
    (ALIF_IS_TYPE((_ob), &_alifSetType_) or ALIF_IS_TYPE((_ob), &_alifFrozenSetType_))
// 30
#define ALIFANYSET_CHECK(_ob) \
(ALIF_IS_TYPE((_ob), &_alifSetType_) or ALIF_IS_TYPE((_ob), &_alifFrozenSetType_) or \
	alifType_isSubType(ALIF_TYPE(_ob), &_alifSetType_) or \
	alifType_isSubType(ALIF_TYPE(_ob), &_alifFrozenSetType_))

// 35
#define ALIFSET_CHECKEXACT(_op) ALIF_IS_TYPE(_op, &_alifSetType_)
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
	AlifObject* weakRefList{};
};


#define ALIFSET_CAST(_so) ALIF_CAST(AlifSetObject*, _so) // 61

static inline AlifSizeT _alifSet_GetSize(AlifObject* so) { // 64
	return alifAtomic_loadSizeRelaxed(&(ALIFSET_CAST(so)->used));
}
#define ALIFSET_GET_SIZE(so) _alifSet_GetSize(ALIFOBJECT_CAST(so))
