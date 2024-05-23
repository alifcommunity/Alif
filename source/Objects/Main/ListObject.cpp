#include "alif.h"

#include "alifCore_List.h"
#include "AlifCore_Object.h"
#include "AlifCore_Memory.h"

bool list_resize(AlifListObject* list, size_t newSize) {

    AlifObject** items{};
    size_t newAllocated{}, newAllocatedBytes{};
    uint64_t allocated = list->allocate;

    if (allocated >= newSize && newSize >= (allocated >> 1)) {
        ((AlifVarObject*)list)->size_ = newSize;
        return true;
    }

    //The growth pattern is : 0, 4, 8, 16, 24, 32, 40, 52, 64, ...
    newAllocated = ((size_t)newSize + (newSize >> 3) + 6) & ~(size_t)3;

    if(newSize == 0){
        newAllocated = 0;
    }

    if (list->items == nullptr && newAllocated <= (size_t)LLONG_MAX / sizeof(AlifObject*)) {
        newAllocatedBytes = newAllocated * sizeof(AlifObject*);
        list->items = (AlifObject**)alifMem_dataAlloc(newAllocatedBytes);
        items = list->items;
    }
    else if (newAllocated <= (size_t)LLONG_MAX / sizeof(AlifObject*)) {
        newAllocatedBytes = newAllocated * sizeof(AlifObject*);
        items = (AlifObject**)alifMem_objRealloc(list->items, newAllocatedBytes);
    }
    else
    {
        return false;
    }
    if (items == nullptr) {
        return false;
    }
    list->items = items;
    ((AlifVarObject*)list)->size_ = newSize;
    list->allocate = newAllocated;
    return true;
}

static int list_preallocate_exact(AlifListObject* self, int64_t size_)
{
    size_ = (size_ + 1) & ~(size_t)1;
    AlifObject** items = (AlifObject**)alifMem_dataAlloc(size_);
    if (items == nullptr) {
        //Err_NoMemory();
        return -1;
    }
    self->items = items;
    self->allocate = size_;
    return 0;
}

AlifObject* alifNew_list(size_t size_) {

	AlifListObject* object = (AlifListObject*)alifMem_objAlloc(sizeof(AlifListObject));

    object->_base_._base_.type_ = &typeList;
    if (size_ <= 0) {
        object->items = nullptr;
    }
    else {
        object->items = (AlifObject**)alifMem_objAlloc(size_ * sizeof(AlifObject*));
    }
    object->_base_.size_ = size_;
    object->allocate = size_;
    return (AlifObject*)object;
}

static AlifObject* list_new_prealloc(int64_t size_)
{
    AlifListObject* op = (AlifListObject*)alifNew_list(0);
    if (op == nullptr) {
        return nullptr;
    }
    op->items = (AlifObject**)alifMem_dataAlloc(size_);
    if (op->items == nullptr) {
        //return PyErr_NoMemory();
    }
    op->allocate = size_;
    return (AlifObject*)op;
}

size_t alifList_size(AlifObject* list) {

    return ((AlifVarObject*)list)->size_;

}

bool valid_index(size_t index, size_t limit)
{
    return (size_t)index < (size_t)limit;
}

AlifObject* alifList_getItem(AlifObject* list, size_t index){

    if (!valid_index(index, ((AlifVarObject*)list)->size_)) {
        std::wcout << L"مؤشر المصفوفة خارج النطاق\n" << std::endl;
        exit(-1);    
    }
    return ((AlifListObject*)list)->items[index];
}

#define ALIFLIST_GETITEM(list, index) (((AlifListObject*)list)->items[(index)])

static int ins1(AlifListObject* self, int64_t where, AlifObject* v)
{
    int64_t i, n = ((AlifVarObject*)self)->size_;
    AlifObject** items;
    if (v == nullptr) {
        return -1;
    }

    if (list_resize(self, n + 1) < 0)
        return -1;

    if (where < 0) {
        where += n;
        if (where < 0)
            where = 0;
    }
    if (where > n)
        where = n;
    items = self->items;
    for (i = n; --i >= where; )
        items[i + 1] = items[i];
    items[where] = (v);
    return 0;
}


