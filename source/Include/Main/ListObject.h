


#pragma once

extern AlifTypeObject _alifListType_;

#define ALIFLIST_CHECK(_op) \
    ALIFTYPE_FASTSUBCLASS(ALIF_TYPE(_op), ALIFTPFLAGS_LIST_SUBCLASS)
#define ALIFLIST_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifListType_)

AlifObject* alifNew_list(size_t size_);
size_t alifList_size(AlifObject* list);

#define ALIFLIST_GET_SIZE(_lst) (((AlifVarObject*)_lst)->size_)
#define ALIFLIST_GET_ITEM(_op, _i) (((AlifListObject*)_op)->items_[_i])

AlifObject* alifList_getItem(AlifObject*, size_t);
int alifList_setItem(AlifObject*, size_t, AlifObject*);
int alifList_insert(AlifObject*, size_t, AlifObject*);
int alifList_append(AlifObject*, AlifObject*);

AlifObject* list_getSlice(AlifObject*, size_t, size_t);
int list_setSlice(AlifObject*, int64_t, int64_t, AlifObject*);

int alifList_sort(AlifObject*);
AlifObject* alifList_asTuple(AlifObject*);

#define ALIFSUBLIST_CAST(op) \
    (ALIFLIST_CHECK(op)), ALIF_CAST(AlifListObject*, (op))


class AlifListObject {
public:

	ALIFOBJECT_VAR_HEAD

		AlifObject** items_{};

	AlifSizeT allocate_;
};


static inline void alifList_set_item(AlifObject* _op, int64_t _index, AlifObject* _value) {
	AlifListObject* list_ = (AlifListObject*)_op;
	list_->items_[_index] = _value;
}
#define ALIFLIST_SET_ITEM(_op, _index, _value) \
    alifList_set_item(ALIFSUBOBJECT_CAST(_op), (_index), ALIFSUBOBJECT_CAST(_value))


AlifObject* alifList_extend(AlifListObject*, AlifObject*);




