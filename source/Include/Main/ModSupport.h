#pragma once


#define ALIF_CLEANUP_SUPPORTED 0x20000

AlifIntT alifModule_addFunctions(AlifObject*, AlifMethodDef*);


int alifArg_parseTuple(AlifObject* , const wchar_t* , ...);