int _alifList_appendTakeRefListResize(AlifListObject* self, AlifObject* newitem)
{
    int64_t len = ((AlifVarObject*)self)->size_;
    if (list_resize(self, len + 1) < 0) {
        return -1;
    }
    alifList_setItem((AlifObject*)self, len, newitem);
    return 0;
}

bool alifList_setItem(AlifObject* list, size_t index, AlifObject* newItem) {

    if (!valid_index(index, ((AlifVarObject*)list)->size_)) {
        std::wcout << L"مؤشر المصفوفة في عمليه اسناد خارج النطاق\n" << std::endl;
        exit(-1);   
    }

    AlifObject** item = ((AlifListObject*)list)->items + index;
    AlifObject** tmpDstPtr = (AlifObject**)list;
    AlifObject* tmpOldDst = (*tmpDstPtr);
    AlifObject* tmpSrc = newItem;
    memcpy(tmpDstPtr, &tmpSrc, sizeof(AlifObject*));
    return 1;
}

bool selected_insert(AlifListObject* list, size_t where, AlifObject* item) {

    size_t i, n = ((AlifVarObject*)list)->size_;
    AlifObject** items{};

    if (!list_resize(list, n + 1)) {
        return false;
    }

    if (where < 0) {
        where += n;
        if (where < 0)
            where = 0;
    }
    if (where > n) {
        where = n;
    }

    items = list->items;
    for (i = n; --i >= where; ){
        items[i + 1] = items[i];
    }
    items[where] = item;
    return true;
}

bool List_insert(AlifObject* list, size_t where, AlifObject* newItem)
{
    if (list->type_ != &typeList) {
        std::wcout << L"يجب ان يكون نوع الكائن مصفوفة ليتم عملية الادراج\n" << std::endl;
        exit(-1);
    }
    return selected_insert((AlifListObject*)list, where, newItem);
}

bool alifList_append(AlifObject* list, AlifObject* newItem)
{
    size_t len = ((AlifVarObject*)list)->size_;
    AlifListObject* listObj = (AlifListObject*)list;
    
    if (newItem == nullptr) {
        std::wcout << L"لا يوجد كائن ليتم اسناده\n" << std::endl;
        exit(-1);
    }

    if (!list_resize(listObj, len + 1)) {
        return false;
    }
    listObj->items[len] = newItem;
    return true;
}

