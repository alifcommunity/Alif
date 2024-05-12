#pragma once

#include "AlifCore_Memory.h" // instade AlifCore_AlifASTMem.h


/* ASDL: Abstract Syntax Description Language */

#define SEQ_HEAD AlifSizeT size{}; void** elements{} 

class Seq { 
public:
	SEQ_HEAD;
};

class GenericSeq { 
public:
	SEQ_HEAD;
	void* typedElements[1];
};

class IdentifierSeq {
public:
	SEQ_HEAD;
	AlifObject* typedElements[1];
};

class IntSeq {
public:
	SEQ_HEAD;
	int typedElements[1];
};

GenericSeq* alifNew_genericSeq(AlifUSizeT, AlifASTMem*); 
IdentifierSeq* alifNew_identifierSeq(AlifUSizeT, AlifASTMem*); 
IntSeq* alifNew_intSeq(AlifUSizeT, AlifASTMem*); 



#define GENERATE_SEQ_CONSTRUCTOR(typeName, name, type) typeName ## Seq* alifNew_ ## name ## Seq(AlifUSizeT _size, AlifASTMem* _astMem) { \
	typeName ## Seq* seq_ = nullptr; \
	size_t n_{}; \
	if (_size < 0 or (_size and (((size_t)_size - 1) > (SIZE_MAX / sizeof(void*))))) { \
			/* error */ \
			return nullptr; \
	} \
	n_ = (_size ? (sizeof(type*) * (_size - 1)) : 0); \
	if (n_ > SIZE_MAX - sizeof(typeName ## Seq)) { \
		/* error */ \
		return nullptr; \
	} \
	n_ += sizeof(typeName ## Seq); \
	seq_ = (typeName ## Seq*)alifASTMem_malloc(_astMem, n_); \
	if (!seq_) { \
		/* error */ \
		return nullptr; \
	} \
	memset(seq_, 0, n_); \
	seq_->size = _size; \
	seq_->elements = (void**)seq_->typedElements; \
	return seq_; \
}


#define SEQ_GETUNTYPED(s,i) ALIF_RVALUE((s)->elements[i]) 
#define SEQ_GET(s,i) ALIF_RVALUE((s)->typedElements[i]) 
#define SEQ_LEN(s) ALIF_RVALUE((s) == nullptr ? 0 : (s)->size) 

#define SEQ_SET(s, i, v) ALIF_RVALUE((s)->typedElements[(i)] = (v)) 

#define SEQ_SETUNTYPED(s, i, v) ALIF_RVALUE((s)->elements[(i)] = (v)) 

