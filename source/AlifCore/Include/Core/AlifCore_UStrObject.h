#pragma once


#include "AlifCore_FileUtils.h"
#include "AlifCore_GlobalObjects.h"




extern AlifIntT _alifUStr_isXIDStart(AlifUCS4); // 18
extern AlifIntT _alifUStr_isXIDContinue(AlifUCS4); // 19



AlifObject* _alifUStr_copy(AlifObject*); // 40



extern void alifUStr_fastCopyCharacters(AlifObject*, AlifSizeT,
	AlifObject*, AlifSizeT, AlifSizeT); // 55



extern AlifObject* alifUStr_fromASCII(const char*, AlifSizeT); // 65




AlifObject* alifUStr_decodeUStrEscapeInternal(const char*, AlifSizeT, const char*, AlifSizeT*, const char**); // 144


AlifObject* alifUStr_joinArray(AlifObject*, AlifObject* const*, AlifSizeT); // 206


AlifIntT alifUStr_equalToASCIIString(AlifObject* , const char* ); // 224


extern AlifIntT alifUStr_eq(AlifObject*, AlifObject*); // 257

AlifIntT _alifUStr_equal(AlifObject*, AlifObject*); // 261

AlifSizeT _alifUStr_scanIdentifier(AlifObject*); // 267

extern AlifIntT alifUStr_initGlobalObjects(AlifInterpreter*); // 272

void alifUStr_internMortal(AlifInterpreter*, AlifObject**); // 284
void alifUStr_internImmortal(AlifInterpreter*, AlifObject**); // 285
