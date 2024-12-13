#pragma once


#include "AlifCore_FileUtils.h"
#include "AlifCore_GlobalObjects.h"








AlifObject* _alifUStr_copy(AlifObject*); // 40



extern void alifUStr_fastCopyCharacters(AlifObject*, AlifSizeT,
	AlifObject*, AlifSizeT, AlifSizeT); // 55



extern AlifObject* alifUStr_fromASCII(const char*, AlifSizeT); // 65




AlifObject* alifUStr_decodeUStrEscapeInternal(const char*, AlifSizeT, const char*, AlifSizeT*, const char**); // 144


AlifObject* alifUStr_joinArray(AlifObject*, AlifObject* const*, AlifSizeT); // 206


AlifIntT alifUStr_equalToASCIIString(AlifObject* , const char* ); // 224


extern AlifIntT alifUStr_eq(AlifObject*, AlifObject*); // 257

extern AlifIntT alifUStr_initGlobalObjects(AlifInterpreter*); // 272

void alifUStr_internMortal(AlifInterpreter*, AlifObject**); // 284
void alifUStr_internImmortal(AlifInterpreter*, AlifObject**); // 285
