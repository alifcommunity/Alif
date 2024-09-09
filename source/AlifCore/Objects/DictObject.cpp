
#define ALIFDICT_LOG_MINSIZE 3 // 115
#define ALIFDICT_MINSIZE 8 // 116

#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_Dict.h"
#include "AlifCore_GC.h"
#include "AlifCore_Object.h"
#include "StringLib/Equal.h"

#define IS_DICT_SHARED(_mp) ALIFOBJECT_GC_IS_SHARED(_mp) // 166
#define SET_DICT_SHARED(_mp) ALIFOBJECT_GC_SET_SHARED(_mp) // 167
#define LOAD_INDEX(_keys, _size, _idx) alifAtomic_loadInt##_size##Relaxed(&((const int##_size##_t*)_keys->indices)[_idx]); // 168
#define STORE_INDEX(_keys, _size, _idx, _value) alifAtomic_storeInt##_size##Relaxed(&((int##_size##_t*)_keys->indices)[_idx], (int##_size##_t)_value); // 169


// 173
#define LOCK_KEYS_IF_SPLIT(_keys, _kind) \
        if (_kind == Dict_Kyes_Split) { \
            LOCK_KEYS(_keys);           \
        }

// 178
#define UNLOCK_KEYS_IF_SPLIT(_keys, _kind) \
        if (_kind == Dict_Kyes_Split) {   \
            UNLOCK_KEYS(_keys);           \
        }


static inline void set_keys(AlifDictObject* _mp, AlifDictKeysObject* _keys) { // 191
	alifAtomic_storePtrRelease(&_mp->keys, _keys);
}

static inline void set_values(AlifDictObject* _mp, AlifDictValues* _values) { // 198
	alifAtomic_storePtrRelease(&_mp->values, _values);
}

 // 204
#define LOCK_KEYS(_keys) alifMutex_lockFlags(&_keys->mutex, AlifLockFlags_::Alif_Lock_Dont_Detach)
#define UNLOCK_KEYS(_keys) ALIFMUTEX_UNLOCK(&_keys->mutex)

#define STORE_SHARED_KEY(_key, _value) alifAtomic_storePtrRelease(&_key, _value) // 209
#define INCREF_KEYS(_dk)  alifAtomic_addSize(&_dk->refCnt, 1) // 211
#define DECREF_KEYS(_dk)  alifAtomic_addSize(&_dk->refCnt, -1)// 213

#define INCREF_KEYS_FT(_dk) dictKeys_incRef(_dk) // 216
#define DECREF_KEYS_FT(_dk, _shared) dictKeys_decRef(alifInterpreter_get(), _dk, _shared) // 217


static inline void splitKeys_entryAdded(AlifDictKeysObject* _keys) { // 219
	alifAtomic_storeSizeRelaxed(&_keys->nentries, _keys->nentries + 1);
	alifAtomic_storeSizeRelaxed(&_keys->usable, _keys->usable - 1);
}



static inline uint8_t* getInsertion_orderArray(AlifDictValues* _values) { // 287
	return (uint8_t*)&_values->values[_values->capacity];
}

#define STORE_VALUE(_ep, _value) alifAtomic_storePtrRelease(_ep->value, _value) // 279
#define STORE_KEYS_USABLE(_keys, _usable) alifAtomic_storeSizeRelaxed(&_keys->usable, _usable)
#define STORE_KEYS_NENTRIES(_keys, _nentries) alifAtomic_storeSizeRelaxed(&_keys->nentries, _nentries)
#define STORE_SPLIT_VALUE(_mp, _idx, _value) alifAtomic_storePtrRelease(&_mp->values->values[_idx], _value) // 280
#define STORE_USED(_mp, _used) alifAtomic_storeSizeRelaxed(&_mp->used, _used) // 284


#define PERTURB_SHIFT 5 // 286

static AlifIntT dictResize(AlifInterpreter* , AlifDictObject* ,
	uint8_t , AlifIntT ); // 380

static inline AlifUSizeT uStr_getHash(AlifObject* _o) { // 399
	return alifAtomic_loadSizeRelaxed(&ALIFASCIIOBJECT_CAST(_o)->hash);
}

#define DK_MASK(_dk) (DK_SIZE(_dk)-1) // 419

static void free_keysObject(AlifDictKeysObject*, bool); // 421


static inline void dictKeys_incRef(AlifDictKeysObject* _dk) { // 430
	if (alifAtomic_loadSizeRelaxed(&_dk->refCnt) == ALIF_IMMORTAL_REFCNT) {
		return;
	}
	INCREF_KEYS(_dk);
}

