#pragma once

class AlifListObject {
public:

	ALIFObject_VAR_HEAD

	AlifObject** items{};

	size_t allocate;
};

extern AlifInitObject typeList;

AlifObject* alifNew_list(size_t size_);
size_t alifList_size(AlifObject* list);

AlifObject* alifList_getItem(AlifObject*, size_t);
bool alifList_setItem(AlifObject*, size_t, AlifObject*);
bool List_insert(AlifObject*, size_t, AlifObject*);
bool alifList_append(AlifObject*, AlifObject*);

AlifObject* list_getSlice(AlifObject*, size_t , size_t);
int list_setSlice(AlifObject*, int64_t, int64_t, AlifObject*);

