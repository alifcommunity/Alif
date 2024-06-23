#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_GC.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"

static AlifTupleObject* tuple_alloc(int64_t _size)
{
	if (_size < 0) {
		return NULL;
	}

	AlifTupleObject* op = nullptr;
	if (op == nullptr) {
		/* Check for overflow */
		if ((size_t)_size > ((size_t)LLONG_MAX - (sizeof(AlifTupleObject) -
			sizeof(AlifObject*))) / sizeof(AlifObject*)) {
			return nullptr;
		}
		op = ALIFOBJECT_GC_NEWVAR(AlifTupleObject, &_alifTupleType_, _size);
		if (op == nullptr)
			return nullptr;
	}
	return op;
}

static inline AlifObject* tuple_getEmpty() {
	return (AlifObject*)&ALIF_SINGLETON(tupleEmpty);
}

AlifObject* alifNew_tuple(AlifSizeT _size)
{
	AlifTupleObject* op_{};
	if (_size == 0) {
		return tuple_getEmpty();
	}
	op_ = tuple_alloc(_size);
	if (op_ == nullptr) {
		return nullptr;
	}
	for (AlifSizeT i = 0; i < _size; i++) {
		op_->items_[i] = nullptr;
	}
	ALIFOBJECT_GC_TRACK(op_);
	return (AlifObject*)op_;
}

int64_t alifTuple_size(AlifObject* _op)
{
	if (!(_op->type_ == &_alifTupleType_)) {
		return -1;
	}
	else
		return ALIF_SIZE(_op);
}

AlifObject* alifTuple_getItem(AlifObject* _op, int64_t _i)
{
	if (!(_op->type_ == &_alifTupleType_)) {
		return NULL;
	}
	if (_i < 0 || _i >= ALIF_SIZE(_op)) {
		return NULL;
	}
	return ((AlifTupleObject*)_op)->items_[_i];
}

//int alifTuple_setItem(AlifObject* _op, int64_t _i, AlifObject* _newItem)
//{
//	AlifObject** p_;
//	if (!(_op->type_ == &_alifTupleType_) || ALIF_REFCNT(_op) != 1) {
//		ALIF_XDECREF(_newItem);
//		return -1;
//	}
//	if (_i < 0 || _i >= ALIF_SIZE(_op)) {
//		ALIF_XDECREF(_newItem);
//		return -1;
//	}
//	p_ = ((AlifTupleObject*)_op)->items_ + _i;
//	ALIF_XSETREF(*p_, _newItem);
//	return 0;
//}

AlifObject* tuple_pack(size_t _size, ...) {

	int64_t i_;
	AlifObject* o_;
	AlifObject** items_;
	va_list vArgs;

	va_start(vArgs, _size);
	AlifTupleObject* result = tuple_alloc(_size);
	if (result == NULL) {
		va_end(vArgs);
		return NULL;
	}
	items_ = result->items_;
	for (i_ = 0; i_ < _size; i_++) {
		o_ = va_arg(vArgs, AlifObject*);
		items_[i_] = ALIF_NEWREF(o_);
	}
	va_end(vArgs);
	ALIFOBJECT_GC_TRACK(result);
	return (AlifObject*)result;

}

static void tuple_dealloc(AlifTupleObject* _op)
{
	alifObject_gcUnTrack(_op);
	//ALIF_TRASHCAN_BEGIN(_op, tuple_dealloc)

	int64_t i = ALIF_SIZE(_op);
	while (--i >= 0) {
		ALIF_XDECREF(_op->items_[i]);
	}
	//if (!maybe_freeList_push(_op)) {
		ALIF_TYPE(_op)->free_((AlifObject*)_op);
	//}

	//ALIF_TRASHCAN_END
}

size_t tupel_hash(AlifTupleObject* object) {

    size_t length = ((AlifVarObject*)(object))->size_;
    AlifObject** items_ = object->items_;


    size_t hash = 2870177450012600261ULL;
    for (size_t i = 0; i < length; i++)
    {
        size_t hashItem = alifObject_hash(items_[i]);

        hash += hashItem;
        hash *= 11400714785074694791ULL;
    }

    return hash;
}

static size_t tuple_length(AlifTupleObject* _a)
{
	return ALIF_SIZE(_a);
}

int tuple_contain(AlifTupleObject* _object, AlifObject* _item) {

    int compare_ = 0;

    for (size_t i = 0; i < ((AlifVarObject*)(_object))->size_ && compare_ == 0; i++)
    {
		compare_ = alifObject_richCompareBool(_object->items_[i], _item, ALIF_EQ);
    }
    return compare_;

}

static AlifObject* tuple_item(AlifTupleObject* _a, int64_t _i)
{
	if (_i < 0 || _i >= ALIF_SIZE(_a)) {
		return NULL;
	}
	return ALIF_NEWREF(_a->items_[_i]);
}

AlifObject* alifSubTuple_fromArray(AlifObject *const *_object, size_t _size) {

	AlifTupleObject* tuple_ = tuple_alloc(_size);

	if (tuple_ == nullptr) {
		return nullptr;
	}
	AlifObject** dst_= tuple_->items_;
	for (int64_t i_ = 0; i_ < _size; i_++) {
		AlifObject* item_ = _object[i_];
		dst_[i_] = ALIF_NEWREF(item_);
	}

    return (AlifObject*)tuple_;

}

