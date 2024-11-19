#include "alif.h"

#include "AlifCore_Compile.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_OpcodeMetadata.h"


























AlifObject* _alifInstructionSequence_new() { // 197
	AlifInstructionSequence* seq = instSeq_create();
	if (seq == nullptr) {
		return nullptr;
	}
	return (AlifObject*)seq;
}