static AlifObject* list_extend(AlifListObject* self, AlifObject* iterable)
{
    AlifObject* it;    
    int64_t m;                 
    int64_t n{};
    int64_t i;
    AlifObject* (*IterNext)(AlifObject*);

    if ((iterable->type_ == &typeList) || (iterable->type_ == &typeTuple) ||
        (AlifObject*)self == iterable) {
        AlifObject** src, ** dest;
        iterable = alifSequence_fast(iterable, L"argument must be iterable");
        if (!iterable)
            return nullptr;
        n = ALIFSEQUENCE_FAST_GETSIZE(iterable);
        if (n == 0) {
            return ALIF_NONE;
        }
        m = ((AlifVarObject*)self)->size_;
        if (self->items == nullptr) {
            if (list_preallocate_exact(self, n) < 0) {
                return nullptr;
            }
            ((AlifVarObject*)self)->size_ = n;
        }
        else if (list_resize(self, m + n) < 0) {
            return nullptr;
        }
        src = ALIFSEQUENCE_FAST_ITEMS(iterable);
        dest = self->items + m;
        for (i = 0; i < n; i++) {
            AlifObject* o = src[i];
            dest[i] = o;
        }
        return ALIF_NONE;
    }

    it = alifObject_getIter(iterable);
    if (it == nullptr)
        return nullptr;
    IterNext = *(it)->type_->iterNext;

    //n = AlifObj_LengthHint(iterable, 8);
    if (n < 0) {
        return nullptr;
    }
    m = ((AlifVarObject*)self)->size_;
    if (m > LLONG_MAX - n) {

    }
    else if (self->items == nullptr) {
        if (n && list_preallocate_exact(self, n) < 0)
            goto error;
    }
    else {
        
        if (list_resize(self, m + n) < 0)
            goto error;
        (self, m);
    }

    for (;;) {
        AlifObject* item = IterNext(it);
        if (item == nullptr) {
            //if (Err_Occurred()) {
                //if (Err_ExceptionMatches(Exc_StopIteration))
                    //Err_Clear();
                //else
                    //goto error;
            //}
            break;
        }
        if (((AlifVarObject*)self)->size_ < self->allocate) {
            int64_t len = ((AlifVarObject*)self)->size_;
            ((AlifVarObject*)self)->size_ = len + 1;
            alifList_setItem((AlifObject*)self, len, (AlifObject*)item);
        }
        else {
            //if (_PyList_AppendTakeRef(self, item) < 0)
                //goto error;
        }
    }

    //if (((AlifVarObject*)self)->size_ < self->allocate) {
        //if (list_resize(self, ((AlifVarObject*)self)->size_) < 0)
            //goto error;
    //}

    
    return ALIF_NONE;

error:
    return nullptr;
}

AlifObject* alifList_extend(AlifListObject* self, AlifObject* iterable)
{
    return list_extend(self, iterable);
}

size_t list_length(AlifListObject* list) {
    return ((AlifVarObject*)list)->size_;
}

AlifObject* list_item(AlifListObject* list, size_t index) {

    if (!valid_index(index, ((AlifVarObject*)list)->size_)) {
        return nullptr;
    }
    return list->items[index];
}

AlifObject* list_slice(AlifListObject* list, size_t indexLow, size_t indexHigh)
{
    AlifListObject* np;
    AlifObject** source, ** destination;
    size_t i, len;
    len = indexHigh - indexLow;
    if (len <= 0) {
        return (AlifObject*)alifNew_list(0);
    }
    np = (AlifListObject*)alifNew_list(len);
    if (np == nullptr)
        return nullptr;

    source = list->items + indexLow;
    destination = np->items;
    for (i = 0; i < len; i++) {
        destination[i] = source[i];
    }
    ((AlifVarObject*)np)->size_ = len;
    return (AlifObject*)np;
}

AlifObject* list_getSlice(AlifObject* list, size_t indexLow, size_t indexHigh)
{
    if (indexLow < 0) {
        indexLow = 0;
    }
    else if (indexLow > ((AlifVarObject*)list)->size_) {
        indexLow = ((AlifVarObject*)list)->size_;
    }
    if (indexHigh < indexLow) {
        indexHigh = indexLow;
    }
    else if (indexHigh > ((AlifVarObject*)list)->size_) {
        indexHigh = ((AlifVarObject*)list)->size_;
    }
    return list_slice((AlifListObject*)list, indexLow, indexHigh);
}

AlifObject* list_combine(AlifListObject* firstList, AlifListObject* secondList)
{
    size_t size_;
    size_t i;
    AlifObject** source, ** destination;
    AlifListObject* np;
    size_ = ((AlifVarObject*)firstList)->size_ + ((AlifVarObject*)secondList)->size_;
    if (size_ == 0) {
        return (AlifObject*)alifNew_list(0);
    }
    np = (AlifListObject*)alifNew_list(size_);
    if (np == nullptr) {
        return nullptr;
    }
    source = firstList->items;
    destination = np->items;
    for (i = 0; i < ((AlifVarObject*)firstList)->size_; i++) {
        destination[i] = source[i];
    }
    source = secondList->items;
    destination = np->items + ((AlifVarObject*)firstList)->size_;
    for (i = 0; i < ((AlifVarObject*)secondList)->size_; i++) {
        destination[i] = source[i];
    }
    ((AlifVarObject*)np)->size_ = size_;
    return (AlifObject*)np;
}