static inline void dictKeys_decRef(AlifInterpreter* _interp,
	AlifDictKeysObject* _dk, bool _useqsbr) { // 442
	if (alifAtomic_loadSizeRelaxed(&_dk->refCnt) == ALIF_IMMORTAL_REFCNT) {
		return;
	}
	if (DECREF_KEYS(_dk) == 1) {
		if (DK_IS_USTR(_dk)) {
			AlifDictUStrEntry* entries = dk_UStrEntries(_dk);
			AlifSizeT i{}, n{};
			for (i = 0, n = _dk->nentries; i < n; i++) {
				ALIF_XDECREF(entries[i].key);
				ALIF_XDECREF(entries[i].value);
			}
		}
		else {
			AlifDictKeyEntry* entries = dk_entries(_dk);
			AlifSizeT i, n;
			for (i = 0, n = _dk->nentries; i < n; i++) {
				ALIF_XDECREF(entries[i].key);
				ALIF_XDECREF(entries[i].value);
			}
		}
		free_keysObject(_dk, _useqsbr);
	}
}

static inline AlifSizeT dictKeys_getIndex(const AlifDictKeysObject* _keys, AlifSizeT _i) { // 475
	AlifIntT log2Size = DK_LOG_SIZE(_keys);
	AlifSizeT ix_;

	if (log2Size < 8) {
		ix_ = LOAD_INDEX(_keys, 8, _i);
	}
	else if (log2Size < 16) {
		ix_ = LOAD_INDEX(_keys, 16, _i);
	}
#if SIZEOF_VOID_P > 4
	else if (log2Size >= 32) {
		ix_ = LOAD_INDEX(_keys, 64, _i);
	}
#endif
	else {
		ix_ = LOAD_INDEX(_keys, 32, _i);
	}
	return ix_;
}

static inline void dictKeys_setIndex(AlifDictKeysObject* _keys, AlifSizeT _i, AlifSizeT _ix) { // 500
	AlifIntT log2Size = DK_LOG_SIZE(_keys);

	if (log2Size < 8) {
		STORE_INDEX(_keys, 8, _i, _ix);
	}
	else if (log2Size < 16) {
		STORE_INDEX(_keys, 16, _i, _ix);
	}
#if SIZEOF_VOID_P > 4
	else if (log2Size >= 32) {
		STORE_INDEX(_keys, 64, _i, _ix);
	}
#endif
	else {
		STORE_INDEX(_keys, 32, _i, _ix);
	}
}

#define USABLE_FRACTION(_n) (((_n) << 1)/3) // 538

static inline uint8_t calculate_log2KeySize(AlifSizeT _minSize) { // 542
#if SIZEOF_LONG == SIZEOF_SIZE_T
	_minSize = (_minSize | ALIFDICT_MINSIZE) - 1;
	return alifBit_length(_minSize | (ALIFDICT_MINSIZE - 1));
#elif defined(_MSC_VER)
	// On 64bit Windows, sizeof(long) == 4.
	_minSize = (_minSize | ALIFDICT_MINSIZE) - 1;
	unsigned long msb;
	_BitScanReverse64(&msb, (uint64_t)_minSize);
	return (uint8_t)(msb + 1);
#else
	uint8_t log2Size;
	for (log2Size = ALIFDICT_LOG_MINSIZE;
		(((AlifSizeT)1) << log2Size) < _minSize;
		log2Size++)
		;
	return log2Size;
#endif
}

#define GROWTH_RATE(_d) ((_d)->used*3) // 571

static AlifDictKeysObject _emptyKeysStruct_ = { // 590
		.refCnt = ALIF_IMMORTAL_REFCNT,
		.log2Size = 0,
		.log2IndexBytes = 0,
		.kind = DictKeysKind_::Dict_Kyes_UStr,
		.mutex = {0},
		.version = 1,
		.usable = 0,
		.nentries = 0,
		.indices = {DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY,
		 DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY},
};
#define ALIF_EMPTY_KEYS &_emptyKeysStruct_

static inline AlifIntT getIndex_fromOrder(AlifDictObject* _mp, AlifSizeT _i) { // 617
	uint8_t* array = getInsertion_orderArray(_mp->values);
	return array[_i];
}

