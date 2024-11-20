#pragma once

extern AlifTypeObject _alifListType_; // 20


// 24
#define ALIFLIST_CHECK(_op) \
    ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIF_TPFLAGS_LIST_SUBCLASS)
#define ALIFLIST_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifListType_)



AlifObject* alifList_new(AlifSizeT); // 28
AlifSizeT alifList_size(AlifObject*); // 29
AlifIntT alifList_append(AlifObject*, AlifObject*); // 37

AlifIntT alifList_setSlice(AlifObject* , AlifSizeT , AlifSizeT , AlifObject*); // 40
AlifIntT alifList_sort(AlifObject*); // 42
AlifObject* alifList_asTuple(AlifObject*); // 44

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
#ifdef ALIF_GIL_DISABLED
	return alifAtomic_loadSizeRelaxed(&(ALIFVAROBJECT_CAST(list)->objSize));
#else
	return ALIF_SIZE(list);
#endif
}
#define ALIFLIST_GET_SIZE(_op) _alifList_getSize(ALIFOBJECT_CAST(_op)) 

#define ALIFLIST_GET_ITEM(_op, _index) (ALIFLIST_CAST(_op)->item[(_index)])



static inline void alifList_SetItem(AlifObject* _op,
	AlifSizeT _index, AlifObject* _value) { // 42
	AlifListObject* list = ALIFLIST_CAST(_op);
	list->item[_index] = _value;
}
#define ALIFLIST_SET_ITEM(_op, _index, _value) \
    alifList_SetItem(ALIFOBJECT_CAST(_op), (_index), ALIFOBJECT_CAST(_value))

