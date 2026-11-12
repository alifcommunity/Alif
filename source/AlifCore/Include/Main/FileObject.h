#pragma once


ALIFAPI_FUNC(AlifObject*) alifFile_getLine(AlifObject*, AlifIntT); // 14
ALIFAPI_FUNC(AlifIntT) alifFile_writeObject(AlifObject*, AlifObject*, AlifIntT); // 15
ALIFAPI_FUNC(AlifIntT) alifFile_writeString(const char*, AlifObject*); // 16
ALIFAPI_FUNC(AlifIntT) alifObject_asFileDescriptor(AlifObject*); // 17







ALIFAPI_DATA(AlifIntT) _alifUTF8Mode_;



/* ------------------------------------------------------------------------------------------------ */


ALIFAPI_FUNC(AlifObject*) alifFile_newStdPrinter(int);
ALIFAPI_DATA(AlifTypeObject) _alifStdPrinterType_;

typedef AlifObject* (*AlifOpenCodeHookFunction)(AlifObject*, void*);


ALIFAPI_FUNC(AlifObject*) alifFile_openCodeObject(AlifObject*);
