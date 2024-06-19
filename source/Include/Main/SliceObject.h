#pragma once


class  AlifSliceObject {
public:
    ALIFOBJECT_HEAD;
    AlifObject* start_{}; /* not nullptr */
    AlifObject* stop_{}; /* not nullptr */
    AlifObject* step_{}; /* not nullptr */
};

extern AlifInitObject _typeSlice_;

#define ALIFSLICE_CHECK(op) ALIF_IS_TYPE((op), &_typeSlice_)


AlifObject* alifNew_slice(AlifObject* , AlifObject* , AlifObject* );

int slice_getIndices(AlifSliceObject* , int64_t , int64_t* , int64_t* , int64_t* );

int slice_getIndicesEx(AlifObject* , int64_t , int64_t* , int64_t* , int64_t* ,int64_t* );


#define ALIFSLICE_GETINDICESEX(slice, length, start_, stop_, step_, sliceLen) (  \
    slice_unpack((slice), (start_), (stop_), (step_)) < 0 ? ((*(sliceLen) = 0), -1) :     \
    ((*(sliceLen) = alifSlice_adjustIndices((length), (start_), (stop_), *(step_))), 0))



int slice_unpack(AlifSliceObject* ,int64_t* , int64_t* , int64_t* );

int64_t alifSlice_adjustIndices(int64_t , int64_t* , int64_t* , int64_t );