#pragma once


#include "AlifCore_FileUtils.h"
#include "AlifCore_GlobalObjects.h"












extern void alifUStr_fastCopyCharacters(AlifObject*, AlifSizeT,
	AlifObject*, AlifSizeT, AlifSizeT); // 55



extern AlifObject* alifUStr_fromASCII(const char*, AlifSizeT); // 65




AlifObject* alifUStr_decodeUStrEscapeInternal(const char*, AlifSizeT, const char*, AlifSizeT*, const char**); // 144



extern AlifIntT alifUStr_initGlobalObjects(AlifInterpreter*); // 272

void alifUStr_internMortal(AlifInterpreter*, AlifObject**); // 284
void alifUStr_internImmortal(AlifInterpreter*, AlifObject**); // 285
