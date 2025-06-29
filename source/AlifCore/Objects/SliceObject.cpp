#include "alif.h"

#include "AlifCore_FreeList.h"

AlifTypeObject _alifEllipsisType_ = { // 66
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "إضمار",
	.basicSize = 0,
	.itemSize = 0,

	.getAttro = alifObject_genericGetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT,
};

AlifObject _alifEllipsisObject_ = ALIFOBJECT_HEAD_INIT(&_alifEllipsisType_); // 107










static AlifSliceObject* _alifBuildSlice_consume2(AlifObject* _start,
	AlifObject* _stop, AlifObject* _step) { // 117
	AlifSliceObject* obj_ = ALIF_FREELIST_POP(AlifSliceObject, slices);
	if (obj_ == nullptr) {
		obj_ = ALIFOBJECT_GC_NEW(AlifSliceObject, &_alifSliceType_);
		if (obj_ == nullptr) {
			goto error;
		}
	}

	obj_->start = _start;
	obj_->stop = _stop;
	obj_->step = ALIF_NEWREF(_step);

	alifObject_gcTrack(obj_);
	return obj_;
error:
	ALIF_DECREF(_start);
	ALIF_DECREF(_stop);
	return nullptr;
}

AlifObject* alifSlice_new(AlifObject* _start, AlifObject* _stop, AlifObject* _step) { // 141
	if (_step == nullptr) {
		_step = ALIF_NONE;
	}
	if (_start == nullptr) {
		_start = ALIF_NONE;
	}
	if (_stop == nullptr) {
		_stop = ALIF_NONE;
	}
	return (AlifObject*)_alifBuildSlice_consume2(ALIF_NEWREF(_start),
		ALIF_NEWREF(_stop), _step);
}






AlifObject* _alifBuildSlice_consumeRefs(AlifObject* _start, AlifObject* _stop) { // 156
	return (AlifObject*)_alifBuildSlice_consume2(_start, _stop, ALIF_NONE);
}






































AlifTypeObject _alifSliceType_ = { // 658
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "قطع",
	.basicSize = sizeof(AlifSliceObject),
	//(Destructor)slice_dealloc,
	//(ReprFunc)slice_repr,
	//(HashFunc)slicehash,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//(TraverseProc)slice_traverse,
	//slice_richcompare,
	//slice_methods,
	//slice_members,
	//slice_new,
};
