#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"






class SeqIterObject {
public:
	ALIFOBJECT_HEAD;
	AlifSizeT index{};
	AlifObject* seq{}; /* Set to nullptr when iterator is exhausted */
};


AlifObject* alifSeqIter_new(AlifObject* _seq) { // 15
	SeqIterObject* it{};

	if (!alifSequence_check(_seq)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	it = ALIFOBJECT_GC_NEW(SeqIterObject, &_alifSeqIterType_);
	if (it == nullptr) return nullptr;
	it->index = 0;
	it->seq = ALIF_NEWREF(_seq);
	ALIFOBJECT_GC_TRACK(it);
	return (AlifObject*)it;
}

















AlifTypeObject _alifSeqIterType_ = { // 144
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مكرر",
	.basicSize = sizeof(SeqIterObject),                  
	.itemSize = 0,                                       
	.getAttro = alifObject_genericGetAttr,               
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};
