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
        AlifObject** src, ** dest_;
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
        dest_ = self->items + m;
        for (i = 0; i < n; i++) {
            AlifObject* o = src[i];
            dest_[i] = o;
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
            //if (_PyList_AppendTakeRef(_self, item) < 0)
                //goto error;
        }
    }

    //if (((AlifVarObject*)_self)->size_ < _self->allocate) {
        //if (list_resize(_self, ((AlifVarObject*)_self)->size_) < 0)
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
    int64_t k_;
    size_t s;
    int result_ = -1;            
#define b ((AlifListObject *)v)
    if (v == nullptr)
        n = 0;
    else {
        if (a == b) {
            //v = list_slice(b, 0, ((AlifVarObject*)b)->size_);
            if (v == nullptr)
                return result_;
            result_ = list_ass_slice(a, ilow, ihigh, v);
            return result_;
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
        if (list_resize(a, ((AlifVarObject*)a)->size_ + d) < 0) {
            memmove(&item[ihigh], &item[ihigh + d], tail);
            memcpy(&item[ilow], recycle, s);
            goto Error;
        }
        item = a->items;
    }
    else if (d > 0) {
        k_ = ((AlifVarObject*)a)->size_;
        if (list_resize(a, k_ + d) < 0)
            goto Error;
        item = a->items;
        memmove(&item[ihigh + d], &item[ihigh],
            (k_ - ihigh) * sizeof(AlifObject*));
    }
    for (k_ = 0; k_ < n; k_++, ilow++) {
        AlifObject* w = vitem[k_];
        item[ilow] = w;
    }

    result_ = 0;
Error:
    if (recycle != recycle_on_stack)
        alifMem_dataFree(recycle);
    return result_;
#undef b
}