static AlifDictKeysObject* new_keysObject(AlifInterpreter* _interp, uint8_t _log2Size, bool _uStr) { // 748
	AlifSizeT usable;
	AlifIntT log2_bytes;
	size_t entry_size = _uStr ? sizeof(AlifDictUStrEntry) : sizeof(AlifDictKeyEntry);

	usable = USABLE_FRACTION((size_t)1 << _log2Size);
	if (_log2Size < 8) {
		log2_bytes = _log2Size;
	}
	else if (_log2Size < 16) {
		log2_bytes = _log2Size + 1;
	}
#if SIZEOF_VOID_P > 4
	else if (_log2Size >= 32) {
		log2_bytes = _log2Size + 3;
	}
#endif
	else {
		log2_bytes = _log2Size + 2;
	}

	AlifDictKeysObject* dk = nullptr;
	if (_log2Size == ALIFDICT_LOG_MINSIZE && _uStr) {
		dk = (AlifDictKeysObject*)ALIF_FREELIST_POP_MEM(dictKeys);
	}
	if (dk == nullptr) {
		dk = (AlifDictKeysObject*)alifMem_objAlloc(sizeof(AlifDictKeysObject)
			+ ((size_t)1 << log2_bytes)
			+ entry_size * usable);
		if (dk == nullptr) {
			//alifErr_noMemory();
			return nullptr;
		}
	}

	dk->refCnt = 1;
	dk->log2Size = _log2Size;
	dk->log2IndexBytes = log2_bytes;
	dk->kind = _uStr ? Dict_Kyes_UStr : Dict_Kyes_General;
#ifdef ALIF_GIL_DISABLED
	dk->mutex = (AlifMutex)0;
#endif
	dk->nentries = 0;
	dk->usable = usable;
	dk->version = 0;
	memset(&dk->indices[0], 0xff, ((size_t)1 << log2_bytes));
	memset(&dk->indices[(size_t)1 << log2_bytes], 0, entry_size * usable);
	return dk;
}


static void free_keysObject(AlifDictKeysObject* _keys, bool _useqsbr) { // 804
	if (_useqsbr) {
		alifMem_freeDelayed(_keys); 
		return;
	}
	if (DK_LOG_SIZE(_keys) == ALIFDICT_LOG_MINSIZE and _keys->kind == DictKeysKind_::Dict_Kyes_UStr) {
		ALIF_FREELIST_FREE(dictKeys, DICT, _keys, alifMem_objFree);
	}
	else {
		alifMem_objFree(_keys);
	}
}

#define CACHED_KEYS(_tp) (((AlifHeapTypeObject*)_tp)->cachedKeys) // 830


static inline void free_values(AlifDictValues* _values, bool _useqsbr) { // 848
	if (_useqsbr) {
		alifMem_freeDelayed(_values);
		return;
	}
	alifMem_objFree(_values);
}

static AlifObject* new_dict(AlifInterpreter* _interp,
	AlifDictKeysObject* _keys, AlifDictValues* _values,
	AlifSizeT _used, AlifIntT _freeValuesOnFailure) { // 860
	AlifDictObject* mp_ = ALIF_FREELIST_POP(AlifDictObject, dicts);
	if (mp_ == nullptr) {
		mp_ = ALIFOBJECT_GC_NEW(AlifDictObject, &_alifDictType_);
		if (mp_ == nullptr) {
			dictKeys_decRef(_interp, _keys, false);
			if (_freeValuesOnFailure) {
				free_values(_values, false);
			}
			return nullptr;
		}
	}
	mp_->keys = _keys;
	mp_->values = _values;
	mp_->used = _used;
	mp_->versionTag = DICT_NEXT_VERSION(_interp);
	return (AlifObject*)mp_;
}





AlifObject* alifDict_new() { // 962
	AlifInterpreter* interp = alifInterpreter_get();
	return new_dict(interp, ALIF_EMPTY_KEYS, nullptr, 0, 0);
}

static inline ALIF_ALWAYS_INLINE AlifSizeT do_lookup(AlifDictObject* _mp, AlifDictKeysObject* _dk, AlifObject* _key, AlifUSizeT _hash,
	AlifIntT (*_checkLookup)(AlifDictObject*, AlifDictKeysObject*, void*, AlifSizeT _ix, AlifObject* _key, AlifUSizeT)) { // 994
	void* ep0 = _dk_entries(_dk);
	AlifUSizeT mask = DK_MASK(_dk);
	AlifUSizeT perturb = _hash;
	AlifUSizeT i = (AlifUSizeT)_hash & mask;
	AlifSizeT ix_;
	for (;;) {
		ix_ = dictKeys_getIndex(_dk, i);
		if (ix_ >= 0) {
			AlifIntT cmp = _checkLookup(_mp, _dk, ep0, ix_, _key, _hash);
			if (cmp < 0) {
				return cmp;
			}
			else if (cmp) {
				return ix_;
			}
		}
		else if (ix_ == DKIX_EMPTY) {
			return DKIX_EMPTY;
		}
		perturb >>= PERTURB_SHIFT;
		i = mask & (i * 5 + perturb + 1);

		// Manual loop unrolling
		ix_ = dictKeys_getIndex(_dk, i);
		if (ix_ >= 0) {
			AlifIntT cmp = _checkLookup(_mp, _dk, ep0, ix_, _key, _hash);
			if (cmp < 0) {
				return cmp;
			}
			else if (cmp) {
				return ix_;
			}
		}
		else if (ix_ == DKIX_EMPTY) {
			return DKIX_EMPTY;
		}
		perturb >>= PERTURB_SHIFT;
		i = mask & (i * 5 + perturb + 1);
	}
	ALIF_UNREACHABLE();
}

