#pragma once







extern AlifErrStackItem* _alifErr_getTopmostException(AlifThread*); // 14


extern AlifIntT _alifException_addNote(AlifObject*, AlifObject*); // 32




static inline AlifObject* _alifErr_occurred(AlifThread* _thread) { // 73
	if (_thread->currentException == nullptr) {
		return nullptr;
	}
	return (AlifObject*)ALIF_TYPE(_thread->currentException);
}


extern AlifObject* _alifErr_getRaisedException(AlifThread*); // 96


void _alifErr_clear(AlifThread*); // 117


void _alifErr_setString(AlifThread*, AlifObject*, const char*); // 123

AlifObject* _alifErr_format(AlifThread*, AlifObject*, const char*, ...); // 128