int list_setSlice(AlifObject* a, int64_t ilow, int64_t ihigh, AlifObject* v)
{
	return list_ass_slice((AlifListObject*)a, ilow, ihigh, v);
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

static void reverse_slice(AlifObject** lo, AlifObject** hi)
{

	--hi;
	while (lo < hi) {
		AlifObject* t_ = *lo;
		*lo = *hi;
		*hi = t_;
		++lo;
		--hi;
	}
}

class SortSlice {
public:
	AlifObject** keys_;
	AlifObject** values_;
} ;

static __inline void __fastcall sortSlice_copy(SortSlice* _s1, int64_t _i, SortSlice* _s2, int64_t _j)
{
	_s1->keys_[_i] = _s2->keys_[_j];
	if (_s1->values_ != nullptr)
		_s1->values_[_i] = _s2->values_[_j];
}

static __inline void __fastcall sortSlice_copy_incr(SortSlice* _dst, SortSlice* _src)
{
	*_dst->keys_++ = *_src->keys_++;
	if (_dst->values_ != nullptr)
		*_dst->values_++ = *_src->values_++;
}

static __inline void __fastcall sortSlice_copy_decr(SortSlice* _dst, SortSlice* _src)
{
	*_dst->keys_-- = *_src->keys_--;
	if (_dst->values_ != nullptr)
		*_dst->values_-- = *_src->values_--;
}

static __inline void __fastcall sortSlice_memcpy(SortSlice* _s1, int64_t _i, SortSlice* _s2, int64_t _j,
	int64_t _n)
{
	memcpy(&_s1->keys_[_i], &_s2->keys_[_j], sizeof(AlifObject*) * _n);
	if (_s1->values_ != nullptr)
		memcpy(&_s1->values_[_i], &_s2->values_[_j], sizeof(AlifObject*) * _n);
}

static __inline void __fastcall sortSlice_memmove(SortSlice* _s1, int64_t _i, SortSlice* _s2, int64_t _j,
	int64_t _n)
{
	memmove(&_s1->keys_[_i], &_s2->keys_[_j], sizeof(AlifObject*) * _n);
	if (_s1->values_ != nullptr)
		memmove(&_s1->values_[_i], &_s2->values_[_j], sizeof(AlifObject*) * _n);
}

static __inline void __fastcall sortSlice_advance(SortSlice* _slice, int64_t _n)
{
	_slice->keys_ += _n;
	if (_slice->values_ != nullptr)
		_slice->values_ += _n;
}

#define ISLT(_X, _Y) (*(_ms->KeyCompare))(_X, _Y, _ms)

#define IFLT(_X, _Y) if ((k_ = ISLT(_X, _Y)) < 0) goto fail;  \
           if (k_)

#define MAX_MERGE_PENDING (sizeof(size_t) * 8)

#define MERGESTATE_TEMP_SIZE 256

#define MAX_MINRUN 64

#define MIN_GALLOP 7

class SSlice {
public:
	SortSlice base_;
	int64_t len_;   /* length of run */
	int power_; /* node "level" for powersort merge strategy */
};

typedef class SMergeState MergeState;
struct SMergeState {
public:
	int64_t minGallop;

	int64_t listLen;     
	AlifObject** baseKeys;   

	SortSlice a_;        
	int64_t alloced_;

	int n_;
	class SSlice pending_[MAX_MERGE_PENDING];

	AlifObject* tempArray[MERGESTATE_TEMP_SIZE];

	int (*KeyCompare)(AlifObject*, AlifObject*, MergeState*);

	AlifObject* (*KeyRichcompare)(AlifObject*, AlifObject*, int);

	int (*TupleElemCompare)(AlifObject*, AlifObject*, MergeState*);
};

static int binarysort(MergeState* _ms, const SortSlice* _ss, int64_t _n, int64_t _ok)
{
	int64_t k_; 
	AlifObject** const a_ = _ss->keys_;
	AlifObject** const v_ = _ss->values_;
	const bool hasValues = v_ != nullptr;
	AlifObject* pivot_;
	int64_t m_;

	if (!_ok)
		++_ok;
	
#if 0 
	AlifObject* vPivot = nullptr;
	for (; _ok < _n; ++_ok) {
		pivot_ = a_[_ok];
		if (hasValues)
			vPivot = v_[_ok];
		for (m_ = _ok - 1; m_ >= 0; --m_) {
			k_ = ISLT(pivot_, a_[m_]);
			if (k_ < 0) {
				a_[m_ + 1] = pivot_;
				if (hasValues)
					v_[m_ + 1] = vPivot;
				goto fail;
			}
			else if (k_) {
				a_[m_ + 1] = a_[m_];
				if (hasValues)
					v_[m_ + 1] = v_[m_];
			}
			else
				break;
		}
		a_[m_ + 1] = pivot_;
		if (hasValues)
			v_[m_ + 1] = vPivot;
	}
#else 
	int64_t L_, R_;
	for (; _ok < _n; ++_ok) {
		L_ = 0;
		R_ = _ok;
		pivot_ = a_[_ok];

		do {

			m_ = (L_ + R_) >> 1;
#if 1
			IFLT(pivot_, a_[m_])
				R_ = m_;
	else
		L_ = m_ + 1;
#else

			k_ = ISLT(pivot_, a_[m_]);
			if (k_ < 0)
				goto fail;
			int64_t mP1 = m_ + 1;
			R_ = k_ ? m_ : R_;
			L_ = k_ ? L : mP1;
#endif
		} while (L_ < R_);
		for (m_ = _ok; m_ > L_; --m_)
			a_[m_] = a_[m_ - 1];
		a_[L_] = pivot_;
		if (hasValues) {
			pivot_ = v_[_ok];
			for (m_ = _ok; m_ > L_; --m_)
				v_[m_] = v_[m_ - 1];
			v_[L_] = pivot_;
		}
	}
#endif 
	return 0;

fail:
	return -1;
}

static void sortSlice_reverse(SortSlice* _s, int64_t _n)
{
	reverse_slice(_s->keys_, &_s->keys_[_n]);
	if (_s->values_ != nullptr)
		reverse_slice(_s->values_, &_s->values_[_n]);
}

static int64_t count_run(MergeState* _ms, SortSlice* _sLo, int64_t _nRemaining)
{
	int64_t k_; 
	int64_t n_;
	AlifObject** const lo_ = _sLo->keys_;

	int64_t neq_ = 0;
#define REVERSE_LAST_NEQ                        \
    if (neq_) {                                  \
        SortSlice slice_ = *_sLo;                 \
        ++neq_;                                  \
        sortSlice_advance(&slice_, n_ - neq_);     \
        sortSlice_reverse(&slice_, neq_);         \
        neq_ = 0;                                \
    }

#define IF_NEXT_LARGER  IFLT(lo_[n_-1], lo_[n_])
#define IF_NEXT_SMALLER IFLT(lo_[n_], lo_[n_-1])

	for (n_ = 1; n_ < _nRemaining; ++n_) {
		IF_NEXT_SMALLER
			break;
	}
	if (n_ == _nRemaining)
		return n_;
	
	if (n_ > 1) {
		IFLT(lo_[0], lo_[n_ - 1])
			return n_;
		sortSlice_reverse(_sLo, n_);
	}
	++n_; 
	for (; n_ < _nRemaining; ++n_) {
		IF_NEXT_SMALLER{

			REVERSE_LAST_NEQ
		}
	else {
		IF_NEXT_LARGER 
			break;
	else 
		++neq_;
	}
	}
	REVERSE_LAST_NEQ
		sortSlice_reverse(_sLo, n_);
	for (; n_ < _nRemaining; ++n_) {
		IF_NEXT_SMALLER
			break;
	}

	return n_;
fail:
	return -1;

#undef REVERSE_LAST_NEQ
#undef IF_NEXT_SMALLER
#undef IF_NEXT_LARGER
}

static int64_t gallop_left(MergeState* _ms, AlifObject* _key, AlifObject** _a, int64_t _n, int64_t _hInt)
{
	int64_t ofs_;
	int64_t lastOfs;
	int64_t k_;

	_a += _hInt;
	lastOfs = 0;
	ofs_ = 1;
	IFLT(*_a, _key) {

		const int64_t maxOfs = _n - _hInt;
		while (ofs_ < maxOfs) {
			IFLT(_a[ofs_], _key) {
				lastOfs = ofs_;
				ofs_ = (ofs_ << 1) + 1;
			}
		   else                
			   break;
		}
		if (ofs_ > maxOfs)
			ofs_ = maxOfs;
		lastOfs += _hInt;
		ofs_ += _hInt;
	}
	else {

		const int64_t maxOfs = _hInt + 1;
		while (ofs_ < maxOfs) {
			IFLT(*(_a - ofs_), _key)
				break;
			lastOfs = ofs_;
			ofs_ = (ofs_ << 1) + 1;
		}
		if (ofs_ > maxOfs)
			ofs_ = maxOfs;
		k_ = lastOfs;
		lastOfs = _hInt - ofs_;
		ofs_ = _hInt - k_;
	}
	_a -= _hInt;


	++lastOfs;
	while (lastOfs < ofs_) {
		int64_t m_ = lastOfs + ((ofs_ - lastOfs) >> 1);

		IFLT(_a[m_], _key)
			lastOfs = m_ + 1;              
	else
	ofs_ = m_;                  
	}
	return ofs_;

	fail:
	return -1;
}

static int64_t gallop_right(MergeState* _ms, AlifObject* _key, AlifObject** _a, int64_t _n, int64_t _hInt)
{
	int64_t ofs_;
	int64_t lastOfs;
	int64_t k_;

	_a += _hInt;
	lastOfs = 0;
	ofs_ = 1;
	IFLT(_key, *_a) {

		const int64_t maxOfs = _hInt + 1;
		while (ofs_ < maxOfs) {
			IFLT(_key, *(_a - ofs_)) {
				lastOfs = ofs_;
				ofs_ = (ofs_ << 1) + 1;
			}
			else               
				break;
		}
		if (ofs_ > maxOfs)
			ofs_ = maxOfs;
		k_ = lastOfs;
		lastOfs = _hInt - ofs_;
		ofs_ = _hInt - k_;
	}
	else {

		const int64_t maxOfs = _n - _hInt;
		while (ofs_ < maxOfs) {
			IFLT(_key, _a[ofs_])
				break;
			lastOfs = ofs_;
			ofs_ = (ofs_ << 1) + 1;
		}
		if (ofs_ > maxOfs)
			ofs_ = maxOfs;
		lastOfs += _hInt;
		ofs_ += _hInt;
		}
		_a -= _hInt;

		++lastOfs;
		while (lastOfs < ofs_) {
			int64_t m_ = lastOfs + ((ofs_ - lastOfs) >> 1);

			IFLT(_key, _a[m_])
				ofs_ = m_;                   
		else
			lastOfs = m_ + 1;            
		}
		return ofs_;

	fail:
		return -1;
}

static void merge_init(MergeState* _ms, int64_t _listSize, int _hasKeyFunc,
	SortSlice* lo)
{
	if (_hasKeyFunc) {
		_ms->alloced_ = (_listSize + 1) / 2;

		if (MERGESTATE_TEMP_SIZE / 2 < _ms->alloced_)
			_ms->alloced_ = MERGESTATE_TEMP_SIZE / 2;
		_ms->a_.values_ = &_ms->tempArray[_ms->alloced_];
	}
	else {
		_ms->alloced_ = MERGESTATE_TEMP_SIZE;
		_ms->a_.values_ = nullptr;
	}
	_ms->a_.keys_ = _ms->tempArray;
	_ms->n_ = 0;
	_ms->minGallop = 7;
	_ms->listLen = _listSize;
	_ms->baseKeys = lo->keys_;
}

static void merge_freemem(MergeState* _ms)
{
	if (_ms->a_.keys_ != _ms->tempArray) {
		alifMem_objFree(_ms->a_.keys_);
		_ms->a_.keys_ = nullptr;
	}
}

static int merge_getmem(MergeState* _ms, int64_t _need)
{
	int multiplier_;

	if (_need <= _ms->alloced_)
		return 0;

	multiplier_ = _ms->a_.values_ != nullptr ? 2 : 1;
	merge_freemem(_ms);
	if ((size_t)_need > LLONG_MAX / sizeof(AlifObject*) / multiplier_) {
		return -1;
	}
	_ms->a_.keys_ = (AlifObject**)alifMem_objAlloc(multiplier_ * _need
		* sizeof(AlifObject*));
	if (_ms->a_.keys_ != nullptr) {
		_ms->alloced_ = _need;
		if (_ms->a_.values_ != nullptr)
			_ms->a_.values_ = &_ms->a_.keys_[_need];
		return 0;
	}
	return -1;
}
#define MERGE_GETMEM(_MS, _NEED) ((_NEED) <= (_MS)->alloced_ ? 0 :   \
                                merge_getmem(_MS, _NEED))