static inline AlifIntT compare_uStrGeneric(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifUSizeT _hash) { // 1038
	AlifDictUStrEntry* ep_ = &((AlifDictUStrEntry*)_ep0)[_ix];
	if (uStr_getHash(ep_->key) == _hash) {
		AlifObject* startkey = ep_->key;
		ALIF_INCREF(startkey);
		AlifIntT cmp = alifObject_richCompareBool(startkey, _key, ALIF_EQ);
		ALIF_DECREF(startkey);
		if (cmp < 0) {
			return DKIX_ERROR;
		}
		if (_dk == _mp->keys && ep_->key == startkey) {
			return cmp;
		}
		else {
			/* The dict was mutated, restart */
			return DKIX_KEY_CHANGED;
		}
	}
	return 0;
}

static AlifSizeT uStrKeys_lookupGeneric(AlifDictObject* _mp, AlifDictKeysObject* _dk, AlifObject* _key, AlifUSizeT _hash) { // 1066
	return do_lookup(_mp, _dk, _key, _hash, compare_uStrGeneric);
}

static inline AlifIntT compare_uStrUStr(AlifDictObject* mp, AlifDictKeysObject* dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifUSizeT _hash) { // 1072
	AlifDictUStrEntry* ep_ = &((AlifDictUStrEntry*)_ep0)[_ix];
	AlifObject* epKey = (AlifObject*)alifAtomic_loadPtrRelaxed(ep_->key);
	if (epKey == _key ||
		(uStr_getHash(epKey) == _hash && uStr_eq(epKey, _key))) {
		return 1;
	}
	return 0;
}

static AlifSizeT ALIF_HOT_FUNCTION uStrKeys_lookupUStr(AlifDictKeysObject* _dk, AlifObject* _key, AlifUSizeT _hash) { // 1087
	return do_lookup(nullptr, _dk, _key, _hash, compare_uStrUStr);
}

AlifSizeT alifDict_lookup(AlifDictObject* _mp, AlifObject* _key, AlifUSizeT _hash, AlifObject** _valueAddr) { // 1173
	AlifDictKeysObject* dk;
	DictKeysKind_ kind;
	AlifSizeT ix;

start:
	dk = _mp->keys;
	kind = (DictKeysKind_)dk->kind;

	if (kind != Dict_Kyes_General) {
		if (ALIFUSTR_CHECKEXACT(_key)) {
#ifdef ALIF_GIL_DISABLED
			if (kind == Dict_Kyes_Split) {
				ix = uStrKeysLookup_uStrThreadSafe(dk, _key, _hash);
				if (ix == DKIX_KEY_CHANGED) {
					LOCK_KEYS(dk);
					ix = uStrKeys_lookupUStr(dk, _key, _hash);
					UNLOCK_KEYS(dk);
				}
			}
			else {
				ix = uStrKeys_lookupUStr(dk, _key, _hash);
			}
#else
			ix = unicodekeys_lookup_unicode(dk, key, hash);
#endif
		}
		else {
			INCREF_KEYS_FT(dk);
			LOCK_KEYS_IF_SPLIT(dk, kind);

			ix = uStrKeys_lookupGeneric(_mp, dk, _key, _hash);

			UNLOCK_KEYS_IF_SPLIT(dk, kind);
			DECREF_KEYS_FT(dk, IS_DICT_SHARED(_mp));
			if (ix == DKIX_KEY_CHANGED) {
				goto start;
			}
		}

		if (ix >= 0) {
			if (kind == Dict_Kyes_Split) {
				*_valueAddr = _mp->values->values[ix];
			}
			else {
				*_valueAddr = dk_UStrEntries(dk)[ix].value;
			}
		}
		else {
			*_valueAddr = NULL;
		}
	}
	else {
		ix = dictKeys_genericLookup(_mp, dk, _key, _hash);
		if (ix == DKIX_KEY_CHANGED) {
			goto start;
		}
		if (ix >= 0) {
			*_valueAddr = dk_entries(dk)[ix].value;
		}
		else {
			*_valueAddr = NULL;
		}
	}

	return ix;
}


