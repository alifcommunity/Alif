#pragma once



AlifIntT alifFile_writeObject(AlifObject*, AlifObject*, AlifIntT); // 15
AlifIntT alifFile_writeString(const char*, AlifObject*); // 16












/* ------------------------------------------------------------------------------------------------ */




typedef AlifObject* (*AlifOpenCodeHookFunction)(AlifObject*, void*);
