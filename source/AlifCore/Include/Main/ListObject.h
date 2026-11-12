#pragma once

ALIFAPI_DATA(AlifTypeObject) _alifListType_; // 20


// 24
#define ALIFLIST_CHECK(_op) \
    ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_LIST_SUBCLASS)
#define ALIFLIST_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifListType_)



ALIFAPI_FUNC(AlifObject*) alifList_new(AlifSizeT); // 28
ALIFAPI_FUNC(AlifSizeT) alifList_size(AlifObject*); // 29

ALIFAPI_FUNC(AlifObject*) alifList_getItem(AlifObject*, AlifSizeT); // 31

ALIFAPI_FUNC(AlifObject*) alifList_getItemRef(AlifObject*, AlifSizeT); // 33

ALIFAPI_FUNC(AlifIntT) alifList_setItem(AlifObject*, AlifSizeT, AlifObject*); // 35
ALIFAPI_FUNC(AlifIntT) alifList_insert(AlifObject*, AlifSizeT, AlifObject*); // 36
ALIFAPI_FUNC(AlifIntT) alifList_append(AlifObject*, AlifObject*); // 37

ALIFAPI_FUNC(AlifIntT) alifList_setSlice(AlifObject* , AlifSizeT , AlifSizeT , AlifObject*); // 40
ALIFAPI_FUNC(AlifIntT) alifList_sort(AlifObject*); // 42
ALIFAPI_FUNC(AlifIntT) alifList_reverse(AlifObject*); // 43
ALIFAPI_FUNC(AlifObject*) alifList_asTuple(AlifObject*); // 44

/* -------------------------------------------------------------------------------------- */



class AlifListObject { //  5
public:
	ALIFOBJECT_VAR_HEAD{};
	AlifObject** item{};

	AlifSizeT allocated{};
};


#define ALIFLIST_CAST(_op) \
    (ALIF_CAST(AlifListObject*, (_op))) // 25


static inline AlifSizeT _alifList_getSize(AlifObject* _op) { // 30
	AlifListObject* list = ALIFLIST_CAST(_op);
	return alifAtomic_loadSizeRelaxed(&(ALIFVAROBJECT_CAST(list)->objSize));
}
#define ALIFLIST_GET_SIZE(_op) _alifList_getSize(ALIFOBJECT_CAST(_op)) 

#define ALIFLIST_GET_ITEM(_op, _index) (ALIFLIST_CAST(_op)->item[(_index)])



static inline void _alifList_setItem(AlifObject* _op,
	AlifSizeT _index, AlifObject* _value) { // 42
	AlifListObject* list = ALIFLIST_CAST(_op);
	list->item[_index] = _value;
}
#define ALIFLIST_SET_ITEM(_op, _index, _value) \
    _alifList_setItem(ALIFOBJECT_CAST(_op), (_index), ALIFOBJECT_CAST(_value))