static int64_t merge_lo(MergeState* _ms, SortSlice _sSa, int64_t _na,
	SortSlice _sSb, int64_t _nb)
{
	int64_t k_;
	SortSlice dest_;
	int result_ = -1;           
	int64_t minGallop;

	if (MERGE_GETMEM(_ms, _na) < 0)
		return -1;
	sortSlice_memcpy(&_ms->a_, 0, &_sSa, 0, _na);
	dest_ = _sSa;
	_sSa = _ms->a_;

	sortSlice_copy_incr(&dest_, &_sSb);
	--_nb;
	if (_nb == 0)
		goto Succeed;
	if (_na == 1)
		goto CopyB;

	minGallop = _ms->minGallop;
	for (;;) {
		int64_t aCount = 0;         
		int64_t bCount = 0;        
		for (;;) {
			k_ = ISLT(_sSb.keys_[0], _sSa.keys_[0]);
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_copy_incr(&dest_, &_sSb);
				++bCount;
				aCount = 0;
				--_nb;
				if (_nb == 0)
					goto Succeed;
				if (bCount >= minGallop)
					break;
			}
			else {
				sortSlice_copy_incr(&dest_, &_sSa);
				++aCount;
				bCount = 0;
				--_na;
				if (_na == 1)
					goto CopyB;
				if (aCount >= minGallop)
					break;
			}
		}

		++minGallop;
		do {
			minGallop -= minGallop > 1;
			_ms->minGallop = minGallop;
			k_ = gallop_right(_ms, _sSb.keys_[0], _sSa.keys_, _na, 0);
			aCount = k_;
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_memcpy(&dest_, 0, &_sSa, 0, k_);
				sortSlice_advance(&dest_, k_);
				sortSlice_advance(&_sSa, k_);
				_na -= k_;
				if (_na == 1)
					goto CopyB;

				if (_na == 0)
					goto Succeed;
			}
			sortSlice_copy_incr(&dest_, &_sSb);
			--_nb;
			if (_nb == 0)
				goto Succeed;

			k_ = gallop_left(_ms, _sSa.keys_[0], _sSb.keys_, _nb, 0);
			bCount = k_;
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_memmove(&dest_, 0, &_sSb, 0, k_);
				sortSlice_advance(&dest_, k_);
				sortSlice_advance(&_sSb, k_);
				_nb -= k_;
				if (_nb == 0)
					goto Succeed;
			}
			sortSlice_copy_incr(&dest_, &_sSa);
			--_na;
			if (_na == 1)
				goto CopyB;
		} while (aCount >= MIN_GALLOP || bCount >= MIN_GALLOP);
		++minGallop;         
		_ms->minGallop = minGallop;
	}