AlifObject* alifTuple_fromArraySteal(AlifObject* const* _src, AlifSizeT _n) {
	if (_n == 0) {
		return tuple_getEmpty();
	}
	AlifTupleObject* tuple = tuple_alloc(_n);
	if (tuple == nullptr) {
		for (AlifSizeT i = 0; i < _n; i++) {
			ALIF_DECREF(_src[i]);
		}
		return nullptr;
	}
	AlifObject** dst = tuple->items_;
	for (AlifSizeT i = 0; i < _n; i++) {
		AlifObject* item = _src[i];
		dst[i] = item;
	}
	ALIFOBJECT_GC_TRACK(tuple);
	return (AlifObject*)tuple;
}

AlifObject* tupleslice(AlifTupleObject* _tuple, size_t _iLow,
    size_t _iHigh)
{
    if (_iLow < 0)
        _iLow = 0;
    if (_iHigh > _tuple->_base_.size_)
        _iHigh = _tuple->_base_.size_;
    if (_iHigh < _iLow)
        _iHigh = _iLow;
    if (_iLow == 0 && _iHigh == _tuple->_base_.size_ && 
        _tuple->_base_._base_.type_ == &_alifTupleType_) {
        return (AlifObject*)_tuple;
    }
    return alifSubTuple_fromArray(_tuple->items_ + _iLow, _iHigh - _iLow);
}

AlifObject* tuple_getSlice(AlifObject* _tuple, size_t _index, size_t _index2)
{
    if (_tuple == nullptr || _tuple->type_ != &_alifTupleType_) {
		return nullptr;
    }
    return tupleslice((AlifTupleObject*)_tuple, _index, _index2);
}

AlifObject* tuple_compare(AlifObject* v, AlifObject* w, int op) {

    if (v->type_ != &_alifTupleType_ || w->type_ != &_alifTupleType_) {
		return ALIF_NOTIMPLEMENTED;
    }

    AlifTupleObject* vTuple = (AlifTupleObject*)v;
    AlifTupleObject* wTuple = (AlifTupleObject*)w;

    size_t vLen = vTuple->_base_.size_,
        wLen = wTuple->_base_.size_,
        i;

    for (i = 0; i < vLen && i < wLen; i++)
    {
        int compare = alifObject_richCompareBool(vTuple->items_[i],
            wTuple->items_[i], ALIF_EQ);
    
        if (compare < 0) {
            return nullptr;
        }
        if (!compare) {
            break;
        }
    }

    if (op == ALIF_EQ) {
        return ALIF_FALSE;
    }
    if (op == ALIF_NE) {
        return ALIF_TRUE;
    }
    
    return alifObject_richCompare(vTuple->items_[i], wTuple->items_[i], op);

}

AlifSequenceMethods _seqAsTuple_ = {
    (LenFunc)tuple_length,                       /* sq_length */
    0,                    /* sq_concat */
    0,                  /* sq_repeat */
    (SSizeArgFunc)tuple_item,                    /* sq_item */
    0,                                          /* sq_slice */
    0,                                          /* sq_ass_item */
    0,                                          /* sq_ass_slice */
    (ObjObjProc)tuple_contain,                  /* sq_contains */
};

AlifInitObject _alifTupleType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
    L"tuple",
    sizeof(AlifTupleObject) - sizeof(AlifObject*),
    sizeof(AlifObject*),
    (Destructor)tuple_dealloc,         
    0,                                          
    0,                                
    0,                                
    0,           
    0,                                  
    &_seqAsTuple_,                   
    0,                   
    (HashFunc)tupel_hash,                    
    0,                                      
    0,                                     
    0,                    
    0,                                         
    0,    
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC | ALIFTPFLAGS_BASETYPE |
	ALIFTPFLAGS_LIST_SUBCLASS | ALIFSUBTPFLAGS_MATCH_SELF | ALIFTPFLAGS_SEQUENCE,
	0,
    0,         
    0,                                
    tuple_compare,                          
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
    0,                                  
    0,                             
	alifObject_gcDel,
    //.tp_vectorcall = tuple_vectorcall,
};

//static inline int maybe_freeList_push(AlifTupleObject* _op)
//{
//#ifdef WITH_FREELISTS
//	struct alifSub_object_freelists* freelists = alifSub_object_freelists_GET();
//	if (ALIF_SIZE(op) == 0) {
//		return 0;
//	}
//	int64_t index = ALIF_SIZE(op) - 1;
//	if (index < alifTuple_NFREELISTS
//		&& TUPLE_FREELIST.numfree[index] < ALIFTuple_MAXFREELIST
//		&& TUPLE_FREELIST.numfree[index] >= 0
//		&& ALIF_IS_TYPE(op, &_alifTupleType_))
//	{
//		/* op is the head of a linked list, with the first item
//		   pointing to the next node.  Here we set op as the new head. */
//		op->ob_item[0] = (AlifObject*)TUPLE_FREELIST.items[index];
//		TUPLE_FREELIST.items[index] = op;
//		TUPLE_FREELIST.numfree[index]++;
//		OBJECT_STAT_INC(to_freelist);
//		return 1;
//	}
//#endif
//	return 0;
//}
