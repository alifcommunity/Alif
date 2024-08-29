#pragma once

#include "AlifCore_Object.h"

class AlifDictKeyEntry { // 74
public:
	AlifUSizeT hash{};
	AlifObject* key{};
	AlifObject* value{};
};

class AlifDictUStrEntry { // 81
public:
	AlifObject* key{};
	AlifObject* value{};
};

// 126
#define DKIX_EMPTY (-1)
#define DKIX_DUMMY (-2)
#define DKIX_ERROR (-3)
#define DKIX_KEY_CHANGED (-4)


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

class DictValues { // 194
public:
	uint8_t capacity{};
	uint8_t size{};
	uint8_t embedded{};
	uint8_t valid{};
	AlifObject* values[1]{};
};

#define DK_LOG_SIZE(_dk)  ALIF_RVALUE((_dk)->dkLog2Size) // 202

static inline void* _dk_entries(AlifDictKeysObject* _dk) { // 209
	int8_t* indices = (int8_t*)(_dk->dkIndices);
	AlifUSizeT index = (AlifUSizeT)1 << _dk->dkLog2IndexBytes;
	return (&indices[index]);
}

static inline AlifDictKeyEntry* dk_entries(AlifDictKeysObject* _dk) { // 215
	return (AlifDictKeyEntry*)_dk_entries(_dk);
}
static inline AlifDictUStrEntry* dk_UStrEntries(AlifDictKeysObject* _dk) { // 219
	return (AlifDictUStrEntry*)_dk_entries(_dk);
}

#define DK_IS_USTR(_dk) ((_dk)->dkKind != DictKeysKind_::Dict_Kyes_General) // 224

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


static inline uint64_t dict_nextVersion(AlifInterpreter* _interp) { // 235
	AlifThread* tstate = alifThread_get();
	uint64_t curProgress = (tstate->dictGlobalVersion &
		(THREAD_LOCAL_DICT_VERSION_BATCH - 1));
	if (curProgress == 0) {
		uint64_t next = alifAtomic_addInt64(&_interp->dictState.globalVersion,
			THREAD_LOCAL_DICT_VERSION_BATCH);
		tstate->dictGlobalVersion = next;
	}
	return tstate->dictGlobalVersion += DICT_VERSION_INCREMENT;
}
#define DICT_NEXT_VERSION(_interp) dict_nextVersion(_interp)
