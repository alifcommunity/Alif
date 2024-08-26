#pragma once

#include "AlifCore_Object.h"

 enum DictKeysKind_ { // 131
	Dict_Kyes_General = 0,
	Dict_Kyes_UStr = 1,
	Dict_Kyes_Split = 2
};

class DictKeysObject { // 138
public:
	AlifSizeT dkRefCnt{};
	uint8_t dkLog2Size{};
	uint8_t dkLog2IndexBytes{};
	uint8_t dkKind{};
	AlifMutex dkMutex{};
	uint32_t dkVersion{};
	AlifSizeT dkUsable{};
	AlifSizeT dkNentries{};
	char dkIndices[];
};


static inline AlifUSizeT sharedKeys_usableSize(AlifDictKeysObject* _keys) { // 305
	AlifSizeT dkUsable = alifAtomic_loadSizeAcquire(&_keys->dkUsable);
	AlifSizeT dkNentries = alifAtomic_loadSizeAcquire(&_keys->dkNentries);
	return dkNentries + dkUsable;
}

static inline AlifUSizeT alifInline_valuesSize(AlifTypeObject* _tp) { // 320
	AlifDictKeysObject* keys = ((AlifHeapTypeObject*)_tp)->cachedKeys;
	AlifUSizeT size = sharedKeys_usableSize(keys);
	AlifUSizeT prefixSize = ALIF_SIZE_ROUND_UP(size, sizeof(AlifObject*));
	return prefixSize + (size + 1) * sizeof(AlifObject*);
}
