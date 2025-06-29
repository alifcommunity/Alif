#pragma once

#include "AlifCore_GC.h"
#include "AlifCore_GlobalString.h"
#include "AlifCore_HashTable.h"
#include "AlifCore_TypeObject.h"





#define ALIF_NSMALLPOSINTS           257 // 20
#define ALIF_NSMALLNEGINTS           5 // 21



// 27
#define ALIF_GLOBAL_OBJECT(_name) \
    _alifDureRun_.staticObjects._name
#define ALIF_SINGLETON(_name) \
    ALIF_GLOBAL_OBJECT(singletons._name)

class AlifCachedObjects { // 32
public:
	AlifHashTableT* internedStrings{};
};












class AlifStaticObjects { // 37
public:
	class {
	public:
		AlifLongObject smallInts[ALIF_NSMALLNEGINTS + ALIF_NSMALLPOSINTS]{};

		AlifBytesObject bytesEmpty{};
		class {
		public:
			AlifBytesObject ob{};
			char eos{};
		} bytesCharacters[256];

		AlifGlobalStrings strings{};

		AlifTupleObject tupleEmpty{};
	} singletons;
};


// 63
#define ALIF_INTERP_CACHED_OBJECT(_interp, _name) \
    (_interp)->cachedObjects._name


class AlifInterpCachedObjects { // 66
public:
	AlifObject* internedStrings{};

	AlifObject* typeSlotsPname{};
	AlifTypeSlotDef* typeSlotsPtrs[MAX_EQUIV]{};
};