Succeed:
	result_ = 0;
Fail:
	if (_na)
		sortSlice_memcpy(&dest_, 0, &_sSa, 0, _na);
	return result_;
CopyB:

	sortSlice_memmove(&dest_, 0, &_sSb, 0, _nb);
	sortSlice_copy(&dest_, _nb, &_sSa, 0);
	return 0;
}

static int64_t merge_hi(MergeState* _ms, SortSlice _sSa, int64_t _na,
	SortSlice _sSb, int64_t _nb)
{
	int64_t k_;
	SortSlice dest_, basea_, baseb_;
	int result_ = -1;          
	int64_t minGallop;


	if (MERGE_GETMEM(_ms, _nb) < 0)
		return -1;
	dest_ = _sSb;
	sortSlice_advance(&dest_, _nb - 1);
	sortSlice_memcpy(&_ms->a_, 0, &_sSb, 0, _nb);
	basea_ = _sSa;
	baseb_ = _ms->a_;
	_sSb.keys_ = _ms->a_.keys_ + _nb - 1;
	if (_sSb.values_ != nullptr)
		_sSb.values_ = _ms->a_.values_ + _nb - 1;
	sortSlice_advance(&_sSa, _na - 1);

	sortSlice_copy_decr(&dest_, &_sSa);
	--_na;
	if (_na == 0)
		goto Succeed;
	if (_nb == 1)
		goto CopyA;

	minGallop = _ms->minGallop;
	for (;;) {
		int64_t aCount = 0;        
		int64_t bCount = 0;          

		for (;;) {
			k_ = ISLT(_sSb.keys_[0], _sSa.keys_[0]);
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_copy_decr(&dest_, &_sSa);
				++aCount;
				bCount = 0;
				--_na;
				if (_na == 0)
					goto Succeed;
				if (aCount >= minGallop)
					break;
			}
			else {
				sortSlice_copy_decr(&dest_, &_sSb);
				++bCount;
				aCount = 0;
				--_nb;
				if (_nb == 1)
					goto CopyA;
				if (bCount >= minGallop)
					break;
			}
		}

		++minGallop;
		do {
			minGallop -= minGallop > 1;
			_ms->minGallop = minGallop;
			k_ = gallop_right(_ms, _sSb.keys_[0], basea_.keys_, _na, _na - 1);
			if (k_ < 0)
				goto Fail;
			k_ = _na - k_;
			aCount = k_;
			if (k_) {
				sortSlice_advance(&dest_, -k_);
				sortSlice_advance(&_sSa, -k_);
				sortSlice_memmove(&dest_, 1, &_sSa, 1, k_);
				_na -= k_;
				if (_na == 0)
					goto Succeed;
			}
			sortSlice_copy_decr(&dest_, &_sSb);
			--_nb;
			if (_nb == 1)
				goto CopyA;

			k_ = gallop_left(_ms, _sSa.keys_[0], baseb_.keys_, _nb, _nb - 1);
			if (k_ < 0)
				goto Fail;
			k_ = _nb - k_;
			bCount = k_;
			if (k_) {
				sortSlice_advance(&dest_, -k_);
				sortSlice_advance(&_sSb, -k_);
				sortSlice_memcpy(&dest_, 1, &_sSb, 1, k_);
				_nb -= k_;
				if (_nb == 1)
					goto CopyA;
				if (_nb == 0)
					goto Succeed;
			}
			sortSlice_copy_decr(&dest_, &_sSa);
			--_na;
			if (_na == 0)
				goto Succeed;
		} while (aCount >= MIN_GALLOP || bCount >= MIN_GALLOP);
		++minGallop;           
		_ms->minGallop = minGallop;
	}