inline void memory_repeat(char* destination, size_t lenDest, size_t lenSrc)
{
    size_t copied = lenSrc;
    while (copied < lenDest) {
        size_t bytesToCopy = min(copied, lenDest - copied);
        memcpy(destination + copied, destination, bytesToCopy);
        copied += bytesToCopy;
    }
}

AlifObject* list_repeat(AlifListObject* list, size_t repeat)
{
    size_t inputSize = ((AlifVarObject*)list)->size_;
    if (inputSize == 0 || repeat <= 0)
        return (AlifObject*)alifNew_list(0);

    size_t outputSize = inputSize * repeat;

    AlifListObject* np = (AlifListObject*)alifNew_list(outputSize);
    if (np == nullptr)
        return nullptr;

    AlifObject** destination = np->items;
    if (inputSize == 1) {
        AlifObject* element = list->items[0];
        AlifObject** destinationEnd = destination + outputSize;
        while (destination < destinationEnd) {
            *destination++ = element;
        }
    }
    else {
        AlifObject** source = list ->items;
        AlifObject** sourceEnd = source + inputSize;
        while (source < sourceEnd) {
            *destination++ = *source++;
        }

        memory_repeat((char*)np->items, sizeof(AlifObject*) * outputSize,
            sizeof(AlifObject*) * inputSize);
    }

    ((AlifVarObject*)np)->size_ = outputSize;
    return (AlifObject*)np;
}

static int _list_clear(AlifListObject* a)
{
    int64_t i;
    AlifObject** item = a->items;
    if (item != nullptr) {

        i = ((AlifVarObject*)a)->size_;
        ((AlifVarObject*)a)->size_ = 0;
        a->items = nullptr;
        a->allocate = 0;

        alifMem_dataFree(item);
    }

    return 0;
}

static AlifObject* list_clear_impl(AlifListObject* self)
{
    _list_clear(self);
    return ALIF_NONE;
}

static AlifObject* list_clear(AlifListObject* self)
{
    return list_clear_impl(self);
}

static AlifObject* list_append(AlifListObject* self, AlifObject* object)
{
    if (_alifList_appendTakeRef(self, (object)) < 0) {
        return nullptr;
    }
    return ALIF_NONE;
}

static AlifObject* list_insert_impl(AlifListObject* self, int64_t index, AlifObject* object)
{
    if (ins1(self, index, object) == 0)
        return ALIF_NONE;
    return nullptr;
}

static AlifObject* list_pop_impl(AlifListObject* self, int64_t index)
{
    AlifObject* v;
    int status;

    if (((AlifVarObject*)self)->size_ == 0) {
        return nullptr;
    }
    if (index < 0)
        index += ((AlifVarObject*)self)->size_;
    if (!valid_index(index, ((AlifVarObject*)self)->size_)) {
        return nullptr;
    }

    AlifObject** items = self->items;
    v = items[index];
    const int64_t size_after_pop = ((AlifVarObject*)self)->size_ - 1;
    if (size_after_pop == 0) {
        status = _list_clear(self);
    }
    else {
        if ((size_after_pop - index) > 0) {
            memmove(&items[index], &items[index + 1], (size_after_pop - index) * sizeof(AlifObject*));
        }
        status = list_resize(self, size_after_pop);
    }
    if (status >= 0) {
        return v; 
    }
    else {
        memmove(&items[index + 1], &items[index], (size_after_pop - index) * sizeof(AlifObject*));
        items[index] = v;
        return nullptr;
    }
}

