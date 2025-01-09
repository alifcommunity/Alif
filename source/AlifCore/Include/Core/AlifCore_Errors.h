#pragma once







extern AlifErrStackItem* _alifErr_getTopmostException(AlifThread*); // 14


extern AlifIntT _alifException_addNote(AlifObject*, AlifObject*); // 32


extern AlifObject* _alifErr_getRaisedException(AlifThread*); // 96


void _alifErr_clear(AlifThread*); // 117


void _alifErr_setString(AlifThread*, AlifObject*, const char*); // 123

AlifObject* _alifErr_format(AlifThread*, AlifObject*, const char*, ...); // 128