Succeed:
	result_ = 0;
Fail:
	if (_nb)
		sortSlice_memcpy(&dest_, -(_nb - 1), &baseb_, 0, _nb);
	return result_;
CopyA:

	sortSlice_memmove(&dest_, 1 - _na, &_sSa, 1 - _na, _na);
	sortSlice_advance(&dest_, -_na);
	sortSlice_advance(&_sSa, -_na);
	sortSlice_copy(&dest_, 0, &_sSb, 0);
	return 0;
}

static int64_t merge_at(MergeState* _ms, int64_t _i)
{
	SortSlice sSa, sSb;
	int64_t na_, nb_;
	int64_t k_;

	sSa = _ms->pending_[_i].base_;
	na_ = _ms->pending_[_i].len_;
	sSb = _ms->pending_[_i + 1].base_;
	nb_ = _ms->pending_[_i + 1].len_;

	_ms->pending_[_i].len_ = na_ + nb_;
	if (_i == _ms->n_ - 3)
		_ms->pending_[_i + 1] = _ms->pending_[_i + 2];
	--_ms->n_;

	k_ = gallop_right(_ms, *sSb.keys_, sSa.keys_, na_, 0);
	if (k_ < 0)
		return -1;
	sortSlice_advance(&sSa, k_);
	na_ -= k_;
	if (na_ == 0)
		return 0;

	nb_ = gallop_left(_ms, sSa.keys_[na_ - 1], sSb.keys_, nb_, nb_ - 1);
	if (nb_ <= 0)
		return nb_;
	if (na_ <= nb_)
		return merge_lo(_ms, sSa, na_, sSb, nb_);
	else
		return merge_hi(_ms, sSa, na_, sSb, nb_);
}

