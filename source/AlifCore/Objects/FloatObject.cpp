#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_FreeList.h"

AlifObject* alifFloat_fromDouble(double _fVal) { // 123
	AlifFloatObject* op_ = ALIF_FREELIST_POP(AlifFloatObject, floats);
	if (op_ == nullptr) {
		op_ = (AlifFloatObject*)alifMem_objAlloc(sizeof(AlifFloatObject));
		if (!op_) {
			//return alifErr_noMemory();
			return nullptr; // temp
		}
		_alifObject_init((AlifObject*)op_, &_alifFloatType_);
	}
	op_->val = _fVal;
	return (AlifObject*)op_;
}

AlifTypeObject _alifFloatType_ = { // 1847
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "عدد_عشري",
	.basicSize = sizeof(AlifFloatObject),
	.itemSize = 0,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		_ALIF_TPFLAGS_MATCH_SELF,
};
