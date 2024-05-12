#include "alif.h"
#include "AlifCore_Memory.h"

AlifObject* alifNew_seqIter(AlifObject* seq)
{
    SeqIterObject* it;

    if (!alifSequence_check(seq)) {
        return nullptr;
    }

    it = (SeqIterObject*)alifMem_objAlloc(sizeof(SeqIterObject));
    if (it == nullptr)
        return nullptr;
    it->object->type_ = &typeSeqIter;
    it->index = 0;
    it->iterSeq = seq;
    return (AlifObject*)it;
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