#pragma once

#include "AlifCore_Object.h"

 enum DictKeysKind {
	DICT_KEYS_GENERAL = 0,
	DICT_KEYS_UNICODE = 1,
	DICT_KEYS_SPLIT = 2
} ;

class DictKeysObject {
public:
	AlifSizeT dkRefCnt;

	uint8_t dkLog2Size;

	uint8_t dkLog2IndexBytes;

	uint8_t dkKind;

#ifdef ALIF_GIL_DISABLED
	AlifMutex dkMutex;
#endif

	uint32_t dkVersion;

	AlifSizeT dkUsable;

	AlifSizeT dkNentries;
	char dkIndices[];  
};


static inline size_t sharedKeys_usableSize(AlifDictKeysObject* _keys) // 305
{
	AlifSizeT dkUsable = (_keys->dkUsable);
	AlifSizeT dkNentries = (_keys->dkNentries);
	return dkNentries + dkUsable;
}

static inline size_t alifInlineValuesSize(AlifTypeObject* _tp) // 320
{
	AlifDictKeysObject* keys = ((AlifHeapTypeObject*)_tp)->cachedKeys;
	size_t size = sharedKeys_usableSize(keys);
	size_t prefixSize = ALIF_SIZE_ROUND_UP(size, sizeof(AlifObject*));
	return prefixSize + (size + 1) * sizeof(AlifObject*);
}