static AlifObject* list_pop(AlifListObject* self, AlifObject* const* args, int64_t nargs)
{
    AlifObject* return_value = nullptr;
    int64_t index = -1;

    if (!_alifArg_checkPositional(L"pop", nargs, 0, 1)) {
        goto exit;
    }
    if (nargs < 1) {
        goto skip_optional;
    }
    {
        int64_t ival = -1;
        AlifObject* iobj = args[0];
        if (iobj != nullptr) {
            ival = alifInteger_asLongLong(iobj);
        }

        index = ival;
    }
skip_optional:
    return_value = list_pop_impl(self, index);

exit:
    return return_value;
}

static AlifObject* list_insert(AlifListObject* self, AlifObject* const* args, int64_t nargs)
{
    AlifObject* return_value = nullptr;
    int64_t index;
    AlifObject* object;

    if (!_alifArg_checkPositional(L"insert", nargs, 2, 2)) {
        goto exit;
    }
    {
        int64_t ival = -1;
        AlifObject* iobj = args[0];
        if (iobj != nullptr) {
            ival = alifInteger_asLongLong(iobj);
        }
        index = ival;
    }
    object = args[1];
    return_value = list_insert_impl(self, index, object);

exit:
    return return_value;
}

static int
list_ass_slice(AlifListObject* a, int64_t ilow, int64_t ihigh, AlifObject* v)
{
    AlifObject* recycle_on_stack[8];
    AlifObject** recycle = recycle_on_stack; 
    AlifObject** item;
    AlifObject** vitem = nullptr;
    AlifObject* v_as_SF = nullptr; 
    int64_t n; 
    int64_t norig; 
    int64_t d; 
    int64_t k;
    size_t s;
    int result = -1;            
#define b ((AlifListObject *)v)
    if (v == nullptr)
        n = 0;
    else {
        if (a == b) {
            //v = list_slice(b, 0, ((AlifVarObject*)b)->size_);
            if (v == nullptr)
                return result;
            result = list_ass_slice(a, ilow, ihigh, v);
            return result;
        }
        v_as_SF = alifSequence_fast(v, L"can only assign an iterable");
        if (v_as_SF == nullptr)
            goto Error;
        n = ALIFSEQUENCE_FAST_GETSIZE(v_as_SF);
        vitem = ALIFSEQUENCE_FAST_ITEMS(v_as_SF);
    }
    if (ilow < 0)
        ilow = 0;
    else if (ilow > ((AlifVarObject*)a)->size_)
        ilow = ((AlifVarObject*)a)->size_;

    if (ihigh < ilow)
        ihigh = ilow;
    else if (ihigh > ((AlifVarObject*)a)->size_)
        ihigh = ((AlifVarObject*)a)->size_;

    norig = ihigh - ilow;
    d = n - norig;
    if (((AlifVarObject*)a)->size_ + d == 0) {
        return _list_clear(a);
    }
    item = a->items;
    s = norig * sizeof(AlifObject*);
    if (s) {
        if (s > sizeof(recycle_on_stack)) {
            recycle = (AlifObject**)alifMem_dataAlloc(s);
            if (recycle == nullptr) {
                goto Error;
            }
        }
        memcpy(recycle, &item[ilow], s);
    }

    if (d < 0) { 
        int64_t tail;
        tail = (((AlifVarObject*)a)->size_ - ihigh) * sizeof(AlifObject*);
        memmove(&item[ihigh + d], &item[ihigh], tail);
        //if (list_resize(a, ((AlifVarObject*)a)->size_ + d) < 0) {
            //memmove(&item[ihigh], &item[ihigh + d], tail);
            //memcpy(&item[ilow], recycle, s);
            //goto Error;
        //}
        item = a->items;
    }
    else if (d > 0) {
        k = ((AlifVarObject*)a)->size_;
        if (list_resize(a, k + d) < 0)
            goto Error;
        item = a->items;
        memmove(&item[ihigh + d], &item[ihigh],
            (k - ihigh) * sizeof(AlifObject*));
    }
    for (k = 0; k < n; k++, ilow++) {
        AlifObject* w = vitem[k];
        item[ilow] = w;
    }

    result = 0;
Error:
    if (recycle != recycle_on_stack)
        alifMem_dataFree(recycle);
    return result;
#undef b
}

