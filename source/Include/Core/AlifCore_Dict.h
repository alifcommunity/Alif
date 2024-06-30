#pragma once

#define ALIFDICT_HASSPLITTABLE(d) ((d)->values != NULL)


class AlifDictKeyEntry {
public:
	size_t hash;
	AlifObject* key;
	AlifObject* value; 
} ;

class AlifDictUnicodeEntry{
public:
	AlifObject* key;   
	AlifObject* value; 
} ;

//bool alifDict_next(AlifObject* dict, int64_t* posPos, AlifObject** posKey, AlifObject** posValue, size_t* posHash); // issue here

#define DKIX_EMPTY (-1)
#define DKIX_DUMMY (-2)  /* Used internally */
#define DKIX_ERROR (-3)
#define DKIX_KEY_CHANGED (-4) /* Used internally */

enum DictKeysKind {
	Dict_Keys_General = 0,
	Dict_Keys_UStr = 1,
	Dict_Keys_Split = 2
} ;

class DictKeysObject {
public:
	AlifSizeT refCnt{};
	uint8_t log2Size{};
	uint8_t log2IndexBytes{};
	DictKeysKind kind{};
	uint32_t version{};
	AlifSizeT usable{};
	AlifSizeT nentries{};
	char indices[];
};

class DictValues {
public:
	uint8_t capacity;
	uint8_t size;
	uint8_t embedded;
	uint8_t valid;
	AlifObject* values[1];
};

#define DK_LOG_SIZE(dk)  ALIF_RVALUE((dk)->log2Size)
#if SIZEOF_VOID_P > 4
#define DK_SIZE(dk)      (((int64_t)1)<<DK_LOG_SIZE(dk))
#else
#define DK_SIZE(dk)      (1<<DK_LOG_SIZE(dk))
#endif

static inline void* dkSub_entries(AlifDictKeysObject* _dk) {
	int8_t* indices_ = (int8_t*)(_dk->indices);
	size_t index_ = (size_t)1 << _dk->log2IndexBytes;
	return (&indices_[index_]);
}

static inline AlifDictKeyEntry* dk_entries(AlifDictKeysObject* _dk) {
	return (AlifDictKeyEntry*)dkSub_entries(_dk);
}
static inline AlifDictUnicodeEntry* dk_uStr_entries(AlifDictKeysObject* _dk) {
	return (AlifDictUnicodeEntry*)dkSub_entries(_dk);
}

// in alifCore_Dict_State file
class AlifDictState {

	uint64_t global_version;
	uint32_t next_keys_version;
	//AlifDictWatchCallback watchers[DICT_MAX_WATCHERS];
};

#define DICT_MAX_WATCHERS 8
#define DICT_WATCHED_MUTATION_BITS 4



#define DICT_VERSION_INCREMENT (1 << (DICT_MAX_WATCHERS + DICT_WATCHED_MUTATION_BITS))
#define DICT_WATCHER_MASK ((1 << DICT_MAX_WATCHERS) - 1)
#define DICT_WATCHER_AND_MODIFICATION_MASK ((1 << (DICT_MAX_WATCHERS + DICT_WATCHED_MUTATION_BITS)) - 1)

#define DICT_NEXT_VERSION(INTERP) \
    ((INTERP)->dictState.globalVersion += DICT_VERSION_INCREMENT)



void alifDict_sendEvent(int ,
	AlifDictWatchEvent ,
	AlifDictObject* ,
	AlifObject* ,
	AlifObject* );

static inline uint64_t alifDict_notifyEvent(AlifInterpreter* _interp,
	AlifDictWatchEvent _event,
	AlifDictObject* _mp,
	AlifObject* _key,
	AlifObject* _value)
{
	int watcherBits = _mp->versionTag & DICT_WATCHER_MASK;
	if (watcherBits) {
		//RARE_EVENT_STAT_INC(watched_dict_modification);
		alifDict_sendEvent(watcherBits, _event, _mp, _key, _value);
	}
	return _mp->versionTag;
	//return DICT_NEXT_VERSION(_interp) | (_mp->versionTag & DICT_WATCHER_AND_MODIFICATION_MASK);
}

AlifObject* alifDict_fromItems(AlifObject* const*, AlifSizeT, AlifObject* const*, AlifSizeT, AlifSizeT);

static inline uint8_t* getInsertion_orderArray(AlifDictValues* _values)
{
	return (uint8_t*)&_values->values[_values->capacity];
}

static inline void alifDictValues_addToInsertionOrder(AlifDictValues* _values, int64_t _ix)
{
	int size_ = _values->size;
	uint8_t* array_ = getInsertion_orderArray(_values);
	array_[size_] = (uint8_t)_ix;
	_values->size = size_ + 1;
}

static inline AlifUSizeT shared_keys_usable_size(AlifDictKeysObject* keys) { // 297
	AlifSizeT dk_usable = keys->usable;
	AlifSizeT dk_nentries = keys->nentries;
	return dk_nentries + dk_usable;
}

static inline AlifUSizeT alifInline_valuesSize(AlifTypeObject* _tp) { // 312
	AlifDictKeysObject* keys = ((AlifHeapTypeObject*)_tp)->cachedKeys;
	size_t size = shared_keys_usable_size(keys);
	size_t prefix_size = ALIFSIZE_ROUND_UP(size, sizeof(AlifObject*));
	return prefix_size + (size + 1) * sizeof(AlifObject*);
}
