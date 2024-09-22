#pragma once

extern AlifTypeObject _alifListType_; // 20


// 24
#define ALIFLIST_CHECK(_op) \
    ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_LIST_SUBCLASS)
#define ALIFLIST_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifListType_)



AlifObject* alifList_new(AlifSizeT); // 28

AlifIntT alifList_append(AlifObject*, AlifObject*); // 37


/* -------------------------------------------------------------------------------------- */



class AlifListObject { //  5
public:
	ALIFOBJECT_VAR_HEAD{};
	AlifObject** item{};

	AlifSizeT allocated{};
};


#define ALIFLIST_CAST(_op) \
    (ALIF_CAST(AlifListObject*, (_op))) // 25





static inline void alifList_SetItem(AlifObject* _op,
	AlifSizeT _index, AlifObject* _value) { // 42
	AlifListObject* list = ALIFLIST_CAST(_op);
	list->item[_index] = _value;
}
#define ALIFLIST_SET_ITEM(_op, _index, _value) \
    alifList_SetItem(ALIFOBJECT_CAST(_op), (_index), ALIFOBJECT_CAST(_value))