static int list_ass_item(AlifListObject* a, int64_t i, AlifObject* v)
{
    if (!valid_index(i, ((AlifVarObject*)a)->size_)) {
        std::wcout << L"مؤشر المصفوفة في عمليه احضار كائن خارج النطاق\n" << std::endl;
        exit(-1);
    }
    if (v == nullptr)
        return list_ass_slice(a, i, i + 1, v);
    ALIF_SETREF(a->items[i], v);
    return 0;
}

bool list_contain(AlifListObject *list, AlifObject* element) {

    AlifObject* item{};
    uint64_t i{};
    int compare{};

    for (i = 0 ,compare = 0; compare == 0 && i < list->_base_.size_ ; i++)
    {
        item = ALIFLIST_GETITEM(list, i);
        compare = alifObject_richCompareBool(item, element, ALIF_EQ);
    }
    return compare;
}

void list_dealloc(AlifObject* object) {

    alifMem_dataFree(((AlifListObject*)object)->items);

    alifMem_objFree(object);

}

static AlifObject* list_count(AlifListObject* self, AlifObject* value)
{
    int64_t count = 0;
    int64_t i;

    for (i = 0; i < ((AlifVarObject*)self)->size_; i++) {
        AlifObject* obj = self->items[i];
        if (obj == value) {
            count++;
            continue;
        }
        int cmp = alifObject_richCompareBool(obj, value, ALIF_EQ);
        if (cmp > 0)
            count++;
        else if (cmp < 0)
            return nullptr;
    }
    return alifInteger_fromLongLong(count);
}

static AlifObject* list_remove(AlifListObject* self, AlifObject* value)
{
    int64_t i;

    for (i = 0; i < ((AlifVarObject*)self)->size_; i++) {
        AlifObject* obj = self->items[i];
        int cmp = alifObject_richCompareBool(obj, value, ALIF_EQ);
        if (cmp > 0) {
            if (list_ass_slice(self, i, i + 1,
                (AlifObject*)nullptr) == 0)
                return ALIF_NONE;
            return nullptr;
        }
        else if (cmp < 0)
            return nullptr;
    }
    return nullptr;
}

static AlifObject* list_compare(AlifObject* v, AlifObject* w, int op)
{
    AlifListObject* vl, * wl;
    int64_t i;

    //if (!(v->type_ == &typeList) || !(w->type_ == &typeList))
        //return NOTIMPLEMENTED;

    vl = (AlifListObject*)v;
    wl = (AlifListObject*)w;

    if (((AlifVarObject*)vl)->size_ != ((AlifVarObject*)wl)->size_ && (op == ALIF_EQ || op == ALIF_NE)) {
        if (op == ALIF_EQ)
            return ALIF_FALSE;
        else
            return ALIF_TRUE;
    }

    for (i = 0; i < ((AlifVarObject*)vl)->size_ && i < ((AlifVarObject*)wl)->size_; i++) {
        AlifObject* vitem = vl->items[i];
        AlifObject* witem = wl->items[i];
        if (vitem == witem) {
            continue;
        }

        int k = alifObject_richCompareBool(vitem, witem, ALIF_EQ);
        if (k < 0)
            return nullptr;
        if (!k)
            break;
    }

    if (i >= ((AlifVarObject*)vl)->size_ || i >= ((AlifVarObject*)wl)->size_) {
        ALIF_RETURN_RICHCOMPARE(((AlifVarObject*)vl)->size_, ((AlifVarObject*)wl)->size_, op);
    }

    if (op == ALIF_EQ) {
        return ALIF_FALSE;
    }
    if (op == ALIF_NE) {
        return ALIF_TRUE;
    }

    return alifObject_richCompare(vl->items[i], wl->items[i], op);
}

