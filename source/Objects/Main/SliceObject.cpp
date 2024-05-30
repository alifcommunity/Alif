#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Object.h"

static AlifSliceObject* buildSlice_consume2(AlifObject* start_, AlifObject* stop_, AlifObject* step_) {


    AlifSliceObject* object = (AlifSliceObject*)alifMem_objAlloc(sizeof(AlifSliceObject*));

    alifSubObject_init((AlifObject*)object, &_typeSlice_);

    object->start_ = start_;
    object->step_ = step_;
    object->stop_ = stop_;
    return object;
}

AlifObject* alifNew_slice(AlifObject* start_, AlifObject* stop_, AlifObject* step_)
{
    if (step_ == nullptr) {
        step_ = ALIF_NONE;
    }
    if (start_ == nullptr) {
        start_ = ALIF_NONE;
    }
    if (stop_ == nullptr) {
        stop_ = ALIF_NONE;
    }
    return (AlifObject*)buildSlice_consume2(start_,
        stop_, step_);
}

AlifObject* buildSlice_consumeRefs(AlifObject* start_, AlifObject* stop_)
{
    return (AlifObject*)buildSlice_consume2(start_, stop_, ALIF_NONE);
}

AlifObject* slice_fromIndices(int64_t indexStart, int64_t indexStop)
{
    AlifObject *start_ = alifInteger_fromLongLong(indexStart);
    if (!start_)
        return nullptr;
    AlifObject* end = alifInteger_fromLongLong(indexStop);
    if (!end) {
        return nullptr;
    }

    AlifObject *slice = alifNew_slice(start_, end, nullptr);
    return slice;
}

int slice_getIndices(AlifSliceObject* object, int64_t length,
    int64_t* start_, int64_t* stop_, int64_t* step_)
{

    if (object->step_ == ALIF_NONE) {
        *step_ = 1;
    }
    else {
        if (!(object->step_->type_ == &_alifIntegerType_)) return -1;
        *step_ = alifInteger_asLongLong(object->step_);
    }
    if (object->start_ == ALIF_NONE) {
        *start_ = *step_ < 0 ? length - 1 : 0;
    }
    else {
        if (!(object->start_->type_ == &_alifIntegerType_)) return -1;
        *start_ = alifInteger_asLongLong(object->start_);
        if (*start_ < 0) *start_ += length;
    }
    if (object->stop_ == ALIF_NONE) {
        *stop_ = *step_ < 0 ? -1 : length;
    }
    else {
        if (!(object->stop_->type_ == &_alifIntegerType_)) return -1;
        *stop_ = alifInteger_asLongLong(object->stop_);
        if (*stop_ < 0) *stop_ += length;
    }
    if (*stop_ > length) return -1;
    if (*start_ >= length) return -1;
    if (*step_ == 0) return -1;
    return 0;
}

int slice_unpack(AlifSliceObject* object,
    int64_t* start_, int64_t* stop_, int64_t* step_)
{

    if (object->step_ == ALIF_NONE) {
        *step_ = 1;
    }
    else {
    //    if (!alifEval_sliceIndex(object->step_, step_)) return -1;
    //    if (*step_ == 0) {
    //        PyErr_SetString(PyExc_ValueError,
    //            "slice step_ cannot be zero");
    //        return -1;
    //    }
        if (*step_ < -LLONG_MAX)
            *step_ = -LLONG_MAX;
    }

    if (object->start_ == ALIF_NONE) {
        *start_ = *step_ < 0 ? LLONG_MAX : 0;
    }
    else {
        //if (!alifEval_sliceIndex(object->start_, start_)) return -1;
    }

    if (object->stop_ == ALIF_NONE) {
        *stop_ = *step_ < 0 ? LLONG_MIN : LLONG_MAX;
    }
    else {
        //if (!alifEval_sliceIndex(object->stop_, stop_)) return -1;
    }

    return 0;
}

int64_t slice_adjustIndices(int64_t length,
    int64_t* start_, int64_t* stop_, int64_t step_)
{

    if (*start_ < 0) {
        *start_ += length;
        if (*start_ < 0) {
            *start_ = (step_ < 0) ? -1 : 0;
        }
    }
    else if (*start_ >= length) {
        *start_ = (step_ < 0) ? length - 1 : length;
    }

    if (*stop_ < 0) {
        *stop_ += length;
        if (*stop_ < 0) {
            *stop_ = (step_ < 0) ? -1 : 0;
        }
    }
    else if (*stop_ >= length) {
        *stop_ = (step_ < 0) ? length - 1 : length;
    }

    if (step_ < 0) {
        if (*stop_ < *start_) {
            return (*start_ - *stop_ - 1) / (-step_) + 1;
        }
    }
    else {
        if (*start_ < *stop_) {
            return (*stop_ - *start_ - 1) / step_ + 1;
        }
    }
    return 0;
}

