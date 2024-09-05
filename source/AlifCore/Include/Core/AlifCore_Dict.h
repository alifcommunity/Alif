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


extern void alifDictKeys_decRef(AlifDictKeysObject*); // 96



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
	AlifSizeT refCnt{};
	uint8_t log2Size{};
	uint8_t log2IndexBytes{};
	uint8_t kind{};
#ifdef ALIF_GIL_DISABLED
	AlifMutex mutex{};
#endif
	uint32_t version{};
	AlifSizeT usable{};
	AlifSizeT nentries{};
	char indices[];
};

class DictValues { // 194
public:
	uint8_t capacity{};
	uint8_t size{};
	uint8_t embedded{};
	uint8_t valid{};
	AlifObject* values[1]{};
};

#define DK_LOG_SIZE(_dk)  ALIF_RVALUE((_dk)->log2Size) // 202

static inline void* _dk_entries(AlifDictKeysObject* _dk) { // 209
	int8_t* indices = (int8_t*)(_dk->indices);
	AlifUSizeT index = (AlifUSizeT)1 << _dk->log2IndexBytes;
	return (&indices[index]);
}

static inline AlifDictKeyEntry* dk_entries(AlifDictKeysObject* _dk) { // 215
	return (AlifDictKeyEntry*)_dk_entries(_dk);
}
static inline AlifDictUStrEntry* dk_UStrEntries(AlifDictKeysObject* _dk) { // 219
	return (AlifDictUStrEntry*)_dk_entries(_dk);
}

#define DK_IS_USTR(_dk) ((_dk)->kind != DictKeysKind_::Dict_Kyes_General) // 224

// 226
#define DICT_VERSION_INCREMENT (1 << (DICT_MAX_WATCHERS + DICT_WATCHED_MUTATION_BITS))
#define DICT_WATCHER_MASK ((1 << DICT_MAX_WATCHERS) - 1)
#define DICT_WATCHER_AND_MODIFICATION_MASK ((1 << (DICT_MAX_WATCHERS + DICT_WATCHED_MUTATION_BITS)) - 1)


#ifdef ALIF_GIL_DISABLED

#define THREAD_LOCAL_DICT_VERSION_COUNT 256
#define THREAD_LOCAL_DICT_VERSION_BATCH THREAD_LOCAL_DICT_VERSION_COUNT * DICT_VERSION_INCREMENT


static inline uint64_t dict_nextVersion(AlifInterpreter* _interp) { // 235
	AlifThread* tstate = alifThread_get();
	uint64_t curProgress = (tstate->dictGlobalVersion &
		(THREAD_LOCAL_DICT_VERSION_BATCH - 1));
	if (curProgress == 0) {
		uint64_t next = alifAtomic_addUint64(&_interp->dictState.globalVersion,
			THREAD_LOCAL_DICT_VERSION_BATCH);
		tstate->dictGlobalVersion = next;
	}
	return tstate->dictGlobalVersion += DICT_VERSION_INCREMENT;
}
#define DICT_NEXT_VERSION(_interp) dict_nextVersion(_interp)

#else
#define DICT_NEXT_VERSION(_interp) \
    ((_interp)->dictState.globalVersion += DICT_VERSION_INCREMENT)
#endif

static inline AlifUSizeT sharedKeys_usableSize(AlifDictKeysObject* _keys) { // 305
	AlifSizeT dkUsable = alifAtomic_loadSizeAcquire(&_keys->usable);
	AlifSizeT dkNentries = alifAtomic_loadSizeAcquire(&_keys->nentries);
	return dkNentries + dkUsable;
}

static inline AlifUSizeT alifInline_valuesSize(AlifTypeObject* _tp) { // 320
	AlifDictKeysObject* keys = ((AlifHeapTypeObject*)_tp)->cachedKeys;
	AlifUSizeT size = sharedKeys_usableSize(keys);
	AlifUSizeT prefixSize = ALIF_SIZE_ROUND_UP(size, sizeof(AlifObject*));
	return prefixSize + (size + 1) * sizeof(AlifObject*);
}


