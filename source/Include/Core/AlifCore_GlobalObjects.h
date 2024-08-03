#pragma once

#include "AlifCore_Context.h"
#include "AlifCore_GC.h"
#include "AlifCore_GlobalString.h"
#include "AlifCore_HashArrayMapTree.h"



#define ALIF_NSMALLPOSINTS           257
#define ALIF_NSMALLNEGINTS           5



#define ALIF_GLOBAL_OBJECT(NAME) \
    _alifDureRun_.staticObjects.NAME
#define ALIF_SINGLETON(NAME) \
    ALIF_GLOBAL_OBJECT(singletons.NAME)


class AlifStaticObjects {
public:
	class {
	public:
		AlifIntegerObject smallInts[ALIF_NSMALLNEGINTS + ALIF_NSMALLPOSINTS];

		AlifWBytesObject bytesEmpty;
		struct {
		public:
			AlifWBytesObject ob{};
			char eos{};
		} bytesCharacters[256];

		//AlifGlobalStrings strings{};

		ALIFGC_HEAD_UNUSED tupleEmptyGCNotUsed{};
		AlifTupleObject tupleEmpty{LLONG_MAX, &_alifTupleType_}; // فكرة خرافية ولكن يجب مراجعتها

		ALIFGC_HEAD_UNUSED hamtBitmapNodeEmptyGCNotUsed{};
		AlifHamtNodeBitmap hamtBitmapNodeEmpty{};
		AlifContextTokenMissing contextTokenMissing;
	} singletons;
};