int slice_getIndicesEx(AlifObject* object, int64_t length,
    int64_t* start_, int64_t* stop_, int64_t* step_,
    int64_t* sliceLength)
{
    if (slice_unpack((AlifSliceObject*)object, start_, stop_, step_) < 0)
        return -1;
    *sliceLength = slice_adjustIndices(length, start_, stop_, *step_);
    return 0;
}

static AlifObject* slice_compare(AlifObject* v, AlifObject* w, int op)
{
    if (!(v->type_ == &_typeSlice_) || !(w->type_ == &_typeSlice_)){
        // return mot implementation
    }

    if (v == w) {
        AlifObject* res;

        switch (op) {
        case ALIF_EQ:
        case ALIF_LE:
        case ALIF_GE:
            res = ALIF_TRUE;
            break;
        default:
            res = ALIF_FALSE;
            break;
        }
        return res;
    }


    AlifObject* t1 = tuple_pack(3,
        ((AlifSliceObject*)v)->start_,
        ((AlifSliceObject*)v)->stop_,
        ((AlifSliceObject*)v)->step_);
    if (t1 == nullptr) {
        return nullptr;
    }

    AlifObject* t2 = tuple_pack(3,
        ((AlifSliceObject*)v)->start_,
        ((AlifSliceObject*)v)->stop_,
        ((AlifSliceObject*)v)->step_);
    if (t2 == nullptr) {
        return nullptr;
    }

    AlifObject* res = alifObject_richCompare(t1, t2, op);

    return res;
}

//static AlifMemberDef _sliceMembers_[] = {
//    {L"start", 6, offsetof(AlifSliceObject, start_), 1},
//    {L"stop", 6, offsetof(AlifSliceObject, stop_), 1},
//    {L"step", 6, offsetof(AlifSliceObject, step_), 1},
//    {0}
//};

#if SIZEOF_ALIF_UHASH_T > 4
#define ALIFHASH_XXPRIME_1 ((size_t)11400714785074694791ULL)
#define ALIFHASH_XXPRIME_2 ((size_t)14029467366897019727ULL)
#define ALIFHASH_XXPRIME_5 ((size_t)2870177450012600261ULL)
#define ALIFHASH_XXROTATE(x) ((x << 31) | (x >> 33))  /* Rotate left 31 bits */
#else
#define ALIFHASH_XXPRIME_1 ((size_t)2654435761UL)
#define ALIFHASH_XXPRIME_2 ((size_t)2246822519UL)
#define ALIFHASH_XXPRIME_5 ((size_t)374761393UL)
#define ALIFHASH_XXROTATE(x) ((x << 13) | (x >> 19))  /* Rotate left 13 bits */
#endif

static size_t sliceHash(AlifSliceObject* v)
{
    size_t acc = ALIFHASH_XXPRIME_5;
#define ALIFHASH_SLICE_PART(com) { \
    size_t lane = alifObject_hash(v->com); \
    if(lane == (size_t)-1) { \
        return -1; \
    } \
    acc += lane * ALIFHASH_XXPRIME_2; \
    acc = ALIFHASH_XXROTATE(acc); \
    acc *= ALIFHASH_XXPRIME_1; \
}
    ALIFHASH_SLICE_PART(start_);
    ALIFHASH_SLICE_PART(stop_);
    ALIFHASH_SLICE_PART(step_);
#undef ALIFHASH_SLICE_PART
    if (acc == (size_t)-1) {
        return 1546275796;
    }
    return acc;
}


AlifTypeObject _typeSlice_ = {
    0,
    0,
    0,
    L"slice",                    
    sizeof(AlifSliceObject),     
    0,                          
    0, //(destructor)slice_dealloc,                 
    0,                                       
    0,                                      
    0,                                      
    0, //(reprfunc)slice_repr,                
    0,                                        
    0,                                          
    0,                                         
    (HashFunc)sliceHash,                 
    0,                                   
    0,                                  
    0, //alifObject_genericGetAttr,               
    0,                                     
    0,  
    0,              
    0,
    0,       
    0,                                  
    slice_compare,                         
    0,                                    
    0,                          
    0,                              
    0, // sliceMethods,                 
    //_sliceMembers_,                 
    0,                                     
    0,                                   
    0,                                   
    0,                                        
    0,                                        
    0,                                         
    0,                                   
    0,                                    
    0, //slice_new,                                
};