static int powerLoop(int64_t _s1, int64_t _n1, int64_t _n2, int64_t _n)
{
	int result_ = 0;
	int64_t a_ = 2 * _s1 + _n1;
	int64_t b_ = a_ + _n1 + _n2;
	for (;;) {
		++result_;
		if (a_ >= _n) {
			a_ -= _n;
			b_ -= _n;
		}
		else if (b_ >= _n) {
			break;
		}
		a_ <<= 1;
		b_ <<= 1;
	}
	return result_;
}

static int found_new_run(MergeState* _ms, int64_t _n2)
{
	if (_ms->n_) {
		class SSlice* p_ = _ms->pending_;
		int64_t s1_ = p_[_ms->n_ - 1].base_.keys_ - _ms->baseKeys; /* start index */
		int64_t n1_ = p_[_ms->n_ - 1].len_;
		int power_ = powerLoop(s1_, n1_, _n2, _ms->listLen);
		while (_ms->n_ > 1 && p_[_ms->n_ - 2].power_ > power_) {
			if (merge_at(_ms, _ms->n_ - 2) < 0)
				return -1;
		}
		p_[_ms->n_ - 1].power_ = power_;
	}
	return 0;
}

static int merge_force_collapse(MergeState* _ms)
{
	class SSlice* p = _ms->pending_;

	while (_ms->n_ > 1) {
		int64_t n_ = _ms->n_ - 2;
		if (n_ > 0 && p[n_ - 1].len_ < p[n_ + 1].len_)
			--n_;
		if (merge_at(_ms, n_) < 0)
			return -1;
	}
	return 0;
}

static int64_t merge_compute_minrun(int64_t _n)
{
	int64_t r = 0;         

	while (_n >= MAX_MINRUN) {
		r |= _n & 1;
		_n >>= 1;
	}
	return _n + r;
}


static int safe_object_compare(AlifObject* v, AlifObject* w, MergeState* _ms)
{
	return alifObject_richCompareBool(v, w, ALIF_LT);
}

static int unsafe_object_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	AlifObject* resObj; int res_;

	if (ALIF_TYPE(_v)->richCompare != _ms->KeyRichcompare)
		return alifObject_richCompareBool(_v, _w, ALIF_LT);

	resObj = (*(_ms->KeyRichcompare))(_v, _w, ALIF_LT);

	//if (resObj == NotImplemented) {
		//ALIF_DECREF(resObj);
		//return alifObject_richCompareBool(v, w, ALIF_LT);
	//}
	if (resObj == nullptr)
		return -1;

	if ((resObj->type_ == &typeBool)) {
		res_ = (resObj == ALIF_TRUE);
	}
	else {
		res_ = alifObject_isTrue(resObj);
	}
	ALIF_DECREF(resObj);

	return res_;
}

static int unsafe_long_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	AlifIntegerObject* vl_, * wl_;
	intptr_t v0_, w0_;
	int res_;

	vl_ = (AlifIntegerObject*)_v;
	wl_ = (AlifIntegerObject*)_w;

	res_ = alifObject_richCompareBool(_v,_w,ALIF_LT);

	return res_;
}

static int unsafe_float_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	int res_;
	res_ = alifFloat_asLongDouble(_v) < alifFloat_asLongDouble(_w);
	return res_;
}

static int unsafe_tuple_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	AlifTupleObject* vt_, * wt_;
	int64_t i_, vLen, wLen;
	int k_;
	vt_ = (AlifTupleObject*)_v;
	wt_ = (AlifTupleObject*)_w;

	vLen = ((AlifVarObject*)vt_)->size_;
	wLen = ((AlifVarObject*)wt_)->size_;

	for (i_ = 0; i_ < vLen && i_ < wLen; i_++) {
		k_ = alifObject_richCompareBool(vt_->items[i_], wt_->items[i_], ALIF_EQ);
		if (k_ < 0)
			return -1;
		if (!k_)
			break;
	}

	if (i_ >= vLen || i_ >= wLen)
		return vLen < wLen;

	if (i_ == 0)
		return _ms->TupleElemCompare(vt_->items[i_], wt_->items[i_], _ms);
	else
		return alifObject_richCompareBool(vt_->items[i_], wt_->items[i_], ALIF_LT);
}

