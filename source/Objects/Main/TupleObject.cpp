#include "alif.h"

#include "AlifCore_Memory.h"

AlifObject* alifNew_tuple(SSIZE_T _size) {

	if (_size == 0) {
		// return empty tuple
		
	}

    //AlifTupleObject* object{};
	AlifTupleObject* object = (AlifTupleObject*)alifMem_objAlloc(_size + 1); // temp and need review

    for (size_t i = 0; i < _size; i++) {
        object->items_[i] = nullptr;
    }

	return (AlifObject*)object;
}

size_t tuple_length(AlifObject* object) {

    return ((AlifVarObject*)(object))->size_;

}

AlifObject* tuple_getItem(AlifTupleObject* tuple, size_t index) {

    if (tuple->_base_.size_ <= index) {
        std::wcout << L"مؤشر المترابطة في عمليه احضار كائن خارج النطاق\n" << std::endl;
        exit(-1);
    }

    return tuple->items_[index];

}

bool tuple_setItem(AlifTupleObject* tuple, size_t index, AlifObject* newItem) {

    if (tuple->_base_.size_ <= index) {
        std::wcout << L"مؤشر المترابطة في عمليه اسناد كائن خارج النطاق\n" << std::endl;
        exit(-1);
    }

    AlifObject** pointer = ((AlifTupleObject*)tuple)->items_ + index;
    ALIF_XSETREF(*pointer, newItem);
    return true;
}

AlifObject* tuple_pack(size_t size_, ...) {

    if (size_ == 0) {
        // return empty
    }

    va_list vArgs;
    va_start(vArgs, size_);
    AlifTupleObject* result = (AlifTupleObject*)alifNew_tuple(size_);

    AlifObject** items_ = result->items_;
    for (size_t i = 0; i < size_; i++)
    {
        items_[i] = va_arg(vArgs, AlifObject*);
    }
    va_end(vArgs);
    return (AlifObject*)result;

}

void tuple_dealloc(AlifTupleObject* object) {
    


    alifMem_objFree(object);

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

int tuple_contain(AlifTupleObject* object, AlifObject* item) {

    int compare = 0;

    for (size_t i = 0; i < ((AlifVarObject*)(object))->size_ && compare == 0; i++)
    {
        compare = alifObject_richCompareBool(object->items_[i], item, ALIF_EQ);
    }
    return compare;

}

AlifObject* alifSubTuple_fromArray(AlifObject *const *object, size_t size_) {

    if (size_ == 0) {
        // return empty tuple
    }

    AlifTupleObject* tuple = (AlifTupleObject*)alifNew_tuple(size_);

	if (tuple == nullptr) {
		return nullptr;
	}
	AlifObject** dst_= tuple->items_;
	for (int64_t i = 0; i < size_; i++) {
		AlifObject* item = object[i];
		dst_[i] = ALIF_NEWREF(item);
	}

    return (AlifObject*)tuple;

}

AlifObject* tupleslice(AlifTupleObject* tuple, size_t iLow,
    size_t iHigh)
{
    if (iLow < 0)
        iLow = 0;
    if (iHigh > tuple->_base_.size_)
        iHigh = tuple->_base_.size_;
    if (iHigh < iLow)
        iHigh = iLow;
    if (iLow == 0 && iHigh == tuple->_base_.size_ && 
        tuple->_base_._base_.type_ == &_alifTupleType_) {
        return (AlifObject*)tuple;
    }
    return alifSubTuple_fromArray(tuple->items_ + iLow, iHigh - iLow);
}

AlifObject* tuple_getSlice(AlifObject* tuple, size_t index, size_t index2)
{
    if (tuple == nullptr || tuple->type_ != &_alifTupleType_) {
        std::wcout << L"نوع فارغ او انه غير صحيح في عملية احضار جزء من مترابطة\n"  << std::endl;
        exit(-1);
    }
    return tupleslice((AlifTupleObject*)tuple, index, index2);
}

AlifObject* tuple_compare(AlifObject* v, AlifObject* w, int op) {

    if (v->type_ != &_alifTupleType_ || w->type_ != &_alifTupleType_) {
        // not implement
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

AlifSequenceMethods seqTuple = {
    (LenFunc)tuple_length,                       /* sq_length */
    0,                    /* sq_concat */
    0,                  /* sq_repeat */
    (SSizeArgFunc)tuple_getItem,                    /* sq_item */
    0,                                          /* sq_slice */
    0,                                          /* sq_ass_item */
    0,                                          /* sq_ass_slice */
    (ObjObjProc)tuple_contain,                  /* sq_contains */
};

AlifInitObject _alifTupleType_ = {
    0,
    0,
    0,
    L"tuple",
    sizeof(AlifTupleObject),
    sizeof(AlifObject*),
    (Destructor)tuple_dealloc,         
    0,                                          
    0,                                
    0,                                
    0,           
    0,                                  
    &seqTuple,                   
    0,                   
    (HashFunc)tupel_hash,                    
    0,                                      
    0,                                     
    0,                    
    0,                                         
    0,
    0,       
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
    0,                          
    //.tp_vectorcall = tuple_vectorcall,
};