static AlifObject* list___sizeof___impl(AlifListObject* self)
{
    size_t res = self->_base_._base_.type_->basicSize;
    res += (size_t)self->allocate * sizeof(void*);
    return alifInteger_fromSizeT(res , true);
}

static AlifObject* list___sizeof__(AlifListObject* self)
{
    return list___sizeof___impl(self);
}

static AlifObject* list_subscript(AlifListObject*, AlifObject*);

static AlifMethodDef listMethods[] = {
    {L"__getItem__", (AlifCFunction)list_subscript, METHOD_O | METHOD_COEXIST,},
    {L"__sizeof__", (AlifCFunction)list___sizeof__, METHOD_NOARGS},
    {L"clear", (AlifCFunction)list_clear, METHOD_NOARGS},
    {L"append", (AlifCFunction)list_append, METHOD_O},
    {L"insert", ALIFCFunction_CAST(list_insert), METHOD_FASTCALL,},
    {L"extend", (AlifCFunction)list_extend, METHOD_O},
    {L"pop", ALIFCFunction_CAST(list_pop), METHOD_FASTCALL},
    {L"remove", (AlifCFunction)list_remove, METHOD_O},
    {L"count", (AlifCFunction)list_count, METHOD_O},
    {nullptr,              nullptr}
};

AlifSequenceMethods listAsSeq = {
    (LenFunc)list_length,                      
    (BinaryFunc)list_combine,                    
    (SSizeArgFunc)list_repeat,               
    (SSizeArgFunc)list_item,                   
    0,                                        
    (SSizeObjArgProc)list_ass_item,
    0,                                          
    (ObjObjProc)list_contain,                  
    0,           
    0,          
};

static AlifObject* list_subscript(AlifListObject* self, AlifObject* item)
{
    if (item->type_->asNumber != nullptr) {
        int64_t i;
        i = alifInteger_asSizeT(item);
        if (i < 0)
            i += ((AlifVarObject*)self)->size_;
        return list_item(self, i);
    }
    else if (item->type_ == &_typeSlice_) {
        int64_t start_, stop_, step_, slicelength, i;
        size_t cur;
        AlifObject* result;
        AlifObject* it;
        AlifObject** src, ** dest;

        if (slice_unpack((AlifSliceObject*)item, &start_, &stop_, &step_) < 0) {
            return nullptr;
        }
        slicelength = slice_adjustIndices(((AlifVarObject*)self)->size_, &start_, &stop_,
            step_);

        if (slicelength <= 0) {
            return alifNew_list(0);
        }
        else if (step_ == 1) {
            return list_slice(self, start_, stop_);
        }
        else {
            result = list_new_prealloc(slicelength);
            if (!result) return nullptr;

            src = self->items;
            dest = ((AlifListObject*)result)->items;
            for (cur = start_, i = 0; i < slicelength;
                cur += (size_t)step_, i++) {
                it = src[cur];
                dest[i] = it;
            }
            ((AlifVarObject*)result)->size_ = slicelength;
            return result;
        }
    }
    else {
        return nullptr;
    }
}