static inline void ensureShared_onResize(AlifDictObject* _mp) { // 1266
#ifdef ALIF_GIL_DISABLED

	if (!alif_isOwnedByCurrentThread((AlifObject*)_mp) and !IS_DICT_SHARED(_mp)) {

		SET_DICT_SHARED(_mp);
	}
#endif
}

static inline ALIF_ALWAYS_INLINE AlifIntT compareUStr_uStrThreadSafe(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifUSizeT _hash) { // 1330
	AlifDictUStrEntry* ep = &((AlifDictUStrEntry*)_ep0)[_ix];
	AlifObject* startkey = (AlifObject*)alifAtomic_loadPtrRelaxed(&ep->key);
	if (startkey == _key) {
		return 1;
	}
	if (startkey != nullptr) {
		if (alif_isImmortal(startkey)) {
			return uStr_getHash(startkey) == _hash && uStr_eq(startkey, _key);
		}
		else {
			if (!alif_tryIncrefCompare(&ep->key, startkey)) {
				return DKIX_KEY_CHANGED;
			}
			if (uStr_getHash(startkey) == _hash && uStr_eq(startkey, _key)) {
				ALIF_DECREF(startkey);
				return 1;
			}
			ALIF_DECREF(startkey);
		}
	}
	return 0;
}

static AlifSizeT ALIF_HOT_FUNCTION uStrKeysLookup_uStrThreadSafe(AlifDictKeysObject* _dk, AlifObject* _key, AlifUSizeT _hash) { // 1359
	return do_lookup(nullptr, _dk, _key, _hash, compareUStr_uStrThreadSafe);
}

// 1526
#define MAINTAIN_TRACKING(_mp, _key, _value) do { \
        if (!ALIFOBJECT_GC_IS_TRACKED(_mp)) { \
            if (alifObjectGC_mayBeTracked(_key) || \
				alifObjectGC_mayBeTracked(_value)) { \
                ALIFOBJECT_GC_TRACK(_mp); \
            } \
        } \
    } while(0)


static inline AlifIntT is_unusableSlot(AlifUSizeT _ix) { // 1585
#ifdef ALIF_GIL_DISABLED
	return _ix >= 0 or _ix == DKIX_DUMMY;
#else
	return _ix >= 0;
#endif
}

static AlifSizeT find_emptySlot(AlifDictKeysObject* _keys, AlifUSizeT _hash) { // 1598
	const AlifUSizeT mask = DK_MASK(_keys);
	AlifUSizeT i = _hash & mask;
	AlifSizeT ix = dictKeys_getIndex(_keys, i);
	for (AlifUSizeT perturb = _hash; is_unusableSlot(ix);) {
		perturb >>= PERTURB_SHIFT;
		i = (i * 5 + perturb + 1) & mask;
		ix = dictKeys_getIndex(_keys, i);
	}
	return i;
}

static AlifIntT insertion_resize(AlifInterpreter* _interp, AlifDictObject* _mp, AlifIntT _uStr) { // 1614
	return dictResize(_interp, _mp, calculate_log2KeySize(GROWTH_RATE(_mp)), _uStr);
}

static AlifSizeT insert_splitKey(AlifDictKeysObject* _keys, AlifObject* _key, AlifUSizeT _hash) { // 1658
	AlifSizeT ix_;

#ifdef ALIF_GIL_DISABLED
	ix_ = uStrKeysLookup_uStrThreadSafe(_keys, _key, _hash);
	if (ix_ >= 0) {
		return ix_;
	}
#endif

	LOCK_KEYS(_keys);
	ix_ = uStrKeys_lookupUStr(_keys, _key, _hash);
	if (ix_ == DKIX_EMPTY && _keys->usable > 0) {
		// Insert into new slot
		_keys->version = 0;
		AlifSizeT hashpos = find_emptySlot(_keys, _hash);
		ix_ = _keys->nentries;
		dictKeys_setIndex(_keys, hashpos, ix_);
		AlifDictUStrEntry* ep = &dk_UStrEntries(_keys)[ix_];
		STORE_SHARED_KEY(ep->key, ALIF_NEWREF(_key));
		splitKeys_entryAdded(_keys);
	}
	UNLOCK_KEYS(_keys);
	return ix_;
}

