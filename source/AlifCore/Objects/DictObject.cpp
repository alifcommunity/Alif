
#define ALIFDICT_LOG_MINSIZE 3 // 115
#define ALIFDICT_MINSIZE 8 // 116

#include "alif.h"

#include "AlifCore_BitUtils.h"
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
        if (_kind == DictKeysKind_::Dict_Keys_Split) { \
            LOCK_KEYS(_keys);           \
        }

// 178
#define UNLOCK_KEYS_IF_SPLIT(_keys, _kind) \
        if (_kind == DictKeysKind_::Dict_Keys_Split) {   \
            UNLOCK_KEYS(_keys);           \
        }


static inline AlifSizeT load_keysNentries(AlifDictObject * mp) { // 183
	AlifDictKeysObject* keys = (AlifDictKeysObject*)alifAtomic_loadPtr(&mp->keys);
	return alifAtomic_loadSize(&keys->nentries);
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
#define LOAD_SHARED_KEY(_key) alifAtomic_loadPtrAcquire(&_key)

#define STORE_SHARED_KEY(_key, _value) alifAtomic_storePtrRelease(&_key, _value) // 209
#define INCREF_KEYS(_dk)  alifAtomic_addSize(&_dk->refCnt, 1) // 211
#define DECREF_KEYS(_dk)  alifAtomic_addSize(&_dk->refCnt, -1) // 213

#define INCREF_KEYS_FT(_dk) dictKeys_incRef(_dk) // 216
#define DECREF_KEYS_FT(_dk, _shared) dictKeys_decRef(_alifInterpreter_get(), _dk, _shared) // 217


static inline void splitKeys_entryAdded(AlifDictKeysObject* _keys) { // 219
	alifAtomic_storeSizeRelaxed(&_keys->nentries, _keys->nentries + 1);
	alifAtomic_storeSizeRelease(&_keys->usable, _keys->usable - 1);
}


#define STORE_KEY(ep_, _key) alifAtomic_storePtrRelease(&ep_->key, _key) // 278
#define STORE_VALUE(ep_, _value) alifAtomic_storePtrRelease(&ep_->value, _value) // 279
#define STORE_SPLIT_VALUE(_mp, _idx, _value) alifAtomic_storePtrRelease(&_mp->values->values[_idx], _value) // 280
#define STORE_HASH(ep_, _hash) alifAtomic_storeSizeRelaxed(&ep_->hash, _hash) // 281
#define STORE_KEYS_USABLE(_keys, _usable) alifAtomic_storeSizeRelaxed(&_keys->usable, _usable) // 282
#define STORE_KEYS_NENTRIES(_keys, _nentries) alifAtomic_storeSizeRelaxed(&_keys->nentries, _nentries) // 283
#define STORE_USED(_mp, _used) alifAtomic_storeSizeRelaxed(&_mp->used, _used) // 284


#define PERTURB_SHIFT 5 // 286

static AlifIntT dict_resize(AlifInterpreter* ,AlifDictObject* ,uint8_t , AlifIntT); // 380

static AlifObject* dict_iter(AlifObject*); // 383

static AlifIntT setItem_lockHeld(AlifDictObject*, AlifObject*, AlifObject*); // 385


#include "clinic/DictObject.cpp.h"


static inline AlifUSizeT uStr_getHash(AlifObject* _o) { // 399
	return alifAtomic_loadSizeRelaxed(&ALIFASCIIOBJECT_CAST(_o)->hash);
}

#define DK_MASK(_dk) (DK_SIZE(_dk)-1) // 417

#define ALIF_DICT_IMMORTAL_INITIAL_REFCNT ALIF_SIZET_MIN // 419

static void free_keysObject(AlifDictKeysObject*, bool); // 421


static inline void dictKeys_incRef(AlifDictKeysObject* _dk) { // 430
	if (alifAtomic_loadSizeRelaxed(&_dk->refCnt) < 0) {
		return;
	}

	INCREF_KEYS(_dk);
}

static inline void dictKeys_decRef(AlifInterpreter* _interp,
	AlifDictKeysObject* _dk, bool _useqsbr) { // 442
	if (alifAtomic_loadSizeRelaxed(&_dk->refCnt) < 0) {
		return;
	}
	if (DECREF_KEYS(_dk) == 1) {
		if (DK_IS_USTR(_dk)) {
			AlifDictUStrEntry* entries = dk_uStrEntries(_dk);
			AlifSizeT i{}, n{};
			for (i = 0, n = _dk->nentries; i < n; i++) {
				ALIF_XDECREF(entries[i].key);
				ALIF_XDECREF(entries[i].value);
			}
		}
		else {
			AlifDictKeyEntry* entries = dk_entries(_dk);
			AlifSizeT i{}, n{};
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
	AlifSizeT ix_{};

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
	unsigned long msb{};
	_BitScanReverse64(&msb, (uint64_t)_minSize);
	return (uint8_t)(msb + 1);
#else
	uint8_t log2Size;
	for (log2Size = ALIFDICT_LOG_MINSIZE;
		(((AlifSizeT)1) << log2Size) < _minSize;
		log2Size++);
	return log2Size;
#endif
}

static inline uint8_t estimate_log2Keysize(AlifSizeT _n) { // 569
	return calculate_log2KeySize((_n * 3 + 1) / 2);
}

#define GROWTH_RATE(_d) ((_d)->used*3) // 585

static AlifDictKeysObject _emptyKeysStruct_ = { // 590
		.refCnt = ALIF_DICT_IMMORTAL_INITIAL_REFCNT,
		.log2Size = 0,
		.log2IndexBytes = 0,
		.kind = DictKeysKind_::Dict_Keys_UStr,
		.mutex = { .bits = 0 },
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

static AlifDictKeysObject* new_keysObject(AlifInterpreter* _interp,
	uint8_t _log2Size, bool _uStr) { // 748
	AlifSizeT usable{};
	AlifIntT log2Bytes{};
	AlifUSizeT entrySize = _uStr ? sizeof(AlifDictUStrEntry) : sizeof(AlifDictKeyEntry);

	usable = USABLE_FRACTION((AlifUSizeT)1 << _log2Size);
	if (_log2Size < 8) {
		log2Bytes = _log2Size;
	}
	else if (_log2Size < 16) {
		log2Bytes = _log2Size + 1;
	}
#if SIZEOF_VOID_P > 4
	else if (_log2Size >= 32) {
		log2Bytes = _log2Size + 3;
	}
#endif
	else {
		log2Bytes = _log2Size + 2;
	}

	AlifDictKeysObject* dk = nullptr;
	if (_log2Size == ALIFDICT_LOG_MINSIZE and _uStr) {
		dk = (AlifDictKeysObject*)ALIF_FREELIST_POP_MEM(dictKeys);
	}
	if (dk == nullptr) {
		dk = (AlifDictKeysObject*)alifMem_objAlloc(sizeof(AlifDictKeysObject)
			+ ((AlifUSizeT)1 << log2Bytes)
			+ entrySize * usable);
		if (dk == nullptr) {
			//alifErr_noMemory();
			return nullptr;
		}
	}

	dk->refCnt = 1;
	dk->log2Size = _log2Size;
	dk->log2IndexBytes = log2Bytes;
	dk->kind = _uStr ? DictKeysKind_::Dict_Keys_UStr : DictKeysKind_::Dict_Keys_General;
	dk->mutex = {0};
	dk->nentries = 0;
	dk->usable = usable;
	dk->version = 0;
	memset(&dk->indices[0], 0xff, ((AlifUSizeT)1 << log2Bytes));
	memset(&dk->indices[(AlifUSizeT)1 << log2Bytes], 0, entrySize * usable);
	return dk;
}


static void free_keysObject(AlifDictKeysObject* _keys, bool _useqsbr) { // 804
	if (_useqsbr) {
		alifMem_freeDelayed(_keys); 
		return;
	}
	if (DK_LOG_SIZE(_keys) == ALIFDICT_LOG_MINSIZE and _keys->kind == DictKeysKind_::Dict_Keys_UStr) {
		ALIF_FREELIST_FREE(dictKeys, DICTS, _keys, alifMem_objFree);
	}
	else {
		alifMem_objFree(_keys);
	}
}

static AlifUSizeT valuesSize_fromCount(AlifUSizeT _count) { // 820
	size_t suffix_size = ALIF_SIZE_ROUND_UP(_count, sizeof(AlifObject*));
	return (_count + 1) * sizeof(AlifObject*) + suffix_size;
}

#define CACHED_KEYS(_tp) (((AlifHeapTypeObject*)_tp)->cachedKeys) // 830

static inline AlifDictValues* new_values(AlifUSizeT _size) { // 832
	AlifUSizeT n = valuesSize_fromCount(_size);
	AlifDictValues* res = (AlifDictValues*)alifMem_dataAlloc(n);
	if (res == nullptr) {
		return nullptr;
	}
	res->embedded = 0;
	res->size = 0;
	res->capacity = (uint8_t)_size;
	return res;
}



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
	mp_->watcherTag = 0;
	return (AlifObject*)mp_;
}




static AlifObject* newDict_withSharedKeys(AlifInterpreter* interp, AlifDictKeysObject* keys) { // 887
	AlifUSizeT size = sharedKeys_usableSize(keys);
	AlifDictValues* values = new_values(size);
	if (values == nullptr) {
		//return alifErr_noMemory();
		return nullptr;
	}
	dictKeys_incRef(keys);
	for (AlifUSizeT i = 0; i < size; i++) {
		values->values[i] = nullptr;
	}
	return new_dict(interp, keys, values, 0, 1);
}


static AlifDictKeysObject* cloneCombined_dictKeys(AlifDictObject* orig) { // 903
	AlifUSizeT keys_size = _alifDict_keysSize(orig->keys);
	AlifDictKeysObject* keys = (AlifDictKeysObject*)alifMem_objAlloc(keys_size);
	if (keys == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}

	memcpy(keys, orig->keys, keys_size);

	AlifObject** pkey{}, ** pvalue{};
	AlifUSizeT offs{};
	if (DK_IS_USTR(orig->keys)) {
		AlifDictUStrEntry* ep0 = dk_uStrEntries(keys);
		pkey = &ep0->key;
		pvalue = &ep0->value;
		offs = sizeof(AlifDictUStrEntry) / sizeof(AlifObject*);
	}
	else {
		AlifDictKeyEntry* ep0 = dk_entries(keys);
		pkey = &ep0->key;
		pvalue = &ep0->value;
		offs = sizeof(AlifDictKeyEntry) / sizeof(AlifObject*);
	}

	AlifSizeT n = keys->nentries;
	for (AlifSizeT i = 0; i < n; i++) {
		AlifObject* value = *pvalue;
		if (value != nullptr) {
			ALIF_INCREF(value);
			ALIF_INCREF(*pkey);
		}
		pvalue += offs;
		pkey += offs;
	}

	return keys;
}



AlifObject* alifDict_new() { // 962
	AlifInterpreter* interp = _alifInterpreter_get();
	return new_dict(interp, ALIF_EMPTY_KEYS, nullptr, 0, 0);
}

static AlifSizeT lookDict_index(AlifDictKeysObject* _k, AlifHashT _hash, AlifSizeT _index) { // 971
	AlifUSizeT mask = DK_MASK(_k);
	AlifUSizeT perturb = (AlifUSizeT)_hash;
	AlifUSizeT i_ = (AlifUSizeT)_hash & mask;

	for (;;) {
		AlifSizeT ix = dictKeys_getIndex(_k, i_);
		if (ix == _index) {
			return i_;
		}
		if (ix == DKIX_EMPTY) {
			return DKIX_EMPTY;
		}
		perturb >>= PERTURB_SHIFT;
		i_ = mask & (i_ * 5 + perturb + 1);
	}
	ALIF_UNREACHABLE();
}

static inline ALIF_ALWAYS_INLINE AlifSizeT do_lookup(AlifDictObject* _mp,
	AlifDictKeysObject* _dk, AlifObject* _key, AlifHashT _hash,
	AlifIntT (*_checkLookup)(AlifDictObject*, AlifDictKeysObject*, void*, AlifSizeT _ix, AlifObject* _key, AlifHashT)) { // 992
	void* ep0 = _dk_entries(_dk);
	AlifUSizeT mask = DK_MASK(_dk);
	AlifUSizeT perturb = _hash;
	AlifUSizeT i = (AlifUSizeT)_hash & mask;
	AlifSizeT ix_{};
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
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifHashT _hash) { // 1038
	AlifDictUStrEntry* ep_ = &((AlifDictUStrEntry*)_ep0)[_ix];
	if (uStr_getHash(ep_->key) == _hash) {
		AlifObject* startkey = ep_->key;
		ALIF_INCREF(startkey);
		AlifIntT cmp = alifObject_richCompareBool(startkey, _key, ALIF_EQ);
		ALIF_DECREF(startkey);
		if (cmp < 0) {
			return DKIX_ERROR;
		}
		if (_dk == _mp->keys and ep_->key == startkey) {
			return cmp;
		}
		else {
			/* The dict was mutated, restart */
			return DKIX_KEY_CHANGED;
		}
	}
	return 0;
}

static AlifSizeT uStrKeys_lookupGeneric(AlifDictObject* _mp,
	AlifDictKeysObject* _dk, AlifObject* _key, AlifHashT _hash) { // 1066
	return do_lookup(_mp, _dk, _key, _hash, compare_uStrGeneric);
}

static inline AlifIntT compare_uStrUStr(AlifDictObject* _mp, AlifDictKeysObject* dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifHashT _hash) { // 1072
	AlifDictUStrEntry* ep_ = &((AlifDictUStrEntry*)_ep0)[_ix];
	AlifObject* epKey = (AlifObject*)alifAtomic_loadPtrRelaxed(&ep_->key);
	if (epKey == _key or
		(uStr_getHash(epKey) == _hash and uStr_eq(epKey, _key))) {
		return 1;
	}
	return 0;
}

static AlifSizeT ALIF_HOT_FUNCTION uStrKeys_lookupUStr(AlifDictKeysObject* _dk,
	AlifObject* _key, AlifHashT _hash) { // 1087
	return do_lookup(nullptr, _dk, _key, _hash, compare_uStrUStr);
}

static inline AlifIntT compare_generic(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifHashT _hash) { // 1093
	AlifDictKeyEntry* ep_ = &((AlifDictKeyEntry*)_ep0)[_ix];
	if (ep_->key == _key) {
		return 1;
	}
	if (ep_->hash == _hash) {
		AlifObject* startkey = ep_->key;
		ALIF_INCREF(startkey);
		AlifIntT cmp = alifObject_richCompareBool(startkey, _key, ALIF_EQ);
		ALIF_DECREF(startkey);
		if (cmp < 0) {
			return DKIX_ERROR;
		}
		if (_dk == _mp->keys and ep_->key == startkey) {
			return cmp;
		}
		else {
			return DKIX_KEY_CHANGED;
		}
	}
	return 0;
}

static AlifSizeT dictKeys_genericLookup(AlifDictObject* _mp, AlifDictKeysObject* _dk, AlifObject* _key, AlifHashT _hash) { // 1121
	return do_lookup(_mp, _dk, _key, _hash, compare_generic);
}

AlifSizeT alifDictKeys_stringLookup(AlifDictKeysObject* _dk, AlifObject* _key) { // 1133
	DictKeysKind_ kind = (DictKeysKind_)_dk->kind;
	if (!ALIFUSTR_CHECKEXACT(_key) or kind == Dict_Keys_General) {
		return DKIX_ERROR;
	}
	AlifHashT hash = uStr_getHash(_key);
	if (hash == -1) {
		hash = _alifUStrType_.hash(_key);
		if (hash == -1) {
			//alfiErr_clear();
			return DKIX_ERROR;
		}
	}
	return uStrKeys_lookupUStr(_dk, _key, hash);
}

static AlifSizeT uStrKeys_lookupUStrThreadSafe(AlifDictKeysObject*, AlifObject*, AlifHashT); // 1153

AlifSizeT alifDict_lookup(AlifDictObject* _mp, AlifObject* _key,
	AlifHashT _hash, AlifObject** _valueAddr) { // 1173
	AlifDictKeysObject* dk_{};
	DictKeysKind_ kind{};
	AlifSizeT ix_{};

start:
	dk_ = _mp->keys;
	kind = (DictKeysKind_)dk_->kind;

	if (kind != DictKeysKind_::Dict_Keys_General) {
		if (ALIFUSTR_CHECKEXACT(_key)) {
			if (kind == DictKeysKind_::Dict_Keys_Split) {
				ix_ = uStrKeys_lookupUStrThreadSafe(dk_, _key, _hash);
				if (ix_ == DKIX_KEY_CHANGED) {
					LOCK_KEYS(dk_);
					ix_ = uStrKeys_lookupUStr(dk_, _key, _hash);
					UNLOCK_KEYS(dk_);
				}
			}
			else {
				ix_ = uStrKeys_lookupUStr(dk_, _key, _hash);
			}
		}
		else {
			INCREF_KEYS_FT(dk_);
			LOCK_KEYS_IF_SPLIT(dk_, kind);

			ix_ = uStrKeys_lookupGeneric(_mp, dk_, _key, _hash);

			UNLOCK_KEYS_IF_SPLIT(dk_, kind);
			DECREF_KEYS_FT(dk_, IS_DICT_SHARED(_mp));
			if (ix_ == DKIX_KEY_CHANGED) {
				goto start;
			}
		}

		if (ix_ >= 0) {
			if (kind == DictKeysKind_::Dict_Keys_Split) {
				*_valueAddr = _mp->values->values[ix_];
			}
			else {
				*_valueAddr = dk_uStrEntries(dk_)[ix_].value;
			}
		}
		else {
			*_valueAddr = nullptr;
		}
	}
	else {
		ix_ = dictKeys_genericLookup(_mp, dk_, _key, _hash);
		if (ix_ == DKIX_KEY_CHANGED) {
			goto start;
		}
		if (ix_ >= 0) {
			*_valueAddr = dk_entries(dk_)[ix_].value;
		}
		else {
			*_valueAddr = nullptr;
		}
	}

	return ix_;
}


static inline void ensureShared_onRead(AlifDictObject* mp) {
	if (!alif_isOwnedByCurrentThread((AlifObject*)mp) and !IS_DICT_SHARED(mp)) {
		ALIF_BEGIN_CRITICAL_SECTION(mp);
		if (!IS_DICT_SHARED(mp)) {
			SET_DICT_SHARED(mp);
		}
		ALIF_END_CRITICAL_SECTION();
	}
}


static inline void ensureShared_onResize(AlifDictObject* _mp) { // 1266

	if (!alif_isOwnedByCurrentThread((AlifObject*)_mp) and !IS_DICT_SHARED(_mp)) {

		SET_DICT_SHARED(_mp);
	}
}


static inline ALIF_ALWAYS_INLINE AlifIntT compareUstr_genericThreadsafe(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifHashT _hash) { // 1288
	AlifDictUStrEntry* ep = &((AlifDictUStrEntry*)_ep0)[_ix];
	AlifObject* startkey = (AlifObject*)alifAtomic_loadPtrRelaxed(&ep->key);

	if (startkey != nullptr) {
		if (!alif_tryIncRefCompare(&ep->key, startkey)) {
			return DKIX_KEY_CHANGED;
		}

		if (uStr_getHash(startkey) == _hash) {
			AlifIntT cmp = alifObject_richCompareBool(startkey, _key, ALIF_EQ);
			ALIF_DECREF(startkey);
			if (cmp < 0) {
				return DKIX_ERROR;
			}
			if (_dk == alifAtomic_loadPtrRelaxed(&_mp->keys) and
				startkey == alifAtomic_loadPtrRelaxed(&ep->key)) {
				return cmp;
			}
			else {
				/* The dict was mutated, restart */
				return DKIX_KEY_CHANGED;
			}
		}
		else {
			ALIF_DECREF(startkey);
		}
	}
	return 0;
}

// Search non-Unicode key from Unicode table
static AlifSizeT uStrKeys_lookupGenericThreadSafe(AlifDictObject* _mp, AlifDictKeysObject* _dk, AlifObject* _key, AlifHashT _hash) { // 1325
	return do_lookup(_mp, _dk, _key, _hash, compareUstr_genericThreadsafe);
}

static inline ALIF_ALWAYS_INLINE AlifIntT compareUStr_uStrThreadSafe(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifHashT _hash) { // 1330
	AlifDictUStrEntry* ep_ = &((AlifDictUStrEntry*)_ep0)[_ix];
	AlifObject* startkey = (AlifObject*)alifAtomic_loadPtrRelaxed(&ep_->key);
	if (startkey == _key) {
		return 1;
	}
	if (startkey != nullptr) {
		if (ALIF_ISIMMORTAL(startkey)) {
			return (uStr_getHash(startkey) == _hash and uStr_eq(startkey, _key));
		}
		else {
			if (!alif_tryIncRefCompare(&ep_->key, startkey)) {
				return DKIX_KEY_CHANGED;
			}
			if (uStr_getHash(startkey) == _hash and uStr_eq(startkey, _key)) {
				ALIF_DECREF(startkey);
				return 1;
			}
			ALIF_DECREF(startkey);
		}
	}
	return 0;
}

static AlifSizeT ALIF_HOT_FUNCTION uStrKeys_lookupUStrThreadSafe(AlifDictKeysObject* _dk,
	AlifObject* _key, AlifHashT _hash) { // 1359
	return do_lookup(nullptr, _dk, _key, _hash, compareUStr_uStrThreadSafe);
}

static inline ALIF_ALWAYS_INLINE AlifIntT compareGeneric_threadSafe(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, AlifSizeT _ix, AlifObject* _key, AlifHashT _hash) { // 1365
	AlifDictKeyEntry* ep = &((AlifDictKeyEntry*)_ep0)[_ix];
	AlifObject* startkey = (AlifObject*)alifAtomic_loadPtrRelaxed(&ep->key);
	if (startkey == _key) {
		return 1;
	}
	AlifSizeT ep_hash = alifAtomic_loadSizeRelaxed(&ep->hash);
	if (ep_hash == _hash) {
		if (startkey == nullptr or !alif_tryIncRefCompare(&ep->key, startkey)) {
			return DKIX_KEY_CHANGED;
		}
		AlifIntT cmp = alifObject_richCompareBool(startkey, _key, ALIF_EQ);
		ALIF_DECREF(startkey);
		if (cmp < 0) {
			return DKIX_ERROR;
		}
		if (_dk == alifAtomic_loadPtrRelaxed(&_mp->keys) and
			startkey == alifAtomic_loadPtrRelaxed(&ep->key)) {
			return cmp;
		}
		else {
			/* The dict was mutated, restart */
			return DKIX_KEY_CHANGED;
		}
	}
	return 0;
}

static AlifSizeT dictKeysGeneric_lookupThreadSafe(AlifDictObject* _mp,
	AlifDictKeysObject* _dk, AlifObject* _key, AlifHashT _hash) { // 1396
	return do_lookup(_mp, _dk, _key, _hash, compareGeneric_threadSafe);
}

AlifSizeT alifDict_lookupThreadSafe(AlifDictObject* _mp,
	AlifObject* _key, AlifHashT _hash, AlifObject** _valueAddr) { // 1402
	AlifDictKeysObject* dk_{};
	DictKeysKind_ kind{};
	AlifSizeT ix_{};
	AlifObject* value{};

	ensureShared_onRead(_mp);

	dk_ = (AlifDictKeysObject*)alifAtomic_loadPtr(&_mp->keys);
	kind = (DictKeysKind_)dk_->kind;

	if (kind != DictKeysKind_::Dict_Keys_General) {
		if (ALIFUSTR_CHECKEXACT(_key)) {
			ix_ = uStrKeys_lookupUStrThreadSafe(dk_, _key, _hash);
		}
		else {
			ix_ = uStrKeys_lookupGenericThreadSafe(_mp, dk_, _key, _hash);
		}
		if (ix_ == DKIX_KEY_CHANGED) {
			goto read_failed;
		}

		if (ix_ >= 0) {
			if (kind == DictKeysKind_::Dict_Keys_Split) {
				AlifDictValues* values = (AlifDictValues*)alifAtomic_loadPtr(&_mp->values);
				if (values == nullptr)
					goto read_failed;

				uint8_t capacity = alifAtomic_loadUint8Relaxed(&values->capacity);
				if (ix_ >= (AlifSizeT)capacity)
					goto read_failed;

				value = alif_tryXGetRef(&values->values[ix_]);
				if (value == nullptr)
					goto read_failed;

				if (values != alifAtomic_loadPtr(&_mp->values)) {
					ALIF_DECREF(value);
					goto read_failed;
				}
			}
			else {
				value = alif_tryXGetRef(&dk_uStrEntries(dk_)[ix_].value);
				if (value == nullptr) {
					goto read_failed;
				}

				if (dk_ != alifAtomic_loadPtr(&_mp->keys)) {
					ALIF_DECREF(value);
					goto read_failed;
				}
			}
		}
		else {
			value = nullptr;
		}
	}
	else {
		ix_ = dictKeysGeneric_lookupThreadSafe(_mp, dk_, _key, _hash);
		if (ix_ == DKIX_KEY_CHANGED) {
			goto read_failed;
		}
		if (ix_ >= 0) {
			value = alif_tryXGetRef(&dk_entries(dk_)[ix_].value);
			if (value == nullptr)
				goto read_failed;

			if (dk_ != alifAtomic_loadPtr(&_mp->keys)) {
				ALIF_DECREF(value);
				goto read_failed;
			}
		}
		else {
			value = nullptr;
		}
	}

	*_valueAddr = value;
	return ix_;

read_failed:
	ALIF_BEGIN_CRITICAL_SECTION(_mp);
	ix_ = alifDict_lookup(_mp, _key, _hash, &value);
	*_valueAddr = value;
	if (value != nullptr) {
		_alif_newRefWithLock(value);
	}
	ALIF_END_CRITICAL_SECTION();
	return ix_;
}


AlifSizeT _alifDict_lookupThreadSafeStackRef(AlifDictObject* _mp, AlifObject* _key,
	AlifHashT _hash, AlifStackRef* _valueAddr) { // 1499
	AlifDictKeysObject* dk = (AlifDictKeysObject*)alifAtomic_loadPtr(&_mp->keys);
	if (dk->kind == DictKeysKind_::Dict_Keys_UStr and ALIFUSTR_CHECKEXACT(_key)) {
		AlifSizeT ix = uStrKeys_lookupUStrThreadSafe(dk, _key, _hash);
		if (ix == DKIX_EMPTY) {
			*_valueAddr = _alifStackRefNull_;
			return ix;
		}
		else if (ix >= 0) {
			AlifObject** addr_of_value = &dk_uStrEntries(dk)[ix].value;
			AlifObject* value = (AlifObject*)alifAtomic_loadPtr(addr_of_value);
			if (value == nullptr) {
				*_valueAddr = _alifStackRefNull_;
				return DKIX_EMPTY;
			}
			if (ALIF_ISIMMORTAL(value) or _alifObject_hasDeferredRefCount(value)) {
				*_valueAddr = AlifStackRef({ .bits = (uintptr_t)value | ALIF_TAG_DEFERRED });
				return ix;
			}
			if (alif_tryIncRefCompare(addr_of_value, value)) {
				*_valueAddr = ALIFSTACKREF_FROMALIFOBJECTSTEAL(value);
				return ix;
			}
		}
	}

	AlifObject* obj{};
	AlifSizeT ix = alifDict_lookupThreadSafe(_mp, _key, _hash, &obj);
	if (ix >= 0 and obj != nullptr) {
		*_valueAddr = ALIFSTACKREF_FROMALIFOBJECTSTEAL(obj);
	}
	else {
		*_valueAddr = _alifStackRefNull_;
	}
	return ix;
}


AlifIntT _alifDict_hasOnlyStringKeys(AlifObject* _dict) { // 1511
	AlifSizeT pos = 0;
	AlifObject* key{}, * value{};
	/* Shortcut */
	if (((AlifDictObject*)_dict)->keys->kind != DictKeysKind_::Dict_Keys_General)
		return 1;
	while (alifDict_next(_dict, &pos, &key, &value))
		if (!ALIFUSTR_CHECK(key))
			return 0;
	return 1;
}


// 1526
#define MAINTAIN_TRACKING(_mp, _key, _value) do { \
        if (!ALIFOBJECT_GC_IS_TRACKED(_mp)) { \
            if (alifObject_gcMayBeTracked(_key) or \
				alifObject_gcMayBeTracked(_value)) { \
                ALIFOBJECT_GC_TRACK(_mp); \
            } \
        } \
    } while(0)


static inline AlifIntT is_unusableSlot(AlifSizeT _ix) { // 1585
	return _ix >= 0 or _ix == DKIX_DUMMY;
}

static AlifSizeT find_emptySlot(AlifDictKeysObject* _keys, AlifHashT _hash) { // 1598
	const AlifUSizeT mask = DK_MASK(_keys);
	AlifUSizeT i_ = _hash & mask;
	AlifSizeT ix_ = dictKeys_getIndex(_keys, i_);
	for (AlifUSizeT perturb = _hash; is_unusableSlot(ix_);) {
		perturb >>= PERTURB_SHIFT;
		i_ = (i_ * 5 + perturb + 1) & mask;
		ix_ = dictKeys_getIndex(_keys, i_);
	}
	return i_;
}

static AlifIntT insertion_resize(AlifInterpreter* _interp,
	AlifDictObject* _mp, AlifIntT _uStr) { // 1614
	return dict_resize(_interp, _mp, calculate_log2KeySize(GROWTH_RATE(_mp)), _uStr);
}

static inline AlifIntT insert_combinedDict(AlifInterpreter* _interp, AlifDictObject* _mp,
	AlifHashT _hash, AlifObject* _key, AlifObject* _value) { // 1620
	if (_mp->keys->usable <= 0) {
		if (insertion_resize(_interp, _mp, 1) < 0) {
			return -1;
		}
	}
	
	_alifDict_notifyEvent(_interp, AlifDictWatchEvent_::AlifDict_Event_Added, _mp, _key, _value);
	_mp->keys->version = 0;

	AlifSizeT hashpos = find_emptySlot(_mp->keys, _hash);
	dictKeys_setIndex(_mp->keys, hashpos, _mp->keys->nentries);

	if (DK_IS_USTR(_mp->keys)) {
		AlifDictUStrEntry* ep_;
		ep_ = &dk_uStrEntries(_mp->keys)[_mp->keys->nentries];
		STORE_KEY(ep_, _key);
		STORE_VALUE(ep_, _value);
	}
	else {
		AlifDictKeyEntry* ep_;
		ep_ = &dk_entries(_mp->keys)[_mp->keys->nentries];
		STORE_KEY(ep_, _key);
		STORE_VALUE(ep_, _value);
		STORE_HASH(ep_, _hash);
	}

	STORE_KEYS_USABLE(_mp->keys, _mp->keys->usable - 1);
	STORE_KEYS_NENTRIES(_mp->keys, _mp->keys->nentries + 1);
	return 0;
}

static AlifSizeT insert_splitKey(AlifDictKeysObject* _keys,
	AlifObject* _key, AlifHashT _hash) { // 1658
	AlifSizeT ix_{};

	ix_ = uStrKeys_lookupUStrThreadSafe(_keys, _key, _hash);
	if (ix_ >= 0) {
		return ix_;
	}

	LOCK_KEYS(_keys);
	ix_ = uStrKeys_lookupUStr(_keys, _key, _hash);
	if (ix_ == DKIX_EMPTY and _keys->usable > 0) {
		// Insert into new slot
		_keys->version = 0;
		AlifSizeT hashpos = find_emptySlot(_keys, _hash);
		ix_ = _keys->nentries;
		dictKeys_setIndex(_keys, hashpos, ix_);
		AlifDictUStrEntry* ep_ = &dk_uStrEntries(_keys)[ix_];
		STORE_SHARED_KEY(ep_->key, ALIF_NEWREF(_key));
		splitKeys_entryAdded(_keys);
	}
	UNLOCK_KEYS(_keys);
	return ix_;
}

static void insert_splitValue(AlifInterpreter* _interp,
	AlifDictObject* _mp, AlifObject* _key, AlifObject* _value, AlifSizeT ix_) { // 1689

	MAINTAIN_TRACKING(_mp, _key, _value);
	AlifObject* oldValue = _mp->values->values[ix_];
	if (oldValue == nullptr) {
		_alifDict_notifyEvent(_interp, AlifDictWatchEvent_::AlifDict_Event_Added, _mp, _key, _value);
		STORE_SPLIT_VALUE(_mp, ix_, ALIF_NEWREF(_value));
		alifDictValues_addToInsertionOrder(_mp->values, ix_);
		STORE_USED(_mp, _mp->used + 1);
	}
	else {
		_alifDict_notifyEvent(_interp, AlifDictWatchEvent_::AlifDict_Event_Modified, _mp, _key, _value);
		STORE_SPLIT_VALUE(_mp, ix_, ALIF_NEWREF(_value));
		ALIF_DECREF(oldValue);
	}
}

static AlifIntT insert_dict(AlifInterpreter* _interp, AlifDictObject* _mp,
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
		_alifDict_notifyEvent(
			_interp, AlifDictWatchEvent_::AlifDict_Event_Modified, _mp, _key, _value);
		if (DK_IS_USTR(_mp->keys)) {
			AlifDictUStrEntry* ep_ = &dk_uStrEntries(_mp->keys)[ix_];
			STORE_VALUE(ep_, _value);
		}
		else {
			AlifDictKeyEntry* ep_ = &dk_entries(_mp->keys)[ix_];
			STORE_VALUE(ep_, _value);
		}
	}
	ALIF_XDECREF(oldValue); /* which **CAN** re-enter */
	ALIF_DECREF(_key);
	return 0;

Fail:
	ALIF_DECREF(_value);
	ALIF_DECREF(_key);
	return -1;
}

static AlifIntT insertTo_emptyDict(AlifInterpreter* _interp, AlifDictObject* _mp,
	AlifObject* _key, AlifHashT _hash, AlifObject* _value) { // 1795

	AlifIntT uStr = ALIFUSTR_CHECKEXACT(_key);
	AlifDictKeysObject* newKeys = new_keysObject(
		_interp, ALIFDICT_LOG_MINSIZE, uStr);
	if (newKeys == nullptr) {
		ALIF_DECREF(_key);
		ALIF_DECREF(_value);
		return -1;
	}
	_alifDict_notifyEvent(_interp, AlifDictWatchEvent_::AlifDict_Event_Added, _mp, _key, _value);

	MAINTAIN_TRACKING(_mp, _key, _value);

	AlifUSizeT hasPos = (AlifUSizeT)_hash & (ALIFDICT_MINSIZE - 1);
	dictKeys_setIndex(newKeys, hasPos, 0);
	if (uStr) {
		AlifDictUStrEntry* ep_ = dk_uStrEntries(newKeys);
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
	newKeys->usable--;
	newKeys->nentries++;
	alifAtomic_storePtrRelease(&_mp->keys, newKeys);
	return 0;
}

static void build_indicesGeneric(AlifDictKeysObject* _keys,
	AlifDictKeyEntry* ep_, AlifSizeT _n) { // 1847
	AlifUSizeT mask = DK_MASK(_keys);
	for (AlifSizeT ix_ = 0; ix_ != _n; ix_++, ep_++) {
		AlifUSizeT hash = ep_->hash;
		AlifUSizeT i = hash & mask;
		for (AlifUSizeT perturb = hash; dictKeys_getIndex(_keys, i) != DKIX_EMPTY;) {
			perturb >>= PERTURB_SHIFT;
			i = mask & (i * 5 + perturb + 1);
		}
		dictKeys_setIndex(_keys, i, ix_);
	}
}

static void build_indicesUStr(AlifDictKeysObject* _keys,
	AlifDictUStrEntry* ep_, AlifSizeT _n) { // 1862
	AlifUSizeT mask = DK_MASK(_keys);
	for (AlifSizeT ix_ = 0; ix_ != _n; ix_++, ep_++) {
		AlifHashT hash = uStr_getHash(ep_->key);
		AlifUSizeT i = hash & mask;
		for (AlifUSizeT perturb = hash; dictKeys_getIndex(_keys, i) != DKIX_EMPTY;) {
			perturb >>= PERTURB_SHIFT;
			i = mask & (i * 5 + perturb + 1);
		}
		dictKeys_setIndex(_keys, i, ix_);
	}
}

static AlifIntT dict_resize(AlifInterpreter* _interp, AlifDictObject* _mp,
	uint8_t _log2NewSize, AlifIntT _uStr) { // 1892
	AlifDictKeysObject* oldkeys{}, * newkeys{};
	AlifDictValues* oldValues{};

	if (_log2NewSize >= SIZEOF_SIZE_T * 8) {
		//alifErr_noMemory();
		return -1;
	}

	oldkeys = _mp->keys;
	oldValues = _mp->values;

	if (!DK_IS_USTR(oldkeys)) {
		_uStr = 0;
	}

	ensureShared_onResize(_mp);

	newkeys = new_keysObject(_interp, _log2NewSize, _uStr);
	if (newkeys == nullptr) {
		return -1;
	}

	AlifSizeT numentries = _mp->used;

	if (oldValues != nullptr) {
		LOCK_KEYS(oldkeys);
		AlifDictUStrEntry* oldentries = dk_uStrEntries(oldkeys);

		if (newkeys->kind == Dict_Keys_General) {
			AlifDictKeyEntry* newentries = dk_entries(newkeys);

			for (AlifSizeT i = 0; i < numentries; i++) {
				AlifIntT index = getIndex_fromOrder(_mp, i);
				AlifDictUStrEntry* ep_ = &oldentries[index];
				newentries[i].key = ALIF_NEWREF(ep_->key);
				newentries[i].hash = uStr_getHash(ep_->key);
				newentries[i].value = oldValues->values[index];
			}
			build_indicesGeneric(newkeys, newentries, numentries);
		}
		else { // split -> combined uStr
			AlifDictUStrEntry* newentries = dk_uStrEntries(newkeys);

			for (AlifSizeT i = 0; i < numentries; i++) {
				AlifIntT index = getIndex_fromOrder(_mp, i);
				AlifDictUStrEntry* ep_ = &oldentries[index];
				newentries[i].key = ALIF_NEWREF(ep_->key);
				newentries[i].value = oldValues->values[index];
			}
			build_indicesUStr(newkeys, newentries, numentries);
		}
		UNLOCK_KEYS(oldkeys);
		set_keys(_mp, newkeys);
		dictKeys_decRef(_interp, oldkeys, IS_DICT_SHARED(_mp));
		set_values(_mp, nullptr);
		if (oldValues->embedded) {
			alifAtomic_storeUint8(&oldValues->valid, 0);
		}
		else {
			free_values(oldValues, IS_DICT_SHARED(_mp));
		}
	}
	else {  // oldkeys is combined.
		if (oldkeys->kind == DictKeysKind_::Dict_Keys_General) {
			// generic -> generic
			AlifDictKeyEntry* oldentries = dk_entries(oldkeys);
			AlifDictKeyEntry* newentries = dk_entries(newkeys);
			if (oldkeys->nentries == numentries) {
				memcpy(newentries, oldentries, numentries * sizeof(AlifDictKeyEntry));
			}
			else {
				AlifDictKeyEntry* ep_ = oldentries;
				for (AlifSizeT i = 0; i < numentries; i++) {
					while (ep_->value == nullptr)
						ep_++;
					newentries[i] = *ep_++;
				}
			}
			build_indicesGeneric(newkeys, newentries, numentries);
		}
		else {  // oldkeys is combined uStr
			AlifDictUStrEntry* oldentries = dk_uStrEntries(oldkeys);
			if (_uStr) { // combined uStr -> combined uStr
				AlifDictUStrEntry* newentries = dk_uStrEntries(newkeys);
				if (oldkeys->nentries == numentries and _mp->keys->kind == DictKeysKind_::Dict_Keys_UStr) {
					memcpy(newentries, oldentries, numentries * sizeof(AlifDictUStrEntry));
				}
				else {
					AlifDictUStrEntry* ep_ = oldentries;
					for (AlifSizeT i = 0; i < numentries; i++) {
						while (ep_->value == nullptr)
							ep_++;
						newentries[i] = *ep_++;
					}
				}
				build_indicesUStr(newkeys, newentries, numentries);
			}
			else { // combined uStr -> generic
				AlifDictKeyEntry* newentries = dk_entries(newkeys);
				AlifDictUStrEntry* ep_ = oldentries;
				for (AlifSizeT i = 0; i < numentries; i++) {
					while (ep_->value == nullptr)
						ep_++;
					newentries[i].key = ep_->key;
					newentries[i].hash = uStr_getHash(ep_->key);
					newentries[i].value = ep_->value;
					ep_++;
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

static AlifObject* dictNew_presized(AlifInterpreter* _interp, AlifSizeT _minused, bool _uStr) { // 2044
	const uint8_t log2MaxPresize = 17;
	const AlifSizeT maxPresize = ((AlifSizeT)1) << log2MaxPresize;
	uint8_t log2NewSize{};
	AlifDictKeysObject* newKeys{};

	if (_minused <= USABLE_FRACTION(ALIFDICT_MINSIZE)) {
		return alifDict_new();
	}
	if (_minused > USABLE_FRACTION(maxPresize)) {
		log2NewSize = log2MaxPresize;
	}
	else {
		log2NewSize = estimate_log2Keysize(_minused);
	}

	newKeys = new_keysObject(_interp, log2NewSize, _uStr);
	if (newKeys == nullptr)
		return nullptr;
	return new_dict(_interp, newKeys, nullptr, 0, 0);
}

AlifObject* _alifDict_newPresized(AlifSizeT _minused) { // 2072
	AlifInterpreter* interp = _alifInterpreter_get();
	return dictNew_presized(interp, _minused, false);
}

AlifObject* _alifDict_fromItems(AlifObject* const* _keys, AlifSizeT _keysOffset,
	AlifObject* const* _values, AlifSizeT _valuesOffset, AlifSizeT _length) { // 2079
	bool uStr = true;
	AlifObject* const* ks_ = _keys;
	AlifInterpreter* interp = _alifInterpreter_get();

	for (AlifSizeT i = 0; i < _length; i++) {
		if (!ALIFUSTR_CHECKEXACT(*ks_)) {
			uStr = false;
			break;
		}
		ks_ += _keysOffset;
	}

	AlifObject* dict = dictNew_presized(interp, _length, uStr);
	if (dict == nullptr) {
		return nullptr;
	}

	ks_ = _keys;
	AlifObject* const* vs_ = _values;

	for (AlifSizeT i = 0; i < _length; i++) {
		AlifObject* key = *ks_;
		AlifObject* value = *vs_;
		if (setItem_lockHeld((AlifDictObject*)dict, key, value) < 0) {
			ALIF_DECREF(dict);
			return nullptr;
		}
		ks_ += _keysOffset;
		vs_ += _valuesOffset;
	}

	return dict;
}


static AlifObject* dict_getItem(AlifObject* _op,
	AlifObject* _key, const char* _warnmsg) { // 2127
	if (!ALIFDICT_CHECK(_op)) {
		return nullptr;
	}
	AlifDictObject* mp = (AlifDictObject*)_op;

	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		//alifErr_formatUnraisable(warnmsg);
		return nullptr;
	}

	AlifThread* tstate = _alifThread_get();

	/* Preserve the existing exception */
	AlifObject* value{};
	AlifSizeT ix{}; (void)ix;

	//AlifObject* exc = _alifErr_getRaisedException(tstate);

	ix = alifDict_lookupThreadSafe(mp, _key, hash, &value);
	ALIF_XDECREF(value);


	/* Ignore any exception raised by the lookup */
	//AlifObject* exc2 = _alifErr_occurred(tstate);
	//if (exc2 and !alifErr_givenExceptionMatches(exc2, _alifExcKeyError_)) {
	//	alifErr_formatUnraisable(warnmsg);
	//}
	//_alifErr_setRaisedException(tstate, exc);

	return value;  // borrowed reference
}

AlifObject* alifDict_getItem(AlifObject* _op, AlifObject* _key) { // 2171
	return dict_getItem(_op, _key,
		"Exception ignored in alifDict_getItem(); consider using "
		"alifDict_getItemRef() or alifDict_getItemWithError()");
}


AlifObject* _alifDict_getItemKnownHash(AlifObject* _op,
	AlifObject* _key, AlifHashT _hash) { // 2200
	AlifSizeT ix{}; (void)ix;
	AlifDictObject* mp = (AlifDictObject*)_op;
	AlifObject* value;

	if (!ALIFDICT_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	ix = alifDict_lookupThreadSafe(mp, _key, _hash, &value);
	ALIF_XDECREF(value);
	return value;  // borrowed reference
}


AlifIntT alifDict_getItemRefKnownHash(AlifDictObject* _op, AlifObject* _key,
	AlifHashT _hash, AlifObject** _result) { // 2249
	AlifObject* value{};
	AlifSizeT ix_ = alifDict_lookupThreadSafe(_op, _key, _hash, &value);
	if (ix_ == DKIX_ERROR) {
		*_result = nullptr;
		return -1;
	}
	if (value == nullptr) {
		*_result = nullptr;
		return 0;  // missing key
	}
	*_result = value;
	return 1;  // key is present
}

AlifIntT alifDict_getItemRef(AlifObject* _op,
	AlifObject* _key, AlifObject** _result) { // 2275
	if (!ALIFDICT_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		*_result = nullptr;
		return -1;
	}

	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		*_result = nullptr;
		return -1;
	}

	return alifDict_getItemRefKnownHash((AlifDictObject*)_op, _key, hash, _result);
}

AlifObject* alifDict_getItemWithError(AlifObject* _op, AlifObject* _key) { // 2323
	AlifSizeT ix{}; (void)ix;
	AlifHashT hash{};
	AlifDictObject* mp = (AlifDictObject*)_op;
	AlifObject* value;

	if (!ALIFDICT_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return nullptr;
	}

	ix = alifDict_lookupThreadSafe(mp, _key, hash, &value);
	ALIF_XDECREF(value);
	return value;  // borrowed reference
}

AlifObject* _alifDict_getItemWithError(AlifObject* _dp, AlifObject* _kv) { // 2350
	AlifHashT hash = ALIF_TYPE(_kv)->hash(_kv);
	if (hash == -1) {
		return nullptr;
	}
	return _alifDict_getItemKnownHash(_dp, _kv, hash);  // borrowed reference
}

AlifObject* _alifDict_loadGlobal(AlifDictObject* _globals,
	AlifDictObject* _builtins, AlifObject* _key) { // 2398
	AlifSizeT ix{};
	AlifHashT hash{};
	AlifObject* value{};

	hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return nullptr;
	}

	/* namespace 1: globals */
	ix = alifDict_lookupThreadSafe(_globals, _key, hash, &value);
	if (ix == DKIX_ERROR)
		return nullptr;
	if (ix != DKIX_EMPTY and value != nullptr)
		return value;

	/* namespace 2: builtins */
	ix = alifDict_lookupThreadSafe(_builtins, _key, hash, &value);
	return value;
}

static AlifIntT setItemTake2_lockHeld(AlifDictObject* _mp,
	AlifObject* _key, AlifObject* _value) { // 2424
	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		ALIF_DECREF(_key);
		ALIF_DECREF(_value);
		return -1;
	}

	AlifInterpreter* interp = _alifInterpreter_get();

	if (_mp->keys == ALIF_EMPTY_KEYS) {
		return insertTo_emptyDict(interp, _mp, _key, hash, _value);
	}
	return insert_dict(interp, _mp, _key, hash, _value);
}

AlifIntT alifDict_setItemTake2(AlifDictObject* _mp,
	AlifObject* _key, AlifObject* _value) { // 2449
	AlifIntT res_{};
	ALIF_BEGIN_CRITICAL_SECTION(_mp);
	res_ = setItemTake2_lockHeld(_mp, _key, _value);
	ALIF_END_CRITICAL_SECTION();
	return res_;
}


AlifIntT alifDict_setItem(AlifObject* _op,
	AlifObject* _key, AlifObject* _value) { // 2465
	if (!ALIFDICT_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	return alifDict_setItemTake2((AlifDictObject*)_op,
		ALIF_NEWREF(_key), ALIF_NEWREF(_value));
}

void _alifDict_loadGlobalStackRef(AlifDictObject* _globals, AlifDictObject* _builtins,
	AlifObject* _key, AlifStackRef* _res) { // 2471
	AlifSizeT ix{};
	AlifHashT hash{};

	hash = alifObject_hashFast(_key);
	if (hash == -1) {
		*_res = _alifStackRefNull_;
		return;
	}

	/* namespace 1: globals */
	ix = _alifDict_lookupThreadSafeStackRef(_globals, _key, hash, _res);
	if (ix == DKIX_ERROR) {
		return;
	}
	if (ix != DKIX_EMPTY and !ALIFSTACKREF_ISNULL(*_res)) {
		return;
	}

	/* namespace 2: builtins */
	ix = _alifDict_lookupThreadSafeStackRef(_builtins, _key, hash, _res);
}

static AlifIntT setItem_lockHeld(AlifDictObject* _mp,
	AlifObject* _key, AlifObject* _value) { // 2477
	return setItemTake2_lockHeld(_mp, ALIF_NEWREF(_key), ALIF_NEWREF(_value));
}



static void deleteIndex_fromValues(AlifDictValues* _values, AlifSizeT _ix) { // 2518
	uint8_t* array = getInsertion_orderArray(_values);
	AlifIntT size = _values->size;
	AlifIntT i_{};
	//for (i_ = 0; array[i_] != _ix; i_++) {
	//	assert(i_ < size);
	//}
	size--;
	for (; i_ < size; i_++) {
		array[i_] = array[i_ + 1];
	}
	_values->size = size;
}


static void delItem_common(AlifDictObject* _mp, AlifHashT _hash, AlifSizeT _ix,
	AlifObject* _old_value) { // 2536
	AlifObject* oldKey{};

	AlifSizeT hashPos = lookDict_index(_mp->keys, _hash, _ix);

	STORE_USED(_mp, _mp->used - 1);
	if (ALIFDICT_HASSPLITTABLE(_mp)) {
		STORE_SPLIT_VALUE(_mp, _ix, nullptr);
		deleteIndex_fromValues(_mp->values, _ix);
	}
	else {
		_mp->keys->version = 0;
		dictKeys_setIndex(_mp->keys, hashPos, DKIX_DUMMY);
		if (DK_IS_USTR(_mp->keys)) {
			AlifDictUStrEntry* ep = &dk_uStrEntries(_mp->keys)[_ix];
			oldKey = ep->key;
			STORE_KEY(ep, nullptr);
			STORE_VALUE(ep, nullptr);
		}
		else {
			AlifDictKeyEntry* ep = &dk_entries(_mp->keys)[_ix];
			oldKey = ep->key;
			STORE_KEY(ep, nullptr);
			STORE_VALUE(ep, nullptr);
			STORE_HASH(ep, 0);
		}
		ALIF_DECREF(oldKey);
	}
	ALIF_DECREF(_old_value);
}


AlifIntT alifDict_delItem(AlifObject* _op, AlifObject* _key) { // 2580
	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return -1;
	}

	return alifDict_delItemKnownHash(_op, _key, hash);
}


static AlifIntT delItem_knownHashLockHeld(AlifObject* _op,
	AlifObject* _key, AlifHashT _hash) { // 2592
	AlifSizeT ix_{};
	AlifDictObject* mp_{};
	AlifObject* oldValue{};

	if (!ALIFDICT_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	mp_ = (AlifDictObject*)_op;
	ix_ = alifDict_lookup(mp_, _key, _hash, &oldValue);
	if (ix_ == DKIX_ERROR)
		return -1;
	if (ix_ == DKIX_EMPTY or oldValue == nullptr) {
		//alifErr_setKeyError(key);
		return -1;
	}

	AlifInterpreter* interp = _alifInterpreter_get();
	_alifDict_notifyEvent(interp, AlifDictWatchEvent_::AlifDict_Event_Deleted, mp_, _key, nullptr);
	delItem_common(mp_, _hash, ix_, oldValue);
	return 0;
}


AlifIntT alifDict_delItemKnownHash(AlifObject* _op,
	AlifObject* _key, AlifHashT _hash) { // 2624
	AlifIntT res{};
	ALIF_BEGIN_CRITICAL_SECTION(_op);
	res = delItem_knownHashLockHeld(_op, _key, _hash);
	ALIF_END_CRITICAL_SECTION();
	return res;
}

AlifIntT _alifDict_next(AlifObject* _op, AlifSizeT* _ppos, AlifObject** _pKey,
	AlifObject** _pValue, AlifHashT* _pHash) { // 2755
	AlifSizeT i{};
	AlifDictObject* mp{};
	AlifObject* key{}, * value{};
	AlifHashT hash{};

	if (!ALIFDICT_CHECK(_op))
		return 0;

	mp = (AlifDictObject*)_op;
	i = *_ppos;
	if (ALIFDICT_HASSPLITTABLE(mp)) {
		if (i < 0 or i >= mp->used)
			return 0;
		AlifIntT index = getIndex_fromOrder(mp, i);
		value = mp->values->values[index];
		key = (AlifObject*)LOAD_SHARED_KEY(dk_uStrEntries(mp->keys)[index].key);
		hash = uStr_getHash(key);
	}
	else {
		AlifSizeT n = mp->keys->nentries;
		if (i < 0 or i >= n)
			return 0;
		if (DK_IS_USTR(mp->keys)) {
			AlifDictUStrEntry* entryPtr = &dk_uStrEntries(mp->keys)[i];
			while (i < n and entryPtr->value == nullptr) {
				entryPtr++;
				i++;
			}
			if (i >= n)
				return 0;
			key = entryPtr->key;
			hash = uStr_getHash(entryPtr->key);
			value = entryPtr->value;
		}
		else {
			AlifDictKeyEntry* entryPtr = &dk_entries(mp->keys)[i];
			while (i < n and entryPtr->value == nullptr) {
				entryPtr++;
				i++;
			}
			if (i >= n)
				return 0;
			key = entryPtr->key;
			hash = entryPtr->hash;
			value = entryPtr->value;
		}
	}
	*_ppos = i + 1;
	if (_pKey)
		*_pKey = key;
	if (_pValue)
		*_pValue = value;
	if (_pHash)
		*_pHash = hash;
	return 1;
}

AlifIntT alifDict_next(AlifObject* _op, AlifSizeT* _pPos, AlifObject** _pKey, AlifObject** _pValue) { // 2836
	return _alifDict_next(_op, _pPos, _pKey, _pValue, nullptr);
}


AlifIntT alifDict_popKnownHash(AlifDictObject* _mp, AlifObject* _key,
	AlifHashT _hash, AlifObject** _result) { // 2843

	if (_mp->used == 0) {
		if (_result) {
			*_result = nullptr;
		}
		return 0;
	}

	AlifObject* oldValue{};
	AlifSizeT ix = alifDict_lookup(_mp, _key, _hash, &oldValue);
	if (ix == DKIX_ERROR) {
		if (_result) {
			*_result = nullptr;
		}
		return -1;
	}

	if (ix == DKIX_EMPTY or oldValue == nullptr) {
		if (_result) {
			*_result = nullptr;
		}
		return 0;
	}

	AlifInterpreter* interp = _alifInterpreter_get();
	_alifDict_notifyEvent(interp, AlifDictWatchEvent_::AlifDict_Event_Deleted, _mp, _key, nullptr);
	delItem_common(_mp, _hash, ix, ALIF_NEWREF(oldValue));

	if (_result) {
		*_result = oldValue;
	}
	else {
		ALIF_DECREF(oldValue);
	}
	return 1;
}

static AlifIntT pop_lockHeld(AlifObject* _op,
	AlifObject* _key, AlifObject** _result) { // 2890
	if (!ALIFDICT_CHECK(_op)) {
		if (_result) {
			*_result = nullptr;
		}
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	AlifDictObject* dict = (AlifDictObject*)_op;

	if (dict->used == 0) {
		if (_result) {
			*_result = nullptr;
		}
		return 0;
	}

	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		if (_result) {
			*_result = nullptr;
		}
		return -1;
	}
	return alifDict_popKnownHash(dict, _key, hash, _result);
}



AlifIntT alifDict_pop(AlifObject* _op,
	AlifObject* _key, AlifObject** _result) { // 2921
	AlifIntT err{};
	ALIF_BEGIN_CRITICAL_SECTION(_op);
	err = pop_lockHeld(_op, _key, _result);
	ALIF_END_CRITICAL_SECTION();

	return err;
}


AlifIntT alifDict_popString(AlifObject* _op,
	const char* _key, AlifObject** _result) { // 2933
	AlifObject* keyObj = alifUStr_fromString(_key);
	if (keyObj == nullptr) {
		if (_result != nullptr) {
			*_result = nullptr;
		}
		return -1;
	}

	AlifIntT res = alifDict_pop(_op, keyObj, _result);
	ALIF_DECREF(keyObj);
	return res;
}





static void dict_dealloc(AlifObject* _self) { // 3089
	AlifDictObject* mp = (AlifDictObject*)_self;
	AlifInterpreter* interp = _alifInterpreter_get();
	ALIF_SET_REFCNT(mp, 1);
	_alifDict_notifyEvent(interp, AlifDictWatchEvent_::AlifDict_Event_Deallocated, mp, nullptr, nullptr);
	if (ALIF_REFCNT(mp) > 1) {
		ALIF_SET_REFCNT(mp, ALIF_REFCNT(mp) - 1);
		return;
	}
	ALIF_SET_REFCNT(mp, 0);
	AlifDictValues* values = mp->values;
	AlifDictKeysObject* keys = mp->keys;
	AlifSizeT i{}, n{};

	alifObject_gcUnTrack(mp);
	ALIF_TRASHCAN_BEGIN(mp, dict_dealloc)
		if (values != nullptr) {
			if (values->embedded == 0) {
				for (i = 0, n = mp->keys->nentries; i < n; i++) {
					ALIF_XDECREF(values->values[i]);
				}
				free_values(values, false);
			}
			dictKeys_decRef(interp, keys, false);
		}
		else if (keys != nullptr) {
			dictKeys_decRef(interp, keys, false);
		}
	if (ALIF_IS_TYPE(mp, &_alifDictType_)) {
		ALIF_FREELIST_FREE(dicts, DICTS, mp, ALIF_TYPE(mp)->free);
	}
	else {
		ALIF_TYPE(mp)->free((AlifObject*)mp);
	}
	ALIF_TRASHCAN_END
}


static AlifObject* dictRepr_lockHeld(AlifObject* _self) { // 3132
	AlifDictObject* mp = (AlifDictObject*)_self;
	AlifObject* key = nullptr, * value = nullptr;

	AlifSizeT i = 0; //* alif
	AlifIntT first = 1;

	AlifIntT res = alif_reprEnter((AlifObject*)mp);
	if (res != 0) {
		return (res > 0 ? alifUStr_fromString("{...}") : nullptr);
	}

	if (mp->used == 0) {
		alif_reprLeave((AlifObject*)mp);
		return alifUStr_fromString("{}");
	}

	// "{" + "1: 2" + ", 3: 4" * (len - 1) + "}"
	AlifSizeT prealloc = 1 + 4 + 6 * (mp->used - 1) + 1;
	AlifUStrWriter* writer = alifUStrWriter_create(prealloc);
	if (writer == nullptr) {
		goto error;
	}

	if (alifUStrWriter_writeChar(writer, '{') < 0) {
		goto error;
	}

	/* Do repr() on each key+value pair, and insert ": " between them.
	   Note that repr may mutate the dict. */
	i = 0;
	first = 1;
	while (_alifDict_next((AlifObject*)mp, &i, &key, &value, nullptr)) {
		// Prevent repr from deleting key or value during key format.
		ALIF_INCREF(key);
		ALIF_INCREF(value);

		if (!first) {
			// Write ", "
			if (alifUStrWriter_writeChar(writer, ',') < 0) {
				goto error;
			}
			if (alifUStrWriter_writeChar(writer, ' ') < 0) {
				goto error;
			}
		}
		first = 0;

		// Write repr(key)
		if (alifUStrWriter_writeRepr(writer, key) < 0) {
			goto error;
		}

		// Write ": "
		if (alifUStrWriter_writeChar(writer, ':') < 0) {
			goto error;
		}
		if (alifUStrWriter_writeChar(writer, ' ') < 0) {
			goto error;
		}

		// Write repr(value)
		if (alifUStrWriter_writeRepr(writer, value) < 0) {
			goto error;
		}

		ALIF_CLEAR(key);
		ALIF_CLEAR(value);
	}

	if (alifUStrWriter_writeChar(writer, '}') < 0) {
		goto error;
	}

	alif_reprLeave((AlifObject*)mp);

	return alifUStrWriter_finish(writer);

error:
	alif_reprLeave((AlifObject*)mp);
	alifUStrWriter_discard(writer);
	ALIF_XDECREF(key);
	ALIF_XDECREF(value);
	return nullptr;
}

static AlifObject* dict_repr(AlifObject* _self) { // 3218
	AlifObject* res{};
	ALIF_BEGIN_CRITICAL_SECTION(_self);
	res = dictRepr_lockHeld(_self);
	ALIF_END_CRITICAL_SECTION();
	return res;
}


static AlifSizeT dict_length(AlifObject* _self) { // 3228
	return alifAtomic_loadSizeRelaxed(&((AlifDictObject*)_self)->used);
}

static AlifObject* dict_subscript(AlifObject* _self, AlifObject* _key) { // 3234
	AlifDictObject* mp = (AlifDictObject*)_self;
	AlifSizeT ix{};
	AlifHashT hash{};
	AlifObject* value{};

	hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return nullptr;
	}
	ix = alifDict_lookupThreadSafe(mp, _key, hash, &value);
	if (ix == DKIX_ERROR)
		return nullptr;
	if (ix == DKIX_EMPTY or value == nullptr) {
		if (!ALIFDICT_CHECKEXACT(mp)) {
			AlifObject* missing{}, * res{};
			//missing = _alifObject_lookupSpecial(
			//	(AlifObject*)mp, &ALIF_ID(__missing__));
			//if (missing != nullptr) {
			//	res = AlifObject_callOneArg(missing, key);
			//	ALIF_DECREF(missing);
			//	return res;
			//}
			//else if (alifErr_occurred())
			//	return nullptr;
		}
		//_alifErr_setKeyError(key);
		return nullptr;
	}
	return value;
}

static AlifIntT dict_assSub(AlifObject* _mp, AlifObject* _v, AlifObject* _w) { // 3269
	if (_w == nullptr)
		return alifDict_delItem(_mp, _v);
	else
		return alifDict_setItem(_mp, _v, _w);
}

static AlifMappingMethods _dictAsMapping_ = { // 3278
	.length = dict_length,
	.subscript = dict_subscript,
	.assSubscript = dict_assSub,
};


static AlifObject* keys_lockHeld(AlifObject* _dict) { // 3284
	if (_dict == nullptr or !ALIFDICT_CHECK(_dict)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	AlifDictObject* mp = (AlifDictObject*)_dict;
	AlifObject* v{};
	AlifSizeT n{};

again:
	n = mp->used;
	v = alifList_new(n);
	if (v == nullptr) return nullptr;
	if (n != mp->used) {
		ALIF_DECREF(v);
		goto again;
	}

	AlifSizeT j = 0, pos = 0;
	AlifObject* key{};
	while (_alifDict_next((AlifObject*)mp, &pos, &key, nullptr, nullptr)) {
		ALIFLIST_SET_ITEM(v, j, ALIF_NEWREF(key));
		j++;
	}
	return v;
}

AlifObject* alifDict_keys(AlifObject* _dict) { // 3322
	AlifObject* res{};
	ALIF_BEGIN_CRITICAL_SECTION(_dict);
	res = keys_lockHeld(_dict);
	ALIF_END_CRITICAL_SECTION();

	return res;
}


static AlifIntT dict_dictMerge(AlifInterpreter* interp, AlifDictObject* mp, AlifDictObject* other, AlifIntT override) { // 3619
	if (other == mp or other->used == 0)
		return 0;
	if (mp->used == 0) {
		override = 1;
		AlifDictKeysObject* okeys = other->keys;

		if (mp->values == nullptr and
			other->values == nullptr and
			other->used == okeys->nentries and
			(DK_LOG_SIZE(okeys) == ALIFDICT_LOG_MINSIZE or
				USABLE_FRACTION(DK_SIZE(okeys) / 2) < other->used)
			) {
			_alifDict_notifyEvent(interp, AlifDictWatchEvent_::AlifDict_Event_Cloned,
				mp, (AlifObject*)other, nullptr);
			AlifDictKeysObject* keys = cloneCombined_dictKeys(other);
			if (keys == nullptr)
				return -1;

			ensureShared_onResize(mp);
			dictKeys_decRef(interp, mp->keys, IS_DICT_SHARED(mp));
			mp->keys = keys;
			STORE_USED(mp, other->used);

			if (ALIFOBJECT_GC_IS_TRACKED(other) and !ALIFOBJECT_GC_IS_TRACKED(mp)) {
				/* Maintain tracking. */
				ALIFOBJECT_GC_TRACK(mp);
			}

			return 0;
		}
	}

	if (USABLE_FRACTION(DK_SIZE(mp->keys)) < other->used) {
		AlifIntT unicode = DK_IS_USTR(other->keys);
		if (dict_resize(interp, mp,
			estimate_log2Keysize(mp->used + other->used),
			unicode)) {
			return -1;
		}
	}

	AlifSizeT orig_size = other->keys->nentries;
	AlifSizeT pos = 0;
	AlifHashT hash{};
	AlifObject* key{}, * value{};

	while (_alifDict_next((AlifObject*)other, &pos, &key, &value, &hash)) {
		AlifIntT err = 0;
		ALIF_INCREF(key);
		ALIF_INCREF(value);
		if (override == 1) {
			err = insert_dict(interp, mp,
				ALIF_NEWREF(key), hash, ALIF_NEWREF(value));
		}
		else {
			err = alifDict_containsKnownHash((AlifObject*)mp, key, hash);
			if (err == 0) {
				err = insert_dict(interp, mp,
					ALIF_NEWREF(key), hash, ALIF_NEWREF(value));
			}
			else if (err > 0) {
				if (override != 0) {
					//_alifErr_setKeyError(key);
					ALIF_DECREF(value);
					ALIF_DECREF(key);
					return -1;
				}
				err = 0;
			}
		}
		ALIF_DECREF(value);
		ALIF_DECREF(key);
		if (err != 0)
			return -1;

		if (orig_size != other->keys->nentries) {
			//alifErr_setString(_alifExcRuntimeError_,
			//	"dict mutated during update");
			return -1;
		}
	}
	return 0;
}

static AlifIntT dict_merge(AlifInterpreter* interp,
	AlifObject* a, AlifObject* b, AlifIntT override) { // 3720
	AlifDictObject* mp{}, * other{};

	if (a == nullptr or !ALIFDICT_CHECK(a) or b == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	mp = (AlifDictObject*)a;
	int res = 0;
	if (ALIFDICT_CHECK(b) and (ALIF_TYPE(b)->iter == dict_iter)) {
		other = (AlifDictObject*)b;
		int res;
		ALIF_BEGIN_CRITICAL_SECTION2(a, b);
		res = dict_dictMerge(interp, (AlifDictObject*)a, other, override);
		ALIF_END_CRITICAL_SECTION2();
		return res;
	}
	else {
		/* Do it the generic, slower way */
		ALIF_BEGIN_CRITICAL_SECTION(a);
		AlifObject* keys = alifMapping_keys(b);
		AlifObject* iter{};
		AlifObject* key{}, * value{};
		AlifIntT status{};

		if (keys == nullptr) {
			res = -1;
			goto slow_exit;
		}

		iter = alifObject_getIter(keys);
		ALIF_DECREF(keys);
		if (iter == nullptr) {
			res = -1;
			goto slow_exit;
		}

		for (key = alifIter_next(iter); key; key = alifIter_next(iter)) {
			if (override != 1) {
				status = alifDict_contains(a, key);
				if (status != 0) {
					if (status > 0) {
						if (override == 0) {
							ALIF_DECREF(key);
							continue;
						}
						//_alifErr_setKeyError(key);
					}
					ALIF_DECREF(key);
					ALIF_DECREF(iter);
					res = -1;
					goto slow_exit;
				}
			}
			value = alifObject_getItem(b, key);
			if (value == nullptr) {
				ALIF_DECREF(iter);
				ALIF_DECREF(key);
				res = -1;
				goto slow_exit;
			}
			status = setItem_lockHeld(mp, key, value);
			ALIF_DECREF(key);
			ALIF_DECREF(value);
			if (status < 0) {
				ALIF_DECREF(iter);
				res = -1;
				goto slow_exit;
				return -1;
			}
		}
		ALIF_DECREF(iter);
		//if (alifErr_occurred()) {
		//	/* Iterator completed, via error */
		//	res = -1;
		//	goto slow_exit;
		//}

	slow_exit:
		ALIF_END_CRITICAL_SECTION();
		return res;
	}
}


AlifIntT alifDict_update(AlifObject* _a, AlifObject* _b) { // 3881
	AlifInterpreter* interp = _alifInterpreter_get();
	return dict_merge(interp, _a, _b, 1);
}


static AlifDictValues* copy_values(AlifDictValues* values) { // 3857
	AlifDictValues* newvalues = new_values(values->capacity);
	if (newvalues == nullptr) {
		return nullptr;
	}
	newvalues->size = values->size;
	uint8_t* valuesOrder = getInsertion_orderArray(values);
	uint8_t* newValuesOrder = getInsertion_orderArray(newvalues);
	memcpy(newValuesOrder, valuesOrder, values->capacity);
	for (AlifIntT i = 0; i < values->capacity; i++) {
		newvalues->values[i] = values->values[i];
	}
	return newvalues;
}

static AlifObject* copy_lockHeld(AlifObject* o) { // 3876
	AlifObject* copy{};
	AlifDictObject* mp{};
	AlifInterpreter* interp = _alifInterpreter_get();

	mp = (AlifDictObject*)o;
	if (mp->used == 0) {
		/* The dict is empty; just return a new dict. */
		return alifDict_new();
	}

	if (ALIFDICT_HASSPLITTABLE(mp)) {
		AlifDictObject* split_copy{};
		AlifDictValues* newvalues = copy_values(mp->values);
		if (newvalues == nullptr) {
			//return alifErr_noMemory();
		}
		split_copy = ALIFOBJECT_GC_NEW(AlifDictObject, &_alifDictType_);
		if (split_copy == nullptr) {
			free_values(newvalues, false);
			return nullptr;
		}
		for (size_t i = 0; i < newvalues->capacity; i++) {
			ALIF_XINCREF(newvalues->values[i]);
		}
		split_copy->values = newvalues;
		split_copy->keys = mp->keys;
		split_copy->used = mp->used;
		split_copy->watcherTag = 0;
		dictKeys_incRef(mp->keys);
		if (ALIFOBJECT_GC_IS_TRACKED(mp))
			ALIFOBJECT_GC_TRACK(split_copy);
		return (AlifObject*)split_copy;
	}

	if (ALIF_TYPE(mp)->iter == dict_iter and
		mp->values == nullptr and
		(mp->used >= (mp->keys->nentries * 2) / 3))
	{
		AlifDictKeysObject* keys = cloneCombined_dictKeys(mp);
		if (keys == nullptr) {
			return nullptr;
		}
		AlifDictObject* new_ = (AlifDictObject*)new_dict(interp, keys, nullptr, 0, 0);
		if (new_ == nullptr) {
			return nullptr;
		}

		new_->used = mp->used;
		if (ALIFOBJECT_GC_IS_TRACKED(mp)) {
			/* Maintain tracking. */
			ALIFOBJECT_GC_TRACK(new_);
		}

		return (AlifObject*)new_;
	}

	copy = alifDict_new();
	if (copy == nullptr)
		return nullptr;
	if (dict_merge(interp, copy, o, 1) == 0)
		return copy;
	ALIF_DECREF(copy);
	return nullptr;
}

AlifObject* alifDict_copy(AlifObject* _o) { // 3963
	if (_o == nullptr or !ALIFDICT_CHECK(_o)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	AlifObject* res{};
	ALIF_BEGIN_CRITICAL_SECTION(_o);

	res = copy_lockHeld(_o);

	ALIF_END_CRITICAL_SECTION();
	return res;
}

AlifSizeT alifDict_size(AlifObject* _mp) { // 3980
	if (_mp == nullptr or !ALIFDICT_CHECK(_mp)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	return alifAtomic_loadSizeRelaxed(&((AlifDictObject*)_mp)->used);
}

static AlifIntT dictSetDefault_refLockHeld(AlifObject* _d, AlifObject* _key, AlifObject* _defaultValue,
	AlifObject** _result, AlifIntT _incRefResult) { // 4145
	AlifDictObject* mp_ = (AlifDictObject*)_d;
	AlifObject* value{};
	AlifHashT hash{};
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifSizeT ix_{};

	if (!ALIFDICT_CHECK(_d)) {
		//ALIFERR_BADINTERNALCALL();
		if (_result) {
			*_result = nullptr;
		}
		return -1;
	}

	hash = alifObject_hashFast(_key);
	if (hash == -1) {
		if (_result) {
			*_result = nullptr;
		}
		return -1;
	}

	if (mp_->keys == ALIF_EMPTY_KEYS) {
		if (insertTo_emptyDict(interp, mp_, ALIF_NEWREF(_key), hash,
			ALIF_NEWREF(_defaultValue)) < 0) {
			if (_result) {
				*_result = nullptr;
			}
			return -1;
		}
		if (_result) {
			*_result = _incRefResult ? ALIF_NEWREF(_defaultValue) : _defaultValue;
		}
		return 0;
	}

	if (!ALIFUSTR_CHECKEXACT(_key) and DK_IS_USTR(mp_->keys)) {
		if (insertion_resize(interp, mp_, 0) < 0) {
			if (_result) {
				*_result = nullptr;
			}
			return -1;
		}
	}

	if (ALIFDICT_HASSPLITTABLE(mp_)) {
		AlifSizeT ix_ = insert_splitKey(mp_->keys, _key, hash);
		if (ix_ != DKIX_EMPTY) {
			AlifObject* value = mp_->values->values[ix_];
			AlifIntT alreadyPresent = value != nullptr;
			if (!alreadyPresent) {
				insert_splitValue(interp, mp_, _key, _defaultValue, ix_);
				value = _defaultValue;
			}
			if (_result) {
				*_result = _incRefResult ? ALIF_NEWREF(value) : value;
			}
			return alreadyPresent;
		}

		if (insertion_resize(interp, mp_, 1) < 0) {
			goto error;
		}
	}


	ix_ = alifDict_lookup(mp_, _key, hash, &value);
	if (ix_ == DKIX_ERROR) {
		if (_result) {
			*_result = nullptr;
		}
		return -1;
	}

	if (ix_ == DKIX_EMPTY) {
		value = _defaultValue;

		if (insert_combinedDict(interp, mp_, hash, ALIF_NEWREF(_key), ALIF_NEWREF(value)) < 0) {
			ALIF_DECREF(_key);
			ALIF_DECREF(value);
			if (_result) {
				*_result = nullptr;
			}
		}

		MAINTAIN_TRACKING(mp_, _key, value);
		STORE_USED(mp_, mp_->used + 1);
		if (_result) {
			*_result = _incRefResult ? ALIF_NEWREF(value) : value;
		}
		return 0;
	}

	if (_result) {
		*_result = _incRefResult ? ALIF_NEWREF(value) : value;
	}
	return 1;

error:
	if (_result) {
		*_result = nullptr;
	}
	return -1;
}

AlifIntT alifDict_setDefaultRef(AlifObject* _d, AlifObject* _key,
	AlifObject* _defaultValue, AlifObject** _result) { // 4262
	AlifIntT res{};
	ALIF_BEGIN_CRITICAL_SECTION(_d);
	res = dictSetDefault_refLockHeld(_d, _key, _defaultValue, _result, 1);
	ALIF_END_CRITICAL_SECTION();
	return res;
}

static AlifObject* dictIter_new(AlifDictObject*, AlifTypeObject*); // 4480



AlifUSizeT _alifDict_keysSize(AlifDictKeysObject* _keys) { // 4509
	AlifUSizeT es = (_keys->kind == DictKeysKind_::Dict_Keys_General
		? sizeof(AlifDictKeyEntry) : sizeof(AlifDictUStrEntry));
	AlifUSizeT size = sizeof(AlifDictKeysObject);
	size += (AlifUSizeT)1 << _keys->log2IndexBytes;
	size += USABLE_FRACTION((AlifUSizeT)DK_SIZE(_keys)) * es;
	return size;
}



//static AlifObject* dict_or(AlifObject* _self, AlifObject* _other) { // 4533
//	if (!ALIFDICT_CHECK(_self) or !ALIFDICT_CHECK(_other)) {
//		return ALIF_NOTIMPLEMENTED;
//	}
//	AlifObject* new_ = alifDict_copy(_self);
//	if (new_ == nullptr) {
//		return nullptr;
//	}
//	if (dict_updateArg(new_, _other)) {
//		ALIF_DECREF(new_);
//		return nullptr;
//	}
//	return new_;
//}
//
//static AlifObject* dict_ior(AlifObject* _self, AlifObject* _other) { // 4550
//	if (dict_updateArg(_self, _other)) {
//		return nullptr;
//	}
//	return ALIF_NEWREF(_self);
//}


static AlifMethodDef _dictMethods_[] = { // 4570
	DICT_KEYS_METHODDEF
	//DICT_ITEMS_METHODDEF
	DICT_VALUES_METHODDEF
	{nullptr, nullptr}   /* sentinel */
};


AlifIntT alifDict_contains(AlifObject* _op, AlifObject* _key) { // 4593
	AlifHashT hash = alifObject_hashFast(_key);

	if (hash == -1) {
		return -1;
	}

	return alifDict_containsKnownHash(_op, _key, hash);
}

AlifIntT alifDict_containsString(AlifObject* _op, const char* _key) { // 4605
	AlifObject* keyObj = alifUStr_fromString(_key);
	if (keyObj == nullptr) {
		return -1;
	}
	AlifIntT res = alifDict_contains(_op, keyObj);
	ALIF_DECREF(keyObj);
	return res;
}

AlifIntT alifDict_containsKnownHash(AlifObject* _op,
	AlifObject* _key, AlifHashT _hash) { // 4618
	AlifDictObject* mp = (AlifDictObject*)_op;
	AlifObject* value{};
	AlifSizeT ix_{};

	ix_ = alifDict_lookupThreadSafe(mp, _key, _hash, &value);
	if (ix_ == DKIX_ERROR)
		return -1;
	if (ix_ != DKIX_EMPTY and value != nullptr) {
		ALIF_DECREF(value);
		return 1;
	}
	return 0;
}


static AlifSequenceMethods _dictAsSequence_ = { // 4652
	.length = 0,
	.concat = 0,
	.repeat = 0,
	.item = 0,
	.wasSlice = 0,
	.assItem = 0,
	.wasAssSlice = 0,
	.contains = alifDict_contains,
	.inplaceConcat = 0,
	.inplaceRepeat = 0,
};

//static AlifNumberMethods _dictAsNumber_ = { // 4665
//	.or_ = dict_or,
//	.inplaceOr = dict_ior,
//};


static AlifObject* dict_iter(AlifObject* _self) { // 4742
	AlifDictObject* dict = (AlifDictObject*)_self;
	return dictIter_new(dict, &_alifDictIterKeyType_);
}


AlifTypeObject _alifDictType_ = { // 4760
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "فهرس",
	.basicSize = sizeof(AlifDictObject),
	.dealloc = dict_dealloc,
	.repr = dict_repr,
	.asSequence = &_dictAsSequence_,
	.asMapping = &_dictAsMapping_,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_DICT_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF | ALIF_TPFLAGS_MAPPING,
	.iter = dict_iter,
	.methods = _dictMethods_,
	//.init = dict_init,
	.alloc = alifType_allocNoTrack,
	//.new_ = dict_new,
	.free = alifObject_gcDel,
};


AlifIntT alifDict_getItemStringRef(AlifObject* _v,
	const char* _key, AlifObject** _result) { // 4826
	AlifObject* keyObj = alifUStr_fromString(_key);
	if (keyObj == nullptr) {
		*_result = nullptr;
		return -1;
	}
	AlifIntT res = alifDict_getItemRef(_v, keyObj, _result);
	ALIF_DECREF(keyObj);
	return res;
}

AlifIntT alifDict_delItemString(AlifObject* _v, const char* _key) { // 4873
	AlifObject* kv_{};
	AlifIntT err_{};
	kv_ = alifUStr_fromString(_key);
	if (kv_ == nullptr)
		return -1;
	err_ = alifDict_delItem(_v, kv_);
	ALIF_DECREF(kv_);
	return err_;
}

AlifIntT alifDict_setItemString(AlifObject* _v,
	const char* _key, AlifObject* _item) { // 4848
	AlifObject* kv{};
	AlifIntT err{};
	kv = alifUStr_fromString(_key);
	if (kv == nullptr) return -1;
	AlifInterpreter* interp = _alifInterpreter_get();
	alifUStr_internImmortal(interp, &kv);
	err = alifDict_setItem(_v, kv, _item);
	ALIF_DECREF(kv);
	return err;
}

class DictIterObject { // 4887
public:
	ALIFOBJECT_HEAD{};
	AlifDictObject* dict{}; /* Set to nullptr when iterator is exhausted */
	AlifSizeT used{};
	AlifSizeT pos{};
	AlifObject* result{}; /* reusable result tuple for iteritems */
	AlifSizeT len{};
};

static AlifObject* dictIter_new(AlifDictObject* _dict,
	AlifTypeObject* _itertype) { // 4896
	AlifSizeT used{};
	DictIterObject* di{};
	di = ALIFOBJECT_GC_NEW(DictIterObject, _itertype);
	if (di == nullptr) {
		return nullptr;
	}
	di->dict = (AlifDictObject*)ALIF_NEWREF(_dict);
	used = alifAtomic_loadSizeRelaxed(&_dict->used);
	di->used = used;
	di->len = used;
	if (_itertype == &_alifDictRevIterKeyType_ or
		_itertype == &_alifDictRevIterItemType_ or
		_itertype == &_alifDictRevIterValueType_) {
		if (ALIFDICT_HASSPLITTABLE(_dict)) {
			di->pos = used - 1;
		}
		else {
			di->pos = load_keysNentries(_dict) - 1;
		}
	}
	else {
		di->pos = 0;
	}
	if (_itertype == &_alifDictIterItemType_ or
		_itertype == &_alifDictRevIterItemType_) {
		di->result = alifTuple_pack(2, ALIF_NONE, ALIF_NONE);
		if (di->result == nullptr) {
			ALIF_DECREF(di);
			return nullptr;
		}
	}
	else {
		di->result = nullptr;
	}
	ALIFOBJECT_GC_TRACK(di);
	return (AlifObject*)di;
}



static AlifIntT dictIter_iterNextThreadSafe(AlifDictObject*, AlifObject*,
	AlifObject**, AlifObject**); // 4986



static AlifObject* dictIter_iterNextKey(AlifObject* self) { // 5060
	DictIterObject* di = (DictIterObject*)self;
	AlifDictObject* d = di->dict;

	if (d == nullptr)
		return nullptr;

	AlifObject* value{};
	if (dictIter_iterNextThreadSafe(d, self, &value, nullptr) < 0) {
		value = nullptr;
	}

	return value;
}


AlifTypeObject _alifDictIterKeyType_ = { // 5081
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "فهرس_تكرار_المفاتيح",
	.basicSize = sizeof(DictIterObject),
	/* methods */
	//.dealloc = dictIter_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = dictIter_traverse,
	.iter = alifObject_selfIter, 
	.iterNext = dictIter_iterNextKey,
	//.methods = dictIter_methods,
};





static AlifIntT dictIter_iterNextItemLockHeld(AlifDictObject* d, AlifObject* self,
	AlifObject** out_key, AlifObject** out_value) { // 5237
	DictIterObject* di = (DictIterObject*)self;
	AlifObject* key{}, * value{};
	AlifSizeT i{};

	if (di->used != d->used) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"dictionary changed size during iteration");
		di->used = -1; /* Make this state sticky */
		return -1;
	}

	i = alifAtomic_loadSizeRelaxed(&di->pos);

	if (ALIFDICT_HASSPLITTABLE(d)) {
		if (i >= d->used)
			goto fail;
		AlifIntT index = getIndex_fromOrder(d, i);
		key = (AlifObject*)LOAD_SHARED_KEY(dk_uStrEntries(d->keys)[index].key);
		value = d->values->values[index];
	}
	else {
		AlifSizeT n = d->keys->nentries;
		if (DK_IS_USTR(d->keys)) {
			AlifDictUStrEntry* entry_ptr = &dk_uStrEntries(d->keys)[i];
			while (i < n and entry_ptr->value == nullptr) {
				entry_ptr++;
				i++;
			}
			if (i >= n)
				goto fail;
			key = entry_ptr->key;
			value = entry_ptr->value;
		}
		else {
			AlifDictKeyEntry* entry_ptr = &dk_entries(d->keys)[i];
			while (i < n and entry_ptr->value == nullptr) {
				entry_ptr++;
				i++;
			}
			if (i >= n)
				goto fail;
			key = entry_ptr->key;
			value = entry_ptr->value;
		}
	}
	// We found an element, but did not expect it
	if (di->len == 0) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"dictionary keys changed during iteration");
		goto fail;
	}
	di->pos = i + 1;
	di->len--;
	if (out_key != nullptr) {
		*out_key = ALIF_NEWREF(key);
	}
	if (out_value != nullptr) {
		*out_value = ALIF_NEWREF(value);
	}
	return 0;

fail:
	di->dict = nullptr;
	ALIF_DECREF(d);
	return -1;
}

static AlifIntT acquire_keyValue(AlifObject** key_loc, AlifObject* value,
	AlifObject** value_loc, AlifObject** out_key, AlifObject** out_value) { // 5318
	if (out_key) {
		*out_key = alif_tryXGetRef(key_loc);
		if (*out_key == nullptr) {
			return -1;
		}
	}

	if (out_value) {
		if (!alif_tryIncRefCompare(value_loc, value)) {
			if (out_key) {
				ALIF_DECREF(*out_key);
			}
			return -1;
		}
		*out_value = value;
	}

	return 0;
}

static AlifIntT dictIter_iterNextThreadSafe(AlifDictObject* _d, AlifObject* _self,
	AlifObject** _outKey, AlifObject** _outValue) { // 5342
	AlifIntT res{};
	DictIterObject* di = (DictIterObject*)_self;
	AlifSizeT i{};
	AlifDictKeysObject* k{};

	if (di->used != alifAtomic_loadSizeRelaxed(&_d->used)) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"dictionary changed size during iteration");
		di->used = -1; /* Make this state sticky */
		return -1;
	}

	ensureShared_onRead(_d);

	i = alifAtomic_loadSizeRelaxed(&di->pos);
	k = (AlifDictKeysObject*)alifAtomic_loadPtrRelaxed(&_d->keys);
	if (ALIFDICT_HASSPLITTABLE(_d)) {
		AlifDictValues* values = (AlifDictValues*)alifAtomic_loadPtrRelaxed(&_d->values);
		if (values == nullptr) {
			goto concurrent_modification;
		}

		AlifSizeT used = (AlifSizeT)alifAtomic_loadUint8(&values->size);
		if (i >= used) {
			goto fail;
		}

		AlifIntT index = getIndex_fromOrder(_d, i);
		AlifObject* value = (AlifObject*)alifAtomic_loadPtr(&values->values[index]);
		if (acquire_keyValue(&dk_uStrEntries(k)[index].key, value,
			&values->values[index], _outKey, _outValue) < 0) {
			goto try_locked;
		}
	}
	else {
		AlifSizeT n = alifAtomic_loadSizeRelaxed(&k->nentries);
		if (DK_IS_USTR(k)) {
			AlifDictUStrEntry* entryPtr = &dk_uStrEntries(k)[i];
			AlifObject* value{};
			while (i < n and
				(value = (AlifObject*)alifAtomic_loadPtr(&entryPtr->value)) == nullptr) {
				entryPtr++;
				i++;
			}
			if (i >= n)
				goto fail;

			if (acquire_keyValue(&entryPtr->key, value,
				&entryPtr->value, _outKey, _outValue) < 0) {
				goto try_locked;
			}
		}
		else {
			AlifDictKeyEntry* entry_ptr = &dk_entries(k)[i];
			AlifObject* value{};
			while (i < n and
				(value = (AlifObject*)alifAtomic_loadPtr(&entry_ptr->value)) == nullptr) {
				entry_ptr++;
				i++;
			}

			if (i >= n)
				goto fail;

			if (acquire_keyValue(&entry_ptr->key, value,
				&entry_ptr->value, _outKey, _outValue) < 0) {
				goto try_locked;
			}
		}
	}
	// We found an element (key), but did not expect it
	AlifSizeT len;
	if ((len = alifAtomic_loadSizeRelaxed(&di->len)) == 0) {
		goto concurrent_modification;
	}

	alifAtomic_storeSizeRelaxed(&di->pos, i + 1);
	alifAtomic_storeSizeRelaxed(&di->len, len - 1);
	return 0;

concurrent_modification:
	//alifErr_setString(_alifExcRuntimeError_,
	//	"dictionary keys changed during iteration");

fail:
	di->dict = nullptr;
	ALIF_DECREF(_d);
	return -1;

try_locked:
	ALIF_BEGIN_CRITICAL_SECTION(_d);
	res = dictIter_iterNextItemLockHeld(_d, _self, _outKey, _outValue);
	ALIF_END_CRITICAL_SECTION();
	return res;
}


static bool has_uniqueReference(AlifObject* op) { // 5451
	return (alif_isOwnedByCurrentThread(op) and
		op->refLocal == 1 and
		alifAtomic_loadSizeRelaxed(&op->refShared) == 0);
}

static bool acquire_iterResult(AlifObject* result) { // 5463
	if (has_uniqueReference(result)) {
		ALIF_INCREF(result);
		return true;
	}
	return false;
}

static AlifObject* dictIter_iterNextItem(AlifObject* self) { // 5473
	DictIterObject* di = (DictIterObject*)self;
	AlifDictObject* d = di->dict;

	if (d == nullptr)
		return nullptr;

	AlifObject* key{}, * value{};
	if (dictIter_iterNextThreadSafe(d, self, &key, &value) == 0) {
		AlifObject* result = di->result;
		if (acquire_iterResult(result)) {
			AlifObject* oldkey = ALIFTUPLE_GET_ITEM(result, 0);
			AlifObject* oldvalue = ALIFTUPLE_GET_ITEM(result, 1);
			ALIFTUPLE_SET_ITEM(result, 0, key);
			ALIFTUPLE_SET_ITEM(result, 1, value);
			ALIF_DECREF(oldkey);
			ALIF_DECREF(oldvalue);

			if (!ALIFOBJECT_GC_IS_TRACKED(result)) {
				ALIFOBJECT_GC_TRACK(result);
			}
		}
		else {
			result = alifTuple_new(2);
			if (result == nullptr)
				return nullptr;
			ALIFTUPLE_SET_ITEM(result, 0, key);
			ALIFTUPLE_SET_ITEM(result, 1, value);
		}
		return result;
	}
	return nullptr;
}


AlifTypeObject _alifDictIterItemType_ = { // 5515
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "فهرس_تكرار_عنصر",
	.basicSize = sizeof(DictIterObject),
	/* methods */
	//.dealloc = dictIter_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = dictIter_traverse,
	.iter = alifObject_selfIter,
	.iterNext = dictIter_iterNextItem,
	//.methods = _dictIterMethods_,
};



AlifTypeObject _alifDictRevIterKeyType_ = { // 5665
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "فهرس_تكرار_المفاتيح_العكسي",
	.basicSize = sizeof(DictIterObject),
	//.dealloc = dictIter_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = dictIter_traverse,
	.iter = alifObject_selfIter,
	//.iternext = dictRevIter_iterNext,
	//.methods = dictiter_methods
};





AlifTypeObject _alifDictRevIterItemType_ = { // 5707
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "فهرس_تكرار_عنصر_عكسي",
	.basicSize = sizeof(DictIterObject),
	//.dealloc = dictIter_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = dictIter_traverse,
	.iter = alifObject_selfIter,
	//.iterNext = dictRevIter_iterNext,
	//.methods = dictIter_methods
};

AlifTypeObject _alifDictRevIterValueType_ = { // 5719
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "فهرس_تكرار_قيمة_عكسية",
	.basicSize = sizeof(DictIterObject),
	//.dealloc = dictIter_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = dictIter_traverse,
	.iter = alifObject_selfIter,
	//.iterNext = dictRevIter_iterNext,
	//.methods = dictIter_methods
};



AlifObject* _alifDictView_new(AlifObject* dict, AlifTypeObject* type) { // 5765
	AlifDictViewObject* dv{};
	if (dict == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	if (!ALIFDICT_CHECK(dict)) {
		//alifErr_format(_alifExcTypeError_,
		//	"%s() requires a dict argument, not '%s'",
		//	type->name, ALIF_TYPE(dict)->name);
		return nullptr;
	}
	dv = ALIFOBJECT_GC_NEW(AlifDictViewObject, type);
	if (dv == nullptr)
		return nullptr;
	dv->dict = (AlifDictObject*)ALIF_NEWREF(dict);
	ALIFOBJECT_GC_TRACK(dv);
	return (AlifObject*)dv;
}


static AlifObject* dictView_repr(AlifObject* self) { // 5897
	AlifDictViewObject* dv = (AlifDictViewObject*)self;
	AlifObject* seq{};
	AlifObject* result = nullptr;
	AlifSizeT rc{};

	rc = alif_reprEnter((AlifObject*)dv);
	if (rc != 0) {
		return rc > 0 ? alifUStr_fromString("...") : nullptr;
	}
	seq = alifSequence_list((AlifObject*)dv);
	if (seq == nullptr) {
		goto Done;
	}
	result = alifUStr_fromFormat("%s(%R)", ALIF_TYPE(dv)->name, seq);
	ALIF_DECREF(seq);

Done:
	alif_reprLeave((AlifObject*)dv);
	return result;
}



AlifTypeObject _alifDictKeysType_ = { // 6300
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مفاتيح_فهرس",
	.basicSize = sizeof(AlifDictViewObject),
	/* methods */
	.repr = dictView_repr,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};


static AlifObject* dict_keysImpl(AlifDictObject* self) { // 6339
	return _alifDictView_new((AlifObject*)self, &_alifDictKeysType_);
}

AlifTypeObject _alifDictItemsType_ = { // 6412
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "عناصر_فهرس",                            
	.basicSize = sizeof(AlifDictViewObject),                
	/* methods */
	.getAttro = alifObject_genericGetAttr,                    
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};


AlifTypeObject _alifDictValuesType_ = { // 6562
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "قيم_فهرس",                             
	.basicSize = sizeof(AlifDictViewObject),                
	/* methods */
	.repr = dictView_repr,
	.getAttro = alifObject_genericGetAttr,                  
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};


static AlifObject* dict_valuesImpl(AlifDictObject* self) { // 6601
	return _alifDictView_new((AlifObject*)self, &_alifDictValuesType_);
}


AlifDictKeysObject* _alifDict_newKeysForClass(AlifHeapTypeObject* _cls) { // 6561
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifDictKeysObject* keys = new_keysObject(
		interp, NEXT_LOG2_SHARED_KEYS_MAX_SIZE, 1);
	if (keys == nullptr) {
		alifErr_clear();
	}
	else {
		keys->usable = SHARED_KEYS_MAX_SIZE;
		keys->kind = DictKeysKind_::Dict_Keys_Split;
	}
	if (_cls->type.dict) {
		AlifObject* attrs = alifDict_getItem(_cls->type.dict, &ALIF_ID(__staticAttributes__));
		if (attrs != nullptr and ALIFTUPLE_CHECK(attrs)) {
			for (AlifSizeT i = 0; i < ALIFTUPLE_GET_SIZE(attrs); i++) {
				AlifObject* key = ALIFTUPLE_GET_ITEM(attrs, i);
				AlifHashT hash{};
				if (ALIFUSTR_CHECKEXACT(key) and (hash = uStr_getHash(key)) != -1) {
					if (insert_splitKey(keys, key, hash) == DKIX_EMPTY) {
						break;
					}
				}
			}
		}
	}
	return keys;
}


void alifObject_initInlineValues(AlifObject* _obj, AlifTypeObject* _tp) {  // 6594
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

static AlifDictObject* makeDict_fromInstanceAttributes(AlifInterpreter* _interp,
	AlifDictKeysObject* _keys, AlifDictValues* _values) { // 6616
	dictKeys_incRef(_keys);
	AlifSizeT used = 0;
	AlifSizeT track = 0;
	AlifUSizeT size = sharedKeys_usableSize(_keys);
	for (AlifUSizeT i = 0; i < size; i++) {
		AlifObject* val = _values->values[i];
		if (val != nullptr) {
			used += 1;
			track += alifObject_gcMayBeTracked(val);
		}
	}
	AlifDictObject* res = (AlifDictObject*)new_dict(_interp, _keys, _values, used, 0);
	if (track and res) {
		ALIFOBJECT_GC_TRACK(res);
	}
	return res;
}

AlifDictObject* alifObject_materializeManagedDictLockHeld(AlifObject* _obj) { // 6638

	AlifDictValues* values = alifObject_inlineValues(_obj);
	AlifDictObject* dict{};
	if (values->valid) {
		AlifInterpreter* interp = _alifInterpreter_get();
		AlifDictKeysObject* keys = CACHED_KEYS(ALIF_TYPE(_obj));
		dict = makeDict_fromInstanceAttributes(interp, keys, values);
	}
	else {
		dict = (AlifDictObject*)alifDict_new();
	}
	alifAtomic_storePtrRelease(&alifObject_managedDictPointer(_obj)->dict, dict);
	return dict;
}

AlifDictObject* alifObject_materializeManagedDict(AlifObject* _obj) { // 6660
	AlifDictObject* dict = alifObject_getManagedDict(_obj);
	if (dict != nullptr) {
		return dict;
	}

	ALIF_BEGIN_CRITICAL_SECTION(_obj);

	dict = alifObject_getManagedDict(_obj);
	if (dict != nullptr) {
		goto exit;
	}
	dict = alifObject_materializeManagedDictLockHeld(_obj);

	exit :
	ALIF_END_CRITICAL_SECTION();
	return dict;
}



static AlifIntT storeInstance_attrLockHeld(AlifObject* obj, AlifDictValues* values,
	AlifObject* name, AlifObject* value) { // 6702
	AlifDictKeysObject* keys = CACHED_KEYS(ALIF_TYPE(obj));
	AlifSizeT ix = DKIX_EMPTY;
	AlifDictObject* dict = alifObject_getManagedDict(obj);
	if (ALIFUSTR_CHECKEXACT(name)) {
		AlifHashT hash = uStr_getHash(name);
		if (hash == -1) {
			hash = _alifUStrType_.hash(name);
		}

		ix = insert_splitKey(keys, name, hash);

#ifdef ALIF_STATS
		if (ix == DKIX_EMPTY) {
			if (ALIFUSTR_CHECKEXACT(name)) {
				if (sharedKeys_usableSize(keys) == SHARED_KEYS_MAX_SIZE) {
					OBJECT_STAT_INC(dictMaterializedTooBig);
				}
				else {
					OBJECT_STAT_INC(dictMaterializedNewKey);
				}
			}
			else {
				OBJECT_STAT_INC(dictMaterializedStrSubclass);
			}
		}
#endif
	}

	if (ix == DKIX_EMPTY) {
		AlifIntT res{};
		if (dict == nullptr) {
			dict = makeDict_fromInstanceAttributes(alifInterpreter_get(), keys, values);
			if (dict == nullptr or
				alifDict_setItemLockHeld(dict, name, value) < 0) {
				ALIF_XDECREF(dict);
				return -1;
			}

			alifAtomic_storePtrRelease(&alifObject_managedDictPointer(obj)->dict,
				(AlifDictObject*)dict);
			return 0;
		}

		res = alifDict_setItemLockHeld(dict, name, value);
		return res;
	}

	AlifObject* old_value = values->values[ix];
	alifAtomic_storePtrRelease(&values->values[ix], ALIF_XNEWREF(value));

	if (old_value == nullptr) {
		if (value == nullptr) {
			//alifErr_format(_alifExcAttributeError_,
			//	"'%.100s' object has no attribute '%U'",
			//	ALIF_TYPE(obj)->name, name);
			return -1;
		}
		alifDictValues_addToInsertionOrder(values, ix);
		if (dict) {
			STORE_USED(dict, dict->used + 1);
		}
	}
	else {
		if (value == nullptr) {
			deleteIndex_fromValues(values, ix);
			if (dict) {
				STORE_USED(dict, dict->used - 1);
			}
		}
		ALIF_DECREF(old_value);
	}
	return 0;
}



static inline AlifIntT storeInstance_attrDict(AlifObject* _obj,
	AlifDictObject* _dict, AlifObject* _name, AlifObject* _value) { // 6791
	AlifDictValues* values = alifObject_inlineValues(_obj);
	AlifIntT res{};
	ALIF_BEGIN_CRITICAL_SECTION(_dict);
	if (_dict->values == values) {
		res = storeInstance_attrLockHeld(_obj, values, _name, _value);
	}
	else {
		res = alifDict_setItemLockHeld(_dict, _name, _value);
	}
	ALIF_END_CRITICAL_SECTION();
	return res;
}


AlifIntT alifObject_storeInstanceAttribute(AlifObject* _obj,
	AlifObject* _name, AlifObject* _value) { // 6807
	AlifDictValues* values = alifObject_inlineValues(_obj);
	if (!alifAtomic_loadUint8(&values->valid)) {
		AlifDictObject* dict = alifObject_getManagedDict(_obj);
		if (dict == nullptr) {
			dict = (AlifDictObject*)alifObject_genericGetDict(_obj, nullptr);
			if (dict == nullptr) {
				return -1;
			}
			AlifIntT res = storeInstance_attrDict(_obj, dict, _name, _value);
			ALIF_DECREF(dict);
			return res;
		}
		return storeInstance_attrDict(_obj, dict, _name, _value);
	}

	AlifDictObject* dict = alifObject_getManagedDict(_obj);
	if (dict == nullptr) {
		AlifIntT res{};
		ALIF_BEGIN_CRITICAL_SECTION(_obj);
		dict = alifObject_getManagedDict(_obj);

		if (dict == nullptr) {
			res = storeInstance_attrLockHeld(_obj, values, _name, _value);
		}
		ALIF_END_CRITICAL_SECTION();

		if (dict == nullptr) {
			return res;
		}
	}
	return storeInstance_attrDict(_obj, dict, _name, _value);
}

bool alifObject_tryGetInstanceAttribute(AlifObject* _obj,
	AlifObject* _name, AlifObject** _attr) { // 6891
	AlifDictValues* values = alifObject_inlineValues(_obj);
	if (!alifAtomic_loadUint8(&values->valid)) {
		return false;
	}

	AlifDictKeysObject* keys = CACHED_KEYS(ALIF_TYPE(_obj));
	AlifSizeT ix_ = alifDictKeys_stringLookup(keys, _name);
	if (ix_ == DKIX_EMPTY) {
		*_attr = nullptr;
		return true;
	}

	AlifObject* value = (AlifObject*)alifAtomic_loadPtrAcquire(&values->values[ix_]);
	if (value == nullptr or alif_tryIncRefCompare(&values->values[ix_], value)) {
		*_attr = value;
		return true;
	}

	AlifDictObject* dict = alifObject_getManagedDict(_obj);
	if (dict == nullptr) {
		bool success = false;
		ALIF_BEGIN_CRITICAL_SECTION(_obj);

		dict = alifObject_getManagedDict(_obj);
		if (dict == nullptr) {
			value = values->values[ix_];
			*_attr = ALIF_XNEWREF(value);
			success = true;
		}

		ALIF_END_CRITICAL_SECTION();

		if (success) {
			return true;
		}
	}


	bool success{};
	ALIF_BEGIN_CRITICAL_SECTION(dict);

	if (dict->values == values and alifAtomic_loadUint8(&values->valid)) {
		value = (AlifObject*)alifAtomic_loadPtrRelaxed(&values->values[ix_]);
		*_attr = ALIF_XNEWREF(value);
		success = true;
	}
	else {
		success = false;
	}

	ALIF_END_CRITICAL_SECTION();

	return success;
}



AlifIntT alifDict_setItemLockHeld(AlifDictObject* _dict,
	AlifObject* _name, AlifObject* _value) { // 6685
	if (_value == nullptr) {
		AlifHashT hash = alifObject_hashFast(_name);
		if (hash == -1) {
			return -1;
		}
		return delItem_knownHashLockHeld((AlifObject*)_dict, _name, hash);
	}
	else {
		return setItem_lockHeld(_dict, _name, _value);
	}
}




//void alifObject_clearManagedDict(AlifObject* _obj) { // 7098
//	if (_alifObject_setManagedDict(_obj, nullptr) < 0) {
//		alifErr_writeUnraisable(nullptr);
//	}
//}




static inline AlifObject* ensure_nonManagedDict(AlifObject* obj,
	AlifObject** dictptr) { // 7171
	AlifDictKeysObject* cached{};
	AlifTypeObject* tp{};

	AlifObject* dict = (AlifObject*)alifAtomic_loadPtrAcquire(&*dictptr);
	if (dict == nullptr) {
		ALIF_BEGIN_CRITICAL_SECTION(obj);
		dict = *dictptr;
		if (dict != nullptr) {
			goto done;
		}
		tp = ALIF_TYPE(obj);
		if (_alifType_hasFeature(tp, ALIF_TPFLAGS_HEAPTYPE) and (cached = CACHED_KEYS(tp))) {
			AlifInterpreter* interp = _alifInterpreter_get();
			dict = newDict_withSharedKeys(interp, cached);
		}
		else {
			dict = alifDict_new();
		}
		alifAtomic_storePtrRelease(&*dictptr, dict);
done:
		ALIF_END_CRITICAL_SECTION();
	}
	return dict;
}


static inline AlifObject* ensure_managedDict(AlifObject* _obj) { // 7138
	AlifDictObject* dict = alifObject_getManagedDict(_obj);
	if (dict == nullptr) {
		AlifTypeObject* tp = ALIF_TYPE(_obj);
		if ((tp->flags & ALIF_TPFLAGS_INLINE_VALUES) &&
			alifAtomic_loadUint8(&alifObject_inlineValues(_obj)->valid)) {
			dict = alifObject_materializeManagedDict(_obj);
		}
		else {
			ALIF_BEGIN_CRITICAL_SECTION(_obj);
			dict = alifObject_getManagedDict(_obj);
			if (dict != nullptr) {
				goto done;
			}
			dict = (AlifDictObject*)newDict_withSharedKeys(_alifInterpreter_get(), CACHED_KEYS(tp));
			alifAtomic_storePtrRelease(&alifObject_managedDictPointer(_obj)->dict, (AlifDictObject*)dict);

			done :
			ALIF_END_CRITICAL_SECTION();
		}
	}
	return (AlifObject*)dict;
}


AlifObject* alifObject_genericGetDict(AlifObject* _obj, void* _context) { // 7203
	AlifTypeObject* tp = ALIF_TYPE(_obj);
	if (_alifType_hasFeature(tp, ALIF_TPFLAGS_MANAGED_DICT)) {
		return ALIF_XNEWREF(ensure_managedDict(_obj));
	}
	else {
		AlifObject** dictptr = alifObject_computedDictPointer(_obj);
		if (dictptr == nullptr) {
			//alifErr_setString(_alifExcAttributeError_,
			//	"This object has no __dict__");
			return nullptr;
		}

		return ALIF_XNEWREF(ensure_nonManagedDict(_obj, dictptr));
	}
}



AlifIntT alifObjectDict_setItem(AlifTypeObject* _tp, AlifObject* _obj, AlifObject** _dictptr,
	AlifObject* _key, AlifObject* _value) { // 7222
	AlifObject* dict{};
	AlifIntT res{};

	dict = ensure_nonManagedDict(_obj, _dictptr);
	if (dict == nullptr) {
		return -1;
	}

	ALIF_BEGIN_CRITICAL_SECTION(dict);
	res = alifDict_setItemLockHeld((AlifDictObject*)dict, _key, _value);
	ALIF_END_CRITICAL_SECTION();
	return res;
}


void alifDictKeys_decRef(AlifDictKeysObject* _keys) { // 7246
	AlifInterpreter* interp = _alifInterpreter_get();
	dictKeys_decRef(interp, _keys, false);
}


void _alifDict_sendEvent(AlifIntT _watcherBits,
	AlifDictWatchEvent_ _event, AlifDictObject* _mp,
	AlifObject* _key, AlifObject* _value) { // 7348
	AlifInterpreter* interp = alifInterpreter_get();
	for (AlifIntT i = 0; i < DICT_MAX_WATCHERS; i++) {
		if (_watcherBits & 1) {
			AlifDictWatchCallback cb_ = interp->dictState.watchers[i];
			if (cb_ and (cb_(_event, (AlifObject*)_mp, _key, _value) < 0)) {
				//alifErr_formatUnraisable(
					//"Exception ignored in %s watcher callback for <dict at %p>",
					//dict_eventName(event), mp);
			}
		}
		_watcherBits >>= 1;
	}
}
