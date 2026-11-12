#include "alif.h"

#include "AlifCore_Cell.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"


#define ALIFCELL_CAST(_op) ALIF_CAST(AlifCellObject*, _op)

AlifObject* alifCell_new(AlifObject* _obj) { // 10
	AlifCellObject* op{};

	op = (AlifCellObject*)ALIFOBJECT_GC_NEW(AlifCellObject, &_alifCellType_);
	if (op == nullptr)
		return nullptr;
	op->ref = ALIF_XNEWREF(_obj);

	ALIFOBJECT_GC_TRACK(op);
	return (AlifObject*)op;
}


static AlifObject* cell_new(AlifTypeObject* _type,
	AlifObject* _args, AlifObject* _kwargs) { // 36
	AlifObject* returnValue{};
	AlifObject* obj{};

	if (!_ALIFARG_NOKEYWORDS("خلية", _kwargs)) {
		goto exit;
	}
	/* min = 0: we allow the cell to be empty */
	if (!alifArg_unpackTuple(_args, "خلية", 0, 1, &obj)) {
		goto exit;
	}
	returnValue = alifCell_new(obj);

exit:
	return returnValue;
}


AlifIntT alifCell_set(AlifObject* _op, AlifObject* _value) { // 63
	if (!ALIFCELL_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	alifCell_setTakeRef((AlifCellObject*)_op, ALIF_XNEWREF(_value));
	return 0;
}




static AlifIntT cell_traverse(AlifObject* _self,
	VisitProc visit, void* arg) { // 118
	AlifCellObject* op = ALIFCELL_CAST(_self);
	ALIF_VISIT(op->ref);
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
	.traverse = cell_traverse,
	.new_ = cell_new
};