static void insert_splitValue(AlifInterpreter* _interp, AlifDictObject* _mp, AlifObject* _key, AlifObject* _value, AlifSizeT ix_) { // 1689

	MAINTAIN_TRACKING(_mp, _key, _value);
	AlifObject* oldValue = _mp->values->values[ix_];
	if (oldValue == nullptr) {
		//uint64_t newVersion = alifDict_notifyEvent(interp, ALIFDICT_EVENT_ADDED, _mp, _key, _value);
		STORE_SPLIT_VALUE(_mp, ix_, ALIF_NEWREF(_value));
		alifDictValues_addToInsertionOrder(_mp->values, ix_);
		STORE_USED(_mp, _mp->used + 1);
		//_mp->versionTag = newVersion;
	}
	else {
		//uint64_t newVersion = alifDict_notifyEvent(interp, ALIFDICT_EVENT_MODIFIED, _mp, _key, _value);
		STORE_SPLIT_VALUE(_mp, ix_, ALIF_NEWREF(_value));
		//_mp->versionTag = newVersion;
		ALIF_DECREF(oldValue);
	}
}

static AlifIntT insertDict(AlifInterpreter* _interp, AlifDictObject* _mp,
	AlifObject* _key, AlifUSizeT _hash, AlifObject* _value) { // 1720
	AlifObject* oldValue{};
	AlifSizeT ix_{};
	if (DK_IS_USTR(_mp->keys) and !ALIFUSTR_CHECKEXACT(_key)) {
		if (insertion_resize(_interp, _mp, 0) < 0)
			goto Fail;
	}

	if (ALIFDICT_HASSPLITTABLE(_mp)) {
		AlifSizeT ix_ = insert_splitKey(_mp->keys, _key, _hash);
		if (ix_ != DKIX_EMPTY) {
			insert_splitValue(_interp, _mp, _key, _value, ix_);
			ALIF_DECREF(_key);
			ALIF_DECREF(_value);
			return 0;
		}

		if (insertion_resize(_interp, _mp, 1) < 0) {
			goto Fail;
		}
	}

	ix_ = alifDict_lookup(_mp, _key, _hash, &oldValue);
	if (ix_ == DKIX_ERROR)
		goto Fail;

	MAINTAIN_TRACKING(_mp, _key, _value);

	if (ix_ == DKIX_EMPTY) {
		if (insert_combinedDict(_interp, _mp, _hash, _key, _value) < 0) {
			goto Fail;
		}
		STORE_USED(_mp, _mp->used + 1);
		return 0;
	}

	if (oldValue != _value) {
		//uint64_t newVersion = alifDict_notifyEvent(
			//interp, ALIFDICT_EVENT_MODIFIED, mp, key, value);
		if (DK_IS_USTR(_mp->keys)) {
			AlifDictUStrEntry* ep = &dk_UStrEntries(_mp->keys)[ix_];
			STORE_VALUE(ep, _value);
		}
		else {
			AlifDictKeyEntry* ep = &dk_entries(_mp->keys)[ix_];
			STORE_VALUE(ep, _value);
		}
		//_mp->versionTag = newVersion;
	}
	ALIF_XDECREF(oldValue); /* which **CAN** re-enter (see issue #22653) */
	ALIF_DECREF(_key);
	return 0;

Fail:
	ALIF_DECREF(_value);
	ALIF_DECREF(_key);
	return -1;
}

static AlifIntT insertTo_emptyDict(AlifInterpreter* _interp, AlifDictObject* _mp, AlifObject* _key, AlifUSizeT _hash, AlifObject* _value) { // 1795

	AlifIntT uStr = ALIFUSTR_CHECKEXACT(_key);
	AlifDictKeysObject* newKeys = new_keysObject(
		_interp, ALIFDICT_LOG_MINSIZE, uStr);
	if (newKeys == nullptr) {
		ALIF_DECREF(_key);
		ALIF_DECREF(_value);
		return -1;
	}
	//uint64_t newVersion = alifDict_notifyEvent(
		//_interp, ALIFDICT_EVENT_ADDED, _mp, _key, _value);


	MAINTAIN_TRACKING(_mp, _key, _value);

	size_t hasPos = (size_t)_hash & (ALIFDICT_MINSIZE - 1);
	dictKeys_setIndex(newKeys, hasPos, 0);
	if (uStr) {
		AlifDictUStrEntry* ep_ = dk_UStrEntries(newKeys);
		ep_->key = _key;
		STORE_VALUE(ep_, _value);
	}
	else {
		AlifDictKeyEntry* ep_ = dk_entries(newKeys);
		ep_->key = _key;
		ep_->hash = _hash;
		STORE_VALUE(ep_, _value);
	}
	STORE_USED(_mp, _mp->used + 1);
	//_mp->versionTag = newVersion;
	newKeys->usable--;
	newKeys->nentries++;
	alifAtomic_storePtrRelease(_mp->keys, newKeys);
	return 0;
}

