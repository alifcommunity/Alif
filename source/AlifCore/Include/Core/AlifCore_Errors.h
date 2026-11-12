#pragma once







extern AlifErrStackItem* _alifErr_getTopMostException(AlifThread*); // 14


ALIFAPI_FUNC(void) _alifErr_setKeyError(AlifObject*); // 20

extern AlifIntT _alifException_addNote(AlifObject*, AlifObject*); // 32


ALIFAPI_FUNC(AlifObject*) _alifErr_programDecodedTextObject(AlifObject*, AlifIntT, const char*); // 41


extern void ALIF_NO_RETURN _alif_fatalErrorFormat(const char*, const char*, ...); // 53

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

ALIFAPI_FUNC(AlifIntT) _alifErr_exceptionMatches(AlifThread*, AlifObject*); // 98

extern void _alifErr_setRaisedException(AlifThread*, AlifObject*); // 102


extern void _alifErr_setObject(AlifThread*, AlifObject*, AlifObject*); // 110

extern void _alifErr_chainStackItem(void); // 115

ALIFAPI_FUNC(void) _alifErr_clear(AlifThread*); // 117


ALIFAPI_FUNC(void) _alifErr_setString(AlifThread*, AlifObject*, const char*); // 128

extern void _alifErr_raiseSyntaxError(AlifObject*, AlifObject*,
	AlifIntT, AlifIntT, AlifIntT, AlifIntT); // 152

AlifObject* _alifErr_format(AlifThread*, AlifObject*, const char*, ...); // 128



extern AlifObject* _alifExc_createExceptionGroup(const char*, AlifObject*); // 151



extern AlifObject* _alif_calculateSuggestions(AlifObject*, AlifObject*); // 162
