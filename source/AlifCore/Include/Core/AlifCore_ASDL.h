#pragma once
/* ASDL: Abstract Syntax Description Language */

#include "AlifCore_Memory.h"

// 13
typedef AlifObject* Identifier;
typedef AlifObject* String;
typedef AlifObject* Object;
typedef AlifObject* Constant;

#define ASDL_SEQ_HEAD AlifSizeT size{}; void** elements{} // 24

class ASDLSeq { // 28
public:
	ASDL_SEQ_HEAD;
};

class ASDLGenericSeq { 
public:
	ASDL_SEQ_HEAD;
	void* typedElements[1]{};
};

class ASDLIdentifierSeq {
public:
	ASDL_SEQ_HEAD;
	AlifObject* typedElements[1]{};
};

class ASDLIntSeq {
public:
	ASDL_SEQ_HEAD;
	AlifIntT typedElements[1]{};
};

ASDLGenericSeq* alifNew_genericSeq(AlifSizeT, AlifASTMem*);  // 47
ASDLIdentifierSeq* alifNew_identifierSeq(AlifSizeT, AlifASTMem*); 
ASDLIntSeq* alifNew_intSeq(AlifSizeT, AlifASTMem*); 



#define GENERATE_SEQ_CONSTRUCTOR(typeName, name, type) typeName ## Seq* alifNew_ ## name ## Seq(AlifUSizeT _size, AlifASTMem* _astMem) { \
	typeName ## Seq* seq_ = nullptr; \
	AlifUSizeT n_{}; \
	if (_size < 0 or (_size and (((AlifUSizeT)_size - 1) > (SIZE_MAX / sizeof(void*))))) { \
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

 // 81
#define ASDL_SEQ_GETUNTYPED(_s, _i) ALIF_RVALUE((_s)->elements[_i]) 
#define ASDL_SEQ_GET(_s, _i) ALIF_RVALUE((_s)->typedElements[_i]) 
#define ASDL_SEQ_LEN(_s) ALIF_RVALUE((_s) == nullptr ? 0 : (_s)->size) 

#define ASDL_SEQ_SET(_s, _i, _v) ALIF_RVALUE((_s)->typedElements[(_i)] = (_v)) 

#define ASDL_SEQ_SETUNTYPED(_s, _i, _v) ALIF_RVALUE((_s)->elements[(_i)] = (_v)) 