static void build_indicesGeneric(AlifDictKeysObject* _keys, AlifDictKeyEntry* _ep, AlifSizeT _n) { // 1847
	size_t mask = DK_MASK(_keys);
	for (AlifSizeT ix_ = 0; ix_ != _n; ix_++, _ep++) {
		AlifUSizeT hash = _ep->hash;
		size_t i = hash & mask;
		for (size_t perturb = hash; dictKeys_getIndex(_keys, i) != DKIX_EMPTY;) {
			perturb >>= PERTURB_SHIFT;
			i = mask & (i * 5 + perturb + 1);
		}
		dictKeys_setIndex(_keys, i, ix_);
	}
}

static void build_indicesUStr(AlifDictKeysObject* _keys, AlifDictUStrEntry* _ep, AlifSizeT _n) { // 1862
	size_t mask = DK_MASK(_keys);
	for (AlifSizeT ix_ = 0; ix_ != _n; ix_++, _ep++) {
		AlifUSizeT hash = uStr_getHash(_ep->key);
		size_t i = hash & mask;
		for (size_t perturb = hash; dictKeys_getIndex(_keys, i) != DKIX_EMPTY;) {
			perturb >>= PERTURB_SHIFT;
			i = mask & (i * 5 + perturb + 1);
		}
		dictKeys_setIndex(_keys, i, ix_);
	}
}

static AlifIntT dictResize(AlifInterpreter* _interp, AlifDictObject* _mp,
	uint8_t _log2NewSize, AlifIntT _uStr) { // 1892
	AlifDictKeysObject* oldkeys, * newkeys;
	AlifDictValues* oldvalues;

	if (_log2NewSize >= SIZEOF_SIZE_T * 8) {
		//alifErr_noMemory();
		return -1;
	}

	oldkeys = _mp->keys;
	oldvalues = _mp->values;

	if (!DK_IS_USTR(oldkeys)) {
		_uStr = 0;
	}

	ensureShared_onResize(_mp);

	newkeys = new_keysObject(_interp, _log2NewSize, _uStr);
	if (newkeys == nullptr) {
		return -1;
	}

	AlifSizeT numentries = _mp->used;

	if (oldvalues != nullptr) {
		LOCK_KEYS(oldkeys);
		AlifDictUStrEntry* oldentries = dk_UStrEntries(oldkeys);

		if (newkeys->kind == Dict_Kyes_General) {
			AlifDictKeyEntry* newentries = dk_entries(newkeys);

			for (AlifSizeT i = 0; i < numentries; i++) {
				AlifIntT index = getIndex_fromOrder(_mp, i);
				AlifDictUStrEntry* ep = &oldentries[index];
				newentries[i].key = ALIF_NEWREF(ep->key);
				newentries[i].hash = uStr_getHash(ep->key);
				newentries[i].value = oldvalues->values[index];
			}
			build_indicesGeneric(newkeys, newentries, numentries);
		}
		else { // split -> combined uStr
			AlifDictUStrEntry* newentries = dk_UStrEntries(newkeys);

			for (AlifSizeT i = 0; i < numentries; i++) {
				AlifIntT index = getIndex_fromOrder(_mp, i);
				AlifDictUStrEntry* ep = &oldentries[index];
				newentries[i].key = ALIF_NEWREF(ep->key);
				newentries[i].value = oldvalues->values[index];
			}
			build_indicesUStr(newkeys, newentries, numentries);
		}
		UNLOCK_KEYS(oldkeys);
		set_keys(_mp, newkeys);
		dictKeys_decRef(_interp, oldkeys, IS_DICT_SHARED(_mp));
		set_values(_mp, nullptr);
		if (oldvalues->embedded) {
			alifAtomic_storeUint8(&oldvalues->valid, 0);
		}
		else {
			free_values(oldvalues, IS_DICT_SHARED(_mp));
		}
	}
	else {  // oldkeys is combined.
		if (oldkeys->kind == Dict_Kyes_General) {
			// generic -> generic
			AlifDictKeyEntry* oldentries = dk_entries(oldkeys);
			AlifDictKeyEntry* newentries = dk_entries(newkeys);
			if (oldkeys->nentries == numentries) {
				memcpy(newentries, oldentries, numentries * sizeof(AlifDictKeyEntry));
			}
			else {
				AlifDictKeyEntry* ep = oldentries;
				for (AlifSizeT i = 0; i < numentries; i++) {
					while (ep->value == nullptr)
						ep++;
					newentries[i] = *ep++;
				}
			}
			build_indicesGeneric(newkeys, newentries, numentries);
		}
		else {  // oldkeys is combined uStr
			AlifDictUStrEntry* oldentries = dk_UStrEntries(oldkeys);
			if (_uStr) { // combined uStr -> combined uStr
				AlifDictUStrEntry* newentries = dk_UStrEntries(newkeys);
				if (oldkeys->nentries == numentries and _mp->keys->kind == Dict_Kyes_UStr) {
					memcpy(newentries, oldentries, numentries * sizeof(AlifDictUStrEntry));
				}
				else {
					AlifDictUStrEntry* ep = oldentries;
					for (AlifSizeT i = 0; i < numentries; i++) {
						while (ep->value == nullptr)
							ep++;
						newentries[i] = *ep++;
					}
				}
				build_indicesUStr(newkeys, newentries, numentries);
			}
			else { // combined uStr -> generic
				AlifDictKeyEntry* newentries = dk_entries(newkeys);
				AlifDictUStrEntry* ep = oldentries;
				for (AlifSizeT i = 0; i < numentries; i++) {
					while (ep->value == nullptr)
						ep++;
					newentries[i].key = ep->key;
					newentries[i].hash = uStr_getHash(ep->key);
					newentries[i].value = ep->value;
					ep++;
				}
				build_indicesGeneric(newkeys, newentries, numentries);
			}
		}

		set_keys(_mp, newkeys);

		if (oldkeys != ALIF_EMPTY_KEYS) {
			free_keysObject(oldkeys, IS_DICT_SHARED(_mp));
		}
	}

	STORE_KEYS_USABLE(_mp->keys, _mp->keys->usable - numentries);
	STORE_KEYS_NENTRIES(_mp->keys, numentries);
	return 0;
}


