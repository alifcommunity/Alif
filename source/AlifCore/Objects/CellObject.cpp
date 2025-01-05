#include "alif.h"

#include "AlifCore_Cell.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"



AlifObject* alifCell_new(AlifObject* _obj) { // 8
	AlifCellObject* op{};

	op = (AlifCellObject*)ALIFOBJECT_GC_NEW(AlifCellObject, &_alifCellType_);
	if (op == nullptr)
		return nullptr;
	op->ref = ALIF_XNEWREF(_obj);

	ALIFOBJECT_GC_TRACK(op);
	return (AlifObject*)op;
}



AlifIntT alifCell_set(AlifObject* _op, AlifObject* _value) { // 63
	if (!ALIFCELL_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	alifCell_setTakeRef((AlifCellObject*)_op, ALIF_XNEWREF(_value));
	return 0;
}






AlifTypeObject _alifCellType_ = { // 151
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "خلية",
	.basicSize = sizeof(AlifCellObject),
	//.dealloc = (Destructor)cell_dealloc,
	//.repr = (ReprFunc)cell_repr,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};
