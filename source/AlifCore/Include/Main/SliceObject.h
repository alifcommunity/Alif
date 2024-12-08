#pragma once


extern AlifObject _alifEllipsisObject_; // 9


#define ALIF_ELLIPSIS (&_alifEllipsisObject_) // 14



class AlifSliceObject { // 26
public:
	ALIFOBJECT_HEAD{};
	AlifObject* start{}, * stop{}, * step{};
};

extern AlifTypeObject _alifSliceType_; // 32

#define ALIFSLICE_CHECK(_op) ALIF_IS_TYPE((_op), &_alifSliceType_) // 35
