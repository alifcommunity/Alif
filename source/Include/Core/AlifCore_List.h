#pragma once

AlifObject* alifList_extend(AlifListObject* , AlifObject* );

extern int _alifList_appendTakeRefListResize(AlifListObject* self, AlifObject* newitem);

static inline int _alifList_appendTakeRef(AlifListObject* self, AlifObject* newitem)
{
    int64_t len = ((AlifVarObject*)self)->size_;
    int64_t allocated = self->allocate;
    if (allocated > len) {
        ((AlifVarObject*)self)->size_ = len + 1;
        alifList_setItem((AlifObject*)self, len, newitem);
        return 0;
    }
    return _alifList_appendTakeRefListResize(self, newitem);
}