#pragma once

class AlifListObject {
public:

	ALIFOBJECT_VAR_HEAD

	AlifObject** items{};

	AlifSizeT allocate;
};

extern AlifInitObject typeList;

AlifObject* alifNew_list(size_t size_);
size_t alifList_size(AlifObject* list);

#define ALIFLIST_GET_SIZE(_lst) (((AlifVarObject*)_lst)->size_)
#define ALIFLIST_GET_ITEM(_op, _i) (((AlifListObject*)_op)->items[_i])

AlifObject* alifList_getItem(AlifObject*, size_t);
bool alifList_setItem(AlifObject*, size_t, AlifObject*);
bool List_insert(AlifObject*, size_t, AlifObject*);
bool alifList_append(AlifObject*, AlifObject*);

AlifObject* list_getSlice(AlifObject*, size_t , size_t);
int list_setSlice(AlifObject*, int64_t, int64_t, AlifObject*);

int alifList_sort(AlifObject* );