static AlifObject* list_sort_impl(AlifListObject* _self, AlifObject* _keyFunc, int _reverse)
{
	MergeState ms_;
	int64_t nRemaining;
	int64_t minRun;
	SortSlice lo_;
	int64_t savedObSize, savedAllocated;
	AlifObject** savedObItem;
	AlifObject** finalObItem;
	AlifObject* result_ = nullptr;           
	int64_t i_;
	AlifObject** keys_;

	if (_keyFunc == ALIF_NONE)
		_keyFunc = nullptr;

	savedObSize = ((AlifVarObject*)_self)->size_;
	savedObItem = _self->items;
	savedAllocated = _self->allocate;
	((AlifVarObject*)_self)->size_ = 0;
	_self->items = nullptr;
	_self->allocate = -1; 

	if (_keyFunc == nullptr) {
		keys_ = nullptr;
		lo_.keys_ = savedObItem;
		lo_.values_ = nullptr;
	}
	else {
		if (savedObSize < MERGESTATE_TEMP_SIZE / 2)
			keys_ = &ms_.tempArray[savedObSize + 1];
		else {
			keys_ = (AlifObject**)alifMem_objAlloc(sizeof(AlifObject*) * savedObSize);
			if (keys_ == nullptr) {
				goto keyfunc_fail;
			}
		}

		for (i_ = 0; i_ < savedObSize; i_++) {
			//keys_[i_] = AlifObject_CallOneArg(_keyFunc, savedObItem[i_]);
			if (keys_[i_] == nullptr) {
				for (i_ = i_ - 1; i_ >= 0; i_--)
					ALIF_DECREF(keys_[i_]);
				if (savedObSize >= MERGESTATE_TEMP_SIZE / 2)
					alifMem_objFree(keys_);
				goto keyfunc_fail;
			}
		}

		lo_.keys_ = keys_;
		lo_.values_ = savedObItem;
	}
	if (savedObSize > 1) {
		int keysAreInTuples = (ALIF_IS_TYPE(lo_.keys_[0], &typeTuple) &&
			((AlifVarObject*)lo_.keys_[0])->size_ > 0);

		AlifTypeObject* keyType = (keysAreInTuples ?
			ALIF_TYPE(((AlifTupleObject*)lo_.keys_[0])->items[0]) :
			ALIF_TYPE(lo_.keys_[0]));

		int keysAreAllSameType = 1;
		int intsAreBounded = 1;

		for (i_ = 0; i_ < savedObSize; i_++) {

			if (keysAreInTuples &&
				!(ALIF_IS_TYPE(lo_.keys_[i_], &typeTuple) && ((AlifVarObject*)lo_.keys_[i_])->size_ != 0)) {
				keysAreInTuples = 0;
				keysAreAllSameType = 0;
				break;
			}

			AlifObject* key = (keysAreInTuples ?
				((AlifTupleObject*)lo_.keys_[i_])->items[0] :
				lo_.keys_[i_]);

			if (!ALIF_IS_TYPE(key, keyType)) {
				keysAreAllSameType = 0;
				if (!keysAreInTuples) {
					break;
				}
			}

			if (keysAreAllSameType) {
				if (keyType == &_alifIntegerType_ &&
					intsAreBounded ) {
					intsAreBounded = 0;
				}
			}
		}

		if (keysAreAllSameType) {

			if (keyType == &_alifIntegerType_ && intsAreBounded) {
				ms_.KeyCompare = unsafe_long_compare;
			}
			else if (keyType == &_typeFloat_) {
				ms_.KeyCompare = unsafe_float_compare;
			}
			else if ((ms_.KeyRichcompare = keyType->richCompare) != nullptr) {
				ms_.KeyCompare = unsafe_object_compare;
			}
			else {
				ms_.KeyCompare = safe_object_compare;
			}
		}
		else {
			ms_.KeyCompare = safe_object_compare;
		}

		if (keysAreInTuples) {
			if (keyType == &typeTuple) {
				ms_.TupleElemCompare = safe_object_compare;
			}
			else {
				ms_.TupleElemCompare = ms_.KeyCompare;
			}

			ms_.KeyCompare = unsafe_tuple_compare;
		}
	}
	merge_init(&ms_, savedObSize, keys_ != nullptr, &lo_);

	nRemaining = savedObSize;
	if (nRemaining < 2)
		goto succeed;

	if (_reverse) {
		if (keys_ != nullptr)
			reverse_slice(&keys_[0], &keys_[savedObSize]);
		reverse_slice(&savedObItem[0], &savedObItem[savedObSize]);
	}

	minRun = merge_compute_minrun(nRemaining);
	do {
		int64_t n;

		n = count_run(&ms_, &lo_, nRemaining);
		if (n < 0)
			goto fail;
		if (n < minRun) {
			const int64_t force = nRemaining <= minRun ?
				nRemaining : minRun;
			if (binarysort(&ms_, &lo_, force, n) < 0)
				goto fail;
			n = force;
		}

		if (found_new_run(&ms_, n) < 0)
			goto fail;
		ms_.pending_[ms_.n_].base_ = lo_;
		ms_.pending_[ms_.n_].len_ = n;
		++ms_.n_;
		sortSlice_advance(&lo_, n);
		nRemaining -= n;
	} while (nRemaining);

	if (merge_force_collapse(&ms_) < 0)
		goto fail;
	lo_ = ms_.pending_[0].base_;

succeed:
	result_ = ALIF_NONE;
fail:
	if (keys_ != nullptr) {
		for (i_ = 0; i_ < savedObSize; i_++)
			ALIF_DECREF(keys_[i_]);
		if (savedObSize >= MERGESTATE_TEMP_SIZE / 2)
			alifMem_objFree(keys_);
	}

	if (_self->allocate != -1 && result_ != nullptr) {
		result_ = nullptr;
	}

	if (_reverse && savedObSize > 1)
		reverse_slice(savedObItem, savedObItem + savedObSize);

	merge_freemem(&ms_);

keyfunc_fail:
	finalObItem = _self->items;
	i_ = ((AlifVarObject*)_self)->size_;
	((AlifVarObject*)_self)->size_= savedObSize;
	_self->items = savedObItem;
	_self->allocate = savedAllocated;
	if (finalObItem != nullptr) {
		while (--i_ >= 0) {
			ALIF_XDECREF(finalObItem[i_]);
		}
#ifdef ALIF_GIL_DISABLED
		bool useQSbr = alifSubObject_GC_is_shared(_self);
#else
		bool useQSbr = false;
#endif
		//free_list_items(finalObItem, useQSbr);
	}
	return ALIF_XNEWREF(result_);
}


