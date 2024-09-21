#pragma once


#include "AlifCore_FileUtils.h"
#include "AlifCore_GlobalObjects.h"












extern void alifUStr_fastCopyCharacters(AlifObject*, AlifSizeT,
	AlifObject*, AlifSizeT, AlifSizeT); // 55



extern AlifIntT alifUStr_initGlobalObjects(AlifInterpreter*); // 272

void alifUStr_internMortal(AlifInterpreter*, AlifObject**); // 284
