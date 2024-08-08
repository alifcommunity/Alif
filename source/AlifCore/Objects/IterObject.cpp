#include "alif.h"
#include "AlifCore_Memory.h"

AlifObject* alifNew_seqIter(AlifObject* _seq)
{
    SeqIterObject* it_;

    if (!alifSequence_check(_seq)) {
        return nullptr;
    }

	it_ = (SeqIterObject*)alifMem_objAlloc(sizeof(SeqIterObject));
    if (it_ == nullptr)
        return nullptr;
	it_->object->type_ = &typeSeqIter;
	it_->index = 0;
	it_->iterSeq = _seq;
    return (AlifObject*)it_;
}

AlifTypeObject typeSeqIter = {
    0,
    0,
    0,
    L"iterator",                                 
    sizeof(SeqIterObject),                      
    0,                                         
    0,                 
    0,                                         
    0,                                        
    0,                                        
    0,                                         
    0,                                        
    0,                                          
    0,                                          
    0,                                        
    0,                                          
    0,       
    0,
    0,                                          
    0,                   
    0,                                         
    0,                                         
    0,   
    0,                                         
    0,              
    0,                                         
    0,                                         
    0,                                        
    0,                          
    0,                              
    0,                           
    0,                                       
};