static AlifIntT setItemTake2_lockHeld(AlifDictObject* mp, AlifObject* key, AlifObject* value) { // 2424
	AlifUSizeT hash = alifObject_hashFast(key);
	if (hash == -1) {
		ALIF_DECREF(key);
		ALIF_DECREF(value);
		return -1;
	}

	AlifInterpreter* interp = alifInterpreter_get();

	if (mp->keys == ALIF_EMPTY_KEYS) {
		return insertTo_emptyDict(interp, mp, key, hash, value);
	}
	return insertDict(interp, mp, key, hash, value);
}

AlifIntT alifDict_setItemTake2(AlifDictObject* _mp, AlifObject* _key, AlifObject* _value) { // 2449
	AlifIntT res_;
	//ALIF_BEGIN_CRITICAL_SECTION(mp);
	res_ = setItemTake2_lockHeld(_mp, _key, _value);
	//ALIF_END_CRITICAL_SECTION();
	return res_;
}


AlifIntT alifDict_setItem(AlifObject* _op, AlifObject* _key, AlifObject* _value) { // 2465
	if (!ALIFDICT_CHECK(_op)) {
		//alifErr_badInternalCall();
		return -1;
	}
	return alifDict_setItemTake2((AlifDictObject*)_op,
		ALIF_NEWREF(_key), ALIF_NEWREF(_value));
}






AlifTypeObject _alifDictType_ = { // 4760
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "قاموس",
	.basicSize = sizeof(AlifDictObject),
};


void alifObject_initInlineValues(AlifObject* _obj, AlifTypeObject* _tp) {  // 6580
	AlifDictKeysObject* keys = CACHED_KEYS(_tp);
	AlifSizeT usable = alifAtomic_loadSizeRelaxed(&keys->usable);
	if (usable > 1) {
		LOCK_KEYS(keys);
		if (keys->usable > 1) {
			alifAtomic_storeSize(&keys->usable, keys->usable - 1);
		}
		UNLOCK_KEYS(keys);
	}
	AlifUSizeT size = sharedKeys_usableSize(keys);
	AlifDictValues* values = alifObject_inlineValues(_obj);
	values->capacity = (uint8_t)size;
	values->size = 0;
	values->embedded = 1;
	values->valid = 1;
	for (AlifUSizeT i = 0; i < size; i++) {
		values->values[i] = nullptr;
	}
	alifObject_managedDictPointer(_obj)->dict = nullptr;
}










void alifDictKeys_decRef(AlifDictKeysObject* _keys) { // 7246
	AlifInterpreter* interp = alifInterpreter_get();
	dictKeys_decRef(interp, _keys, false);
}
