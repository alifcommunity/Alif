#include "alif.h"

#include "AlifCore_Memory.h"

AlifObject* alifNew_tuple(SSIZE_T size_) {

	if (size_ == 0) {
		// return empty tuple
	}

    AlifTupleObject* object{};
	//AlifTupleObject* object = (AlifTupleObject*)a(&typeTuple, size_);

    for (size_t i = 0; i < size_; i++) {
        object->items[i] = nullptr;
    }

	return (AlifObject*)object;
}

size_t tuple_length(AlifObject* object) {

    return ((AlifVarObject*)(object))->size_;

}

AlifObject* tuple_getItem(AlifTupleObject* tuple, size_t index) {

    if (tuple->object.size_ <= index) {
        std::wcout << L"مؤشر المترابطة في عمليه احضار كائن خارج النطاق\n" << std::endl;
        exit(-1);
    }

    return tuple->items[index];

}

bool tuple_setItem(AlifTupleObject* tuple, size_t index, AlifObject* newItem) {

    if (tuple->object.size_ <= index) {
        std::wcout << L"مؤشر المترابطة في عمليه اسناد كائن خارج النطاق\n" << std::endl;
        exit(-1);
    }

    AlifObject** pointer = ((AlifTupleObject*)tuple)->items + index;
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

    AlifObject** items = result->items;
    for (size_t i = 0; i < size_; i++)
    {
        items[i] = va_arg(vArgs, AlifObject*);
    }
    va_end(vArgs);
    return (AlifObject*)result;

}

void tuple_dealloc(AlifTupleObject* object) {
    


    alifMem_objFree(object);

}

size_t tupel_hash(AlifTupleObject* object) {

    size_t length = ((AlifVarObject*)(object))->size_;
    AlifObject** items = object->items;


    size_t hash = 2870177450012600261ULL;
    for (size_t i = 0; i < length; i++)
    {
        size_t hashItem = alifObject_hash(items[i]);

        hash += hashItem;
        hash *= 11400714785074694791ULL;
    }

    return hash;
}

int tuple_contain(AlifTupleObject* object, AlifObject* item) {

    int compare = 0;

    for (size_t i = 0; i < ((AlifVarObject*)(object))->size_ && compare == 0; i++)
    {
        compare = alifObject_richCompareBool(object->items[i], item, ALIF_EQ);
    }
    return compare;

}

AlifObject* tuple_fromArray(AlifObject *const *object, size_t size_) {

    if (size_ == 0) {
        // return empty tuple
    }

    AlifTupleObject* tuple = (AlifTupleObject*)alifNew_tuple(size_);

    for (size_t i = 0; i < size_; i++)
    {
        tuple->items[i] = object[i];
    }
    return (AlifObject*)tuple;

}

AlifObject* tupleslice(AlifTupleObject* tuple, size_t iLow,
    size_t iHigh)
{
    if (iLow < 0)
        iLow = 0;
    if (iHigh > tuple->object.size_)
        iHigh = tuple->object.size_;
    if (iHigh < iLow)
        iHigh = iLow;
    if (iLow == 0 && iHigh == tuple->object.size_ && 
        tuple->object._base_.type_ == &typeTuple) {
        return (AlifObject*)tuple;
    }
    return tuple_fromArray(tuple->items + iLow, iHigh - iLow);
}

AlifObject* tuple_getSlice(AlifObject* tuple, size_t index, size_t index2)
{
    if (tuple == nullptr || tuple->type_ != &typeTuple) {
        std::wcout << L"نوع فارغ او انه غير صحيح في عملية احضار جزء من مترابطة\n"  << std::endl;
        exit(-1);
    }
    return tupleslice((AlifTupleObject*)tuple, index, index2);
}

AlifObject* tuple_compare(AlifObject* v, AlifObject* w, int op) {

    if (v->type_ != &typeTuple || w->type_ != &typeTuple) {
        // not implement
    }

    AlifTupleObject* vTuple = (AlifTupleObject*)v;
    AlifTupleObject* wTuple = (AlifTupleObject*)w;

    size_t vLen = vTuple->object.size_,
        wLen = wTuple->object.size_,
        i;

    for (i = 0; i < vLen && i < wLen; i++)
    {
        int compare = alifObject_richCompareBool(vTuple->items[i],
            wTuple->items[i], ALIF_EQ);
    
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
    
    return alifObject_richCompare(vTuple->items[i], wTuple->items[i], op);

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

AlifInitObject typeTuple = {
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