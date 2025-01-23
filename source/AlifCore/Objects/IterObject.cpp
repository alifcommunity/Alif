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







static AlifObject* iter_iterNext(AlifObject* iterator) { // 48
	SeqIterObject* it{};
	AlifObject* seq{};
	AlifObject* result{};

	it = (SeqIterObject*)iterator;
	seq = it->seq;
	if (seq == nullptr)
		return nullptr;
	if (it->index == ALIF_SIZET_MAX) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"iter index too large");
		return nullptr;
	}

	result = alifSequence_getItem(seq, it->index);
	if (result != nullptr) {
		it->index++;
		return result;
	}
	//if (alifErr_exceptionMatches(_alifExcIndexError_) or
	//	alifErr_exceptionMatches(_alifExcStopIteration_))
	//{
	//	alifErr_clear();
	//	it->seq = nullptr;
	//	ALIF_DECREF(seq);
	//}
	return nullptr;
}









AlifTypeObject _alifSeqIterType_ = { // 144
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مكرر",
	.basicSize = sizeof(SeqIterObject),                  
	.itemSize = 0,                                       
	.getAttro = alifObject_genericGetAttr,               
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,

	.iterNext = iter_iterNext,
};