static int list_ass_subscript(AlifListObject* self, AlifObject* item, AlifObject* value)
{
    if ((item)->type_->asNumber != nullptr) {
        int64_t i = alifInteger_asSizeT(item);

        if (i < 0)
            i += ((AlifVarObject*)self)->size_;
        return list_ass_item(self, i, value);
    }
    else if ((item)->type_ == &_typeSlice_) {
        int64_t start_, stop_, step_, slicelength;

        if (slice_unpack((AlifSliceObject*)item, &start_, &stop_, &step_) < 0) {
            return -1;
        }
        slicelength = slice_adjustIndices(((AlifVarObject*)self)->size_, &start_, &stop_,
            step_);

        if (step_ == 1)
            return list_ass_slice(self, start_, stop_, value);

        /* Make sure s[5:2] = [..] inserts at the right place:
           before 5, not before 2. */
        if ((step_ < 0 && start_ < stop_) ||
            (step_ > 0 && start_ > stop_))
            stop_ = start_;

        if (value == nullptr) {
            /* delete slice */
            AlifObject** garbage;
            size_t cur;
            int64_t i;
            int res;

            if (slicelength <= 0)
                return 0;

            if (step_ < 0) {
                stop_ = start_ + 1;
                start_ = stop_ + step_ * (slicelength - 1) - 1;
                step_ = -step_;
            }

            garbage = (AlifObject**)
                alifMem_dataAlloc(slicelength * sizeof(AlifObject*));
            if (!garbage) {
                
                return -1;
            }
            for (cur = start_, i = 0;
                cur < (size_t)stop_;
                cur += step_, i++) {
                int64_t lim = step_ - 1;

                garbage[i] = ((AlifListObject*)self)->items[cur];

                if (cur + step_ >= (size_t)((AlifVarObject*)self)->size_) {
                    lim = ((AlifVarObject*)self)->size_ - cur - 1;
                }

                memmove(self->items + cur - i,
                    self->items + cur + 1,
                    lim * sizeof(AlifObject*));
            }
            cur = start_ + (size_t)slicelength * step_;
            //if (cur < (size_t)((AlifVarObject*)self)->size_) {
                //memmove(self->items + cur - slicelength,
                    //self->items + cur,
                    //(((AlifVarObject*)self)->size_ - cur) *
                    //sizeof(AlifObject*));
            //}

            ((AlifVarObject*)self)->size_ = ((AlifVarObject*)self)->size_ - slicelength;
            res = list_resize(self, ((AlifVarObject*)self)->size_);

            alifMem_dataFree(garbage);

            return res;
        }
        else {
            /* assign slice */
            AlifObject* ins, * seq;
            AlifObject** garbage, ** seqitems, ** selfitems;
            int64_t i;
            size_t cur;

            /* protect against a[::-1] = a */
            if (self == (AlifListObject*)value) {
                seq = list_slice((AlifListObject*)value, 0,
                    ((AlifVarObject*)value)->size_);
            }
            else {
                seq = alifSequence_fast(value,
                    L"must assign iterable "
                    "to extended slice");
            }
            if (!seq)
                return -1;

            if (ALIFSEQUENCE_FAST_GETSIZE(seq) != slicelength) {

                return -1;
            }

            if (!slicelength) {
                return 0;
            }

            garbage = (AlifObject**)
                alifMem_dataAlloc(slicelength * sizeof(AlifObject*));
            if (!garbage) {

                return -1;
            }

            selfitems = self->items;
            seqitems = ALIFSEQUENCE_FAST_ITEMS(seq);
            for (cur = start_, i = 0; i < slicelength;
                cur += (size_t)step_, i++) {
                garbage[i] = selfitems[cur];
                ins = seqitems[i];
                selfitems[cur] = ins;
            }

            alifMem_dataFree(garbage);

            return 0;
        }
    }
    else {
        return -1;
    }
}

AlifMappingMethods listAsMap = {
    (LenFunc)list_length,
    (BinaryFunc)list_subscript,
    (ObjObjArgProc)list_ass_subscript
};

AlifInitObject typeList = {
    0,
    0,
    0,
    L"list",
    sizeof(AlifListObject),
    0,
    list_dealloc,                  
    0,                                          
    0,                                          
    0,                                         
    0,                        
    0,                                          
    &listAsSeq,                          
    &listAsMap,                         
    0,//alifObject_hashNotImplemented,             
    0,                                         
    0,     
    0,                                  
    0,                    
    0,                                         
    0,                                         
    0,  
    0,                      
    (Inquiry)_list_clear,
    list_compare,
    0,                        
    0,                                    
    0,                            
    listMethods,
    0,                           
    0,                                      
    0,                                     
    0,                                   
    0,                                   
    0,                                        
    0,                                        
    0,                                         
    0,                   
    0,                        
    0,                          
    0,                          
    //.vectorcall = listVectorcall,
};
