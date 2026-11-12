#pragma once


ALIFAPI_DATA(AlifObject) _alifEllipsisObject_; // 9


#define ALIF_ELLIPSIS (&_alifEllipsisObject_) // 14



class AlifSliceObject { // 26
public:
	ALIFOBJECT_HEAD{};
	AlifObject* start{}, * stop{}, * step{};
};

ALIFAPI_DATA(AlifTypeObject) _alifSliceType_; // 32

#define ALIFSLICE_CHECK(_op) ALIF_IS_TYPE((_op), &_alifSliceType_) // 35

ALIFAPI_FUNC(AlifObject*) alifSlice_new(AlifObject*, AlifObject*, AlifObject*); // 37



ALIFAPI_FUNC(AlifIntT) alifSlice_unpack(AlifObject*, AlifSizeT*, AlifSizeT*, AlifSizeT*); // 59
ALIFAPI_FUNC(AlifSizeT) alifSlice_adjustIndices(AlifSizeT, AlifSizeT*, AlifSizeT*, AlifSizeT);