int alifList_sort(AlifObject* _v)
{
	if (_v == nullptr || !(_v->type_ == &typeList)) {
		return -1;
	}
	//ALIF_BEGIN_CRITICAL_SECTION(_v);
	_v = list_sort_impl((AlifListObject*)_v, nullptr, 0);
	//ALIF_END_CRITICAL_SECTION();
	if (_v == nullptr)
		return -1;
	ALIF_DECREF(_v);
	return 0;
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

        int k_ = alifObject_richCompareBool(vitem, witem, ALIF_EQ);
        if (k_ < 0)
            return nullptr;
        if (!k_)
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
        AlifObject* result_;
        AlifObject* it;
        AlifObject** src, ** dest_;

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
            result_ = list_new_prealloc(slicelength);
            if (!result_) return nullptr;

            src = self->items;
            dest_ = ((AlifListObject*)result_)->items;
            for (cur = start_, i = 0; i < slicelength;
                cur += (size_t)step_, i++) {
                it = src[cur];
                dest_[i] = it;
            }
            ((AlifVarObject*)result_)->size_ = slicelength;
            return result_;
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
            //if (cur < (size_t)((AlifVarObject*)_self)->size_) {
                //memmove(_self->items + cur - slicelength,
                    //_self->items + cur,
                    //(((AlifVarObject*)_self)->size_ - cur) *
                    //sizeof(AlifObject*));
            //}

            ((AlifVarObject*)self)->size_ = ((AlifVarObject*)self)->size_ - slicelength;
            res = list_resize(self, ((AlifVarObject*)self)->size_);

            alifMem_dataFree(garbage);

            return res;
        }
        else {
            AlifObject* ins, * seq;
            AlifObject** garbage, ** seqitems, ** selfitems;
            int64_t i;
            size_t cur;

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
