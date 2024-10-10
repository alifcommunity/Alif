#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_FreeList.h"

AlifObject* alifFloat_fromDouble(double _fVal) { // 123
	AlifFloatObject* op_ = ALIF_FREELIST_POP(AlifFloatObject, floats);
	if (op_ == nullptr) {
		op_ = (AlifFloatObject*)alifMem_objAlloc(sizeof(AlifFloatObject));
		if (!op_) {
			//return alifErr_noMemory();
		}
		_alifObject_init((AlifObject*)op_, &_alifFloatType_);
	}
	op_->fVal = _fVal;
	return (AlifObject*)op_;
}

AlifTypeObject _alifFloatType_ = {
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "عدد عشري",
	.basicSize = sizeof(AlifFloatObject),
};
