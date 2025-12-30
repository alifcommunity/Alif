#pragma once







extern AlifErrStackItem* _alifErr_getTopmostException(AlifThread*); // 14


extern AlifIntT _alifException_addNote(AlifObject*, AlifObject*); // 32


AlifObject* _alifErr_programDecodedTextObject(AlifObject*, AlifIntT, const char*); // 41


static inline AlifObject* _alifErr_occurred(AlifThread* _thread) { // 73
	if (_thread->currentException == nullptr) {
		return nullptr;
	}
	return (AlifObject*)ALIF_TYPE(_thread->currentException);
}

static inline void _alifErr_clearExcState(AlifErrStackItem* _excState) { // 82
	ALIF_CLEAR(_excState->excValue);
}


extern AlifObject* _alifErr_getRaisedException(AlifThread*); // 96

AlifIntT _alifErr_exceptionMatches(AlifThread*, AlifObject*); // 98

extern void _alifErr_setRaisedException(AlifThread*, AlifObject*); // 102

void _alifErr_clear(AlifThread*); // 117


void _alifErr_setString(AlifThread*, AlifObject*, const char*); // 123

extern void _alifErr_raiseSyntaxError(AlifObject*, AlifObject*,
	AlifIntT, AlifIntT, AlifIntT, AlifIntT); // 152

AlifObject* _alifErr_format(AlifThread*, AlifObject*, const char*, ...); // 128



extern AlifObject* _alifExc_createExceptionGroup(const char*, AlifObject*); // 151
