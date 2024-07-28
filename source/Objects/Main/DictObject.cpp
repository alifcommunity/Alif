#define ALIFDICT_LOG_MINSIZE 3
#define ALIFDICT_MINSIZE 8

#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Dict.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Interpreter.h"

#define IS_DICT_SHARED(_mp) (false)
#define LOAD_INDEX(_keys, size, idx) ((const int##size##_t*)(_keys->indices))[idx]
#define STORE_INDEX(_keys, size, idx, value) ((int##size##_t*)(_keys->indices))[idx] = (int##size##_t)value

static inline void split_keys_entry_added(AlifDictKeysObject* _keys)
{
	_keys->usable--;
	_keys->nentries++;
}

static inline void set_keys(AlifDictObject* _mp, AlifDictKeysObject* _keys)
{
	_mp->keys = _keys;
}

static inline void set_values(AlifDictObject* _mp, AlifDictValues* values)
{
	_mp->values = values;
}

static inline int64_t load_keys_nentries(AlifDictObject* _mp)
{
	return _mp->keys->nentries;
}

#define ALIFDICT_LOG_MINSIZE 3
#define ALIFDICT_MINSIZE 8

#define USABLE_FRACTION(n) ((n << 1)/3)

#define STORE_KEYS_USABLE(keys, usable) keys->usable = usable
#define STORE_KEYS_NENTRIES(keys, nentries) keys->nentries = nentries

#define PERTURB_SHIFT 5

// Forwards
static AlifIntT setItem_lockHeld(AlifDictObject*, AlifObject*, AlifObject*);


#define DK_MASK(dk_) (DK_SIZE(dk_)-1)


static inline void dictKeys_decref(AlifInterpreter* _interp, AlifDictKeysObject* _dk, bool _useQsbr) // 454
{
	if (_dk->refCnt == ALIF_IMMORTAL_REFCNT) {
		return;
	}

	if ((_dk->refCnt--) == 1) {
		if ((_dk->kind != Dict_Keys_General)) {
			AlifDictUnicodeEntry* entries_ = dk_uStr_entries(_dk);
			int64_t i_, n_;
			for (i_ = 0, n_ = _dk->nentries; i_ < n_; i_++) {
				ALIF_XDECREF(entries_[i_].key);
				ALIF_XDECREF(entries_[i_].value);
			}
		}
		else {
			AlifDictKeyEntry* entries_ = dk_entries(_dk);
			int64_t i, n;
			for (i = 0, n = _dk->nentries; i < n; i++) {
				ALIF_XDECREF(entries_[i].key);
				ALIF_XDECREF(entries_[i].value);
			}
		}
		//free_keys_object(dk_, use_qsbr);
	}
}

static inline int64_t dictKeys_get_index(const AlifDictKeysObject* _keys, int64_t _i)// 486
{
	int log2size = DK_LOG_SIZE(_keys);
	int64_t ix_;

	if (log2size < 8) {
		ix_ = LOAD_INDEX(_keys, 8, _i);
	}
	else if (log2size < 16) {
		ix_ = LOAD_INDEX(_keys, 16, _i);
	}
#if SIZEOF_VOID_P > 4
	else if (log2size >= 32) {
		ix_ = LOAD_INDEX(_keys, 64, _i);
	}
#endif
	else {
		ix_ = LOAD_INDEX(_keys, 32, _i);
	}
	return ix_;
}

/* write to indices. */
static inline void dictKeys_set_index(AlifDictKeysObject* _keys, int64_t _i, int64_t _ix) // 511
{
	int log2size = DK_LOG_SIZE(_keys);

	if (log2size < 8) {
		STORE_INDEX(_keys, 8, _i, _ix);
	}
	else if (log2size < 16) {
		STORE_INDEX(_keys, 16, _i, _ix);
	}
#if SIZEOF_VOID_P > 4
	else if (log2size >= 32) {
		STORE_INDEX(_keys, 64, _i, _ix);
	}
#endif
	else {
		STORE_INDEX(_keys, 32, _i, _ix);
	}
}

static inline uint8_t calculate_log2_keySize(int64_t _minSize)
{
//#if SIZEOF_LONG == SIZEOF_SIZE_T
	//_minSize = (_minSize | ALIFDICT_MINSIZE) - 1;
	//return alifBit_length(minsize | (ALIFDICT_MINSIZE - 1));
#if defined(_MSC_VER)
	// On 64bit Windows, sizeof(long) == 4.
	_minSize = (_minSize | ALIFDICT_MINSIZE) - 1;
	unsigned long msb_;
	_BitScanReverse64(&msb_, (uint64_t)_minSize);
	return (uint8_t)(msb_ + 1);
#else
	uint8_t log2Size;
	for (log2Size = ALIFDICT_LOG_MINSIZE;
		(((int64_t)1) << log2Size) < _minSize;
		log2Size++)
		;
	return log2Size;
#endif
}

#define GROWTH_RATE(d) ((d)->used*3)


static AlifDictKeysObject _emptyKeysClass_ = { // 601
		ALIF_IMMORTAL_REFCNT, /* dk_refcnt */
		0, /* dk_log2_size */
		0, /* dk_log2_index_bytes */
		Dict_Keys_UStr, /* dk_kind */
		1, /* dk_version */
		0, /* dk_usable (immutable) */
		0, /* dk_nentries */
		{DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY,
		 DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY, DKIX_EMPTY}, /* dk_indices */
};

#define ALIF_EMPTY_KEYS &_emptyKeysClass_

static inline int getIndex_fromOrder(AlifDictObject* _mp, int64_t _i) // 628
{
	uint8_t* array_ = getInsertion_orderArray(_mp->values);
	return array_[_i];
}


static AlifDictKeysObject* new_keys_object(AlifInterpreter* _interp, uint8_t _log2Size, bool _uStr) // 750
{
	AlifDictKeysObject* dk_;
	int64_t usable;
	int log2_bytes;
	size_t entry_size = _uStr ? sizeof(AlifDictUnicodeEntry) : sizeof(AlifDictKeyEntry);

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

	dk_ = (AlifDictKeysObject*)alifMem_dataAlloc(sizeof(AlifDictKeysObject)
		+ ((size_t)1 << log2_bytes)
		+ entry_size * usable);
	if (dk_ == nullptr) {
		return nullptr;
	}

	dk_->refCnt = 1;
	dk_->log2Size = _log2Size;
	dk_->log2IndexBytes = log2_bytes;
	dk_->kind = _uStr ? Dict_Keys_UStr : Dict_Keys_General;

	dk_->nentries = 0;
	dk_->usable = usable;
	dk_->version = 0;
	memset(&dk_->indices[0], 0xff, ((size_t)1 << log2_bytes));
	memset(&dk_->indices[(size_t)1 << log2_bytes], 0, entry_size * usable);
	return dk_;
}

static void free_keys_object(AlifDictKeysObject* _keys, bool _useQsbr) // 811
{
	alifMem_objFree(_keys);
}


static inline void free_values(AlifDictValues* _values, bool _useQsbr) // 861
{
	alifMem_objFree(_values);
}

static AlifObject* new_dict(AlifInterpreter* _interp,
	AlifDictKeysObject* _keys, AlifDictValues* _values,
	int64_t _used, int _freeValuesOnFailure) // 874
{
	AlifDictObject* mp_;
//#ifdef WITH_FREELISTS
//	class AlifDictFreeList* freelist = get_dict_freelist();
//	if (freelist->numfree > 0) {
//		_mp = freelist->items[--freelist->numfree];
//		alifSub_newReference((AlifObject*)_mp);
//	}
//	else
//#endif
	
	mp_ = ALIFOBJECT_GC_NEW(AlifDictObject, &_alifDictType_);
	if (mp_ == nullptr) {
		dictKeys_decref(_interp, _keys, false);
		if (_freeValuesOnFailure) {
			free_values(_values, false);
		}
		return nullptr;
	}
	
	mp_->keys = _keys;
	mp_->values = _values;
	mp_->used = _used;
	//mp_->versionTag = DICT_NEXT_VERSION(_interp); // سيتم العمل عليه عندما يتم عمل نسخة اخرى للغة او ربما لن نحتاجه
	return (AlifObject*)mp_;
}

AlifObject* alifNew_dict() // 987
{
	AlifInterpreter* interp_ = alifInterpreter_get();

    return new_dict(interp_, ALIF_EMPTY_KEYS, nullptr, 0,0);
}

static int64_t lookDict_index(AlifDictKeysObject* _k, size_t _hash, int64_t _index) // 995
{
	size_t mask_ = DK_MASK(_k);
	size_t perturb_ = (size_t)_hash;
	size_t i_ = (size_t)_hash & mask_;

	for (;;) {
		int64_t ix = dictKeys_get_index(_k, i_);
		if (ix == _index) {
			return i_;
		}
		if (ix == DKIX_EMPTY) {
			return DKIX_EMPTY;
		}
		perturb_ >>= PERTURB_SHIFT;
		i_ = mask_ & (i_ * 5 + perturb_ + 1);
	}
	//ALIF_UNREACHABLE();
}

// 1518
#define MAINTAIN_TRACKING(mp, _key, value) \
    do { \
        if (!ALIFSUBOBJECT_GC_ISTRACKED(mp)) { \
            if (alifObjectGC_mayBeTracked(_key) || alifObjectGC_mayBeTracked(value)) { \
                alifObject_gc_track(mp); \
            } \
        } \
    } while (0)

static int insert_to_emptyDict(AlifInterpreter* _interp, AlifDictObject* _mp,
	AlifObject* _key, size_t _hash, AlifObject* _value) // 1786
{
	//uint64_t new_version = alifDict_NotifyEvent(
		//_interp, AlifDict_Even_ADDED, _mp, _key, _value);

	int unicode = ALIFUSTR_CHECKEXACT(_key);
	AlifDictKeysObject* newkeys = new_keys_object(
		_interp, ALIFDICT_LOG_MINSIZE, unicode);
	if (newkeys == nullptr) {
		ALIF_DECREF(_key);
		ALIF_DECREF(_value);
		return -1;
	}
	MAINTAIN_TRACKING(_mp, _key, _value);

	size_t hashpos = (size_t)_hash & (ALIFDICT_MINSIZE - 1);
	dictKeys_set_index(newkeys, hashpos, 0);
	if (unicode) {
		AlifDictUnicodeEntry* ep_ = dk_uStr_entries(newkeys);
		ep_->key = _key;
		ep_->value = _value;
	}
	else {
		AlifDictKeyEntry* ep_ = dk_entries(newkeys);
		ep_->key = _key;
		ep_->hash = _hash;
		ep_->value = _value;
	}
	_mp->used++;
	//_mp->versionTag = new_version;
	newkeys->usable--;
	newkeys->nentries++;
	_mp->keys = newkeys;
	return 0;
}

static void build_indices_generic(AlifDictKeysObject* _keys, AlifDictKeyEntry* _ep, int64_t _n) // 1842
{
	size_t mask_ = DK_MASK(_keys);
	for (int64_t ix_ = 0; ix_ != _n; ix_++, _ep++) {
		size_t hash_ = _ep->hash;
		size_t i_ = hash_ & mask_;
		for (size_t perturb_ = hash_; dictKeys_get_index(_keys, i_) != DKIX_EMPTY;) {
			perturb_ >>= PERTURB_SHIFT;
			i_ = mask_ & (i_ * 5 + perturb_ + 1);
		}
		dictKeys_set_index(_keys, i_, ix_);
	}
}

static void build_indices_unicode(AlifDictKeysObject* _keys, AlifDictUnicodeEntry* _ep, int64_t _n) // 1857
{
	size_t mask_ = DK_MASK(_keys);
	for (int64_t ix_ = 0; ix_ != _n; ix_++, _ep++) {
		size_t hash_ = ((AlifUStrObject*)_ep->key)->hash_;
		size_t i = hash_ & mask_;
		for (size_t perturb_ = hash_; dictKeys_get_index(_keys, i) != DKIX_EMPTY;) {
			perturb_ >>= PERTURB_SHIFT;
			i = mask_ & (i * 5 + perturb_ + 1);
		}
		dictKeys_set_index(_keys, i, ix_);
	}
}

static int dictresize(AlifInterpreter* _interp, AlifDictObject* _mp,
	uint8_t _log2NewSize, int _uStr) // 1887
{
	AlifDictKeysObject* oldKeys, * newKeys;
	AlifDictValues* oldValues;

	if (_log2NewSize >= 8 * 8) {
		return -1;
	}

	oldKeys = _mp->keys;
	oldValues = _mp->values;

	if (!(oldKeys->kind != Dict_Keys_General)) {
		_uStr = 0;
	}

	//ensure_shared_on_resize(_mp);

	newKeys = new_keys_object(_interp, _log2NewSize, _uStr);
	if (newKeys == nullptr) {
		return -1;
	}

	int64_t numEntries = _mp->used;

	if (oldValues != nullptr) {
		//LOCK_KEYS(oldKeys);
		AlifDictUnicodeEntry* oldEntries = dk_uStr_entries(oldKeys);

		if (newKeys->kind == Dict_Keys_General) {
			AlifDictKeyEntry* newEntries = dk_entries(newKeys);

			for (int64_t i = 0; i < numEntries; i++) {
				int index = getIndex_fromOrder(_mp, i);
				AlifDictUnicodeEntry* ep_ = &oldEntries[index];
				newEntries[i].key = ALIF_NEWREF(ep_->key);
				newEntries[i].hash = ((AlifUStrObject*)ep_->key)->hash_;
				newEntries[i].value = oldValues->values[index];
			}
			build_indices_generic(newKeys, newEntries, numEntries);
		}
		else { 
			AlifDictUnicodeEntry* newEntries = dk_uStr_entries(newKeys);

			for (int64_t i = 0; i < numEntries; i++) {
				int index_ = getIndex_fromOrder(_mp, i);
				AlifDictUnicodeEntry* ep_ = &oldEntries[index_];
				newEntries[i].key = ALIF_NEWREF(ep_->key);
				newEntries[i].value = oldValues->values[index_];
			}
			build_indices_unicode(newKeys, newEntries, numEntries);
		}
		//UNLOCK_KEYS(oldKeys);
		set_keys(_mp, newKeys);
		dictKeys_decref(_interp, oldKeys, IS_DICT_SHARED(_mp));
		set_values(_mp, nullptr);
		if (oldValues->embedded) {

			oldValues->valid = 0;
		}
		else {
			free_values(oldValues, IS_DICT_SHARED(_mp));
		}
	}
	else { 
		if (oldKeys->kind == Dict_Keys_General) {
			AlifDictKeyEntry* oldEntries = dk_entries(oldKeys);
			AlifDictKeyEntry* newEntries = dk_entries(newKeys);
			if (oldKeys->nentries == numEntries) {
				memcpy(newEntries, oldEntries, numEntries * sizeof(AlifDictKeyEntry));
			}
			else {
				AlifDictKeyEntry* ep_ = oldEntries;
				for (int64_t i = 0; i < numEntries; i++) {
					while (ep_->value == nullptr)
						ep_++;
					newEntries[i] = *ep_++;
				}
			}
			build_indices_generic(newKeys, newEntries, numEntries);
		}
		else {  
			AlifDictUnicodeEntry* oldEntries = dk_uStr_entries(oldKeys);
			if (_uStr) { 
				AlifDictUnicodeEntry* newEntries = dk_uStr_entries(newKeys);
				if (oldKeys->nentries == numEntries && _mp->keys->kind == Dict_Keys_UStr) {
					memcpy(newEntries, oldEntries, numEntries * sizeof(AlifDictUnicodeEntry));
				}
				else {
					AlifDictUnicodeEntry* ep_ = oldEntries;
					for (int64_t i = 0; i < numEntries; i++) {
						while (ep_->value == nullptr)
							ep_++;
						newEntries[i] = *ep_++;
					}
				}
				build_indices_unicode(newKeys, newEntries, numEntries);
			}
			else { // combined unicode -> generic
				AlifDictKeyEntry* newEntries = dk_entries(newKeys);
				AlifDictUnicodeEntry* ep_ = oldEntries;
				for (int64_t i = 0; i < numEntries; i++) {
					while (ep_->value == nullptr)
						ep_++;
					newEntries[i].key = ep_->key;
					newEntries[i].hash = ((AlifUStrObject*)ep_->key)->hash_;
					newEntries[i].value = ep_->value;
					ep_++;
				}
				build_indices_generic(newKeys, newEntries, numEntries);
			}
		}

		set_keys(_mp, newKeys);

		if (oldKeys != ALIF_EMPTY_KEYS) {
			free_keys_object(oldKeys, IS_DICT_SHARED(_mp));
		}
	}

	_mp->keys->usable =  _mp->keys->usable - numEntries;
	_mp->keys->nentries =  numEntries;
	return 0;
}

static inline  int64_t do_lookup(AlifDictObject* _mp, AlifDictKeysObject* _dk, AlifObject* _key, size_t _hash,
	int64_t(*_checkLookup)(AlifDictObject*, AlifDictKeysObject*, void*, int64_t _ix, AlifObject* _key, size_t)) // 1017
{
	void* ep0_ = dkSub_entries(_dk);
	size_t mask_ = DK_MASK(_dk);
	size_t perturb_ = _hash;
	size_t i_ = (size_t)_hash & mask_;
	int64_t ix_;
	for (;;) {
		ix_ = dictKeys_get_index(_dk, i_);
		if (ix_ >= 0) {
			int64_t cmp_ = _checkLookup(_mp, _dk, ep0_, ix_, _key, _hash);
			if (cmp_ < 0) {
				return cmp_;
			}
			else if (cmp_) {
				return ix_;
			}
		}
		else if (ix_ == DKIX_EMPTY) {
			return DKIX_EMPTY;
		}
		perturb_ >>= PERTURB_SHIFT;
		i_ = mask_ & (i_ * 5 + perturb_ + 1);

		ix_ = dictKeys_get_index(_dk, i_);
		if (ix_ >= 0) {
			int64_t cmp_ = _checkLookup(_mp, _dk, ep0_, ix_, _key, _hash);
			if (cmp_ < 0) {
				return cmp_;
			}
			else if (cmp_) {
				return ix_;
			}
		}
		else if (ix_ == DKIX_EMPTY) {
			return DKIX_EMPTY;
		}
		perturb_ >>= PERTURB_SHIFT;
		i_ = mask_ & (i_ * 5 + perturb_ + 1);
	}
	//ALIF_UNREACHABLE();
}

static inline  int64_t compare_unicode_generic(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, int64_t _ix, AlifObject* _key, size_t _hash) // 1061
{
	AlifDictUnicodeEntry* ep_ = &((AlifDictUnicodeEntry*)_ep0)[_ix];

	if (((AlifUStrObject*)ep_->key)->hash_ == _hash) {
		AlifObject* startKey = ep_->key;
		ALIF_INCREF(startKey);
		int cmp_ = alifObject_richCompareBool(startKey, _key, ALIF_EQ);
		ALIF_DECREF(startKey);
		if (cmp_ < 0) {
			return DKIX_ERROR;
		}
		if (_dk == _mp->keys && ep_->key == startKey) {
			return cmp_;
		}
		else {
			return DKIX_KEY_CHANGED;
		}
	}
	return 0;
}

static int64_t unicodeKeys_lookup_generic(AlifDictObject* _mp, AlifDictKeysObject* _dk, AlifObject* _key, size_t _hash) // 1090
{
	return do_lookup(_mp, _dk, _key, _hash, compare_unicode_generic);
}

static inline int64_t compare_unicode_unicode(AlifDictObject* _mp, AlifDictKeysObject* _dk,
	void* _ep0, int64_t _ix, AlifObject* _key, size_t _hash) // 1096
{
	AlifDictUnicodeEntry* ep_ = &((AlifDictUnicodeEntry*)_ep0)[_ix];
	if (ep_->key == _key ||
		(((AlifUStrObject*)ep_->key)->hash_ == _hash && uStr_eq(ep_->key, _key))) {
		return 1;
	}
	return 0;
}

static int64_t unicodeKeys_lookup_unicode(AlifDictKeysObject* _dk, AlifObject* _key, size_t _hash) // 1110
{
	return do_lookup(nullptr, _dk, _key, _hash, compare_unicode_unicode);
}

static inline int64_t compare_generic(AlifDictObject* mp, AlifDictKeysObject* dk,
	void* ep0, int64_t ix, AlifObject* _key, size_t hash) // 1116
{
	AlifDictKeyEntry* ep_ = &((AlifDictKeyEntry*)ep0)[ix];
	if (ep_->key == _key) {
		return 1;
	}
	if (ep_->hash == hash) {
		AlifObject* startKey = ep_->key;
		ALIF_INCREF(startKey);
		int cmp_ = alifObject_richCompareBool(startKey, _key, ALIF_EQ);
		ALIF_DECREF(startKey);
		if (cmp_ < 0) {
			return DKIX_ERROR;
		}
		if (dk == mp->keys && ep_->key == startKey) {
			return cmp_;
		}
		else {
			return DKIX_KEY_CHANGED;
		}
	}
	return 0;
}

static int64_t dictKeys_generic_lookup(AlifDictObject* mp, AlifDictKeysObject* dk, AlifObject* _key, size_t hash) // 1144
{
	return do_lookup(mp, dk, _key, hash, compare_generic);
}

int64_t alifDict_lookup(AlifDictObject* _mp, AlifObject* _key, size_t _hash, AlifObject** _valueAddr) //1189
{
	AlifDictKeysObject* dk_;
	DictKeysKind kind_;
	int64_t ix_;

start:
	dk_ = _mp->keys;
	kind_ = dk_->kind;

	if (kind_ != Dict_Keys_General) {
		if (ALIFUSTR_CHECKEXACT(_key)) {
			ix_ = unicodeKeys_lookup_unicode(dk_, _key, _hash);
		}
		else {
			ix_ = unicodeKeys_lookup_generic(_mp, dk_, _key, _hash);
			if (ix_ == DKIX_KEY_CHANGED) {
				goto start;
			}
		}

		if (ix_ >= 0) {
			if (kind_ == Dict_Keys_Split) {
				*_valueAddr = _mp->values->values[ix_];
			}
			else {
				*_valueAddr = dk_uStr_entries(dk_)[ix_].value;
			}
		}
		else {
			*_valueAddr = nullptr;
		}
	}
	else {
		ix_ = dictKeys_generic_lookup(_mp, dk_, _key, _hash);
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

int64_t alifDict_lookupThreadSafe(AlifDictObject* _mp, AlifObject* _key, size_t _hash, AlifObject** _valueAddr) // 1493
{
	int64_t ix = alifDict_lookup(_mp, _key, _hash, _valueAddr);
	ALIF_XNEWREF(*_valueAddr);
	return ix;
}

static inline int is_unusable_slot(int64_t _ix) // 1574
{
	return _ix >= 0;
}

static int64_t find_empty_slot(AlifDictKeysObject* _keys, size_t _hash) // 1588
{
	const size_t mask = DK_MASK(_keys);
	size_t i = _hash & mask;
	int64_t ix = dictKeys_get_index(_keys, i);
	for (size_t perturb = _hash; is_unusable_slot(ix);) {
		perturb >>= PERTURB_SHIFT;
		i = (i * 5 + perturb + 1) & mask;
		ix = dictKeys_get_index(_keys, i);
	}
	return i;
}

static int insertion_resize(AlifInterpreter* _interp, AlifDictObject* _mp, int _uStr)// 1604
{
	return dictresize(_interp, _mp, calculate_log2_keySize(GROWTH_RATE(_mp)), _uStr);
}

static inline int insert_combined_dict(AlifInterpreter* _interp, AlifDictObject* _mp,
	size_t _hash, AlifObject* _key, AlifObject* _value) // 1635
{
	if (_mp->keys->usable <= 0) {
		if (insertion_resize(_interp, _mp, 1) < 0) {
			return -1;
		}
	}

	int64_t hashPos = find_empty_slot(_mp->keys, _hash);
	dictKeys_set_index(_mp->keys, hashPos, _mp->keys->nentries);

	if ((_mp->keys->kind != Dict_Keys_General)) {
		AlifDictUnicodeEntry* ep_;
		ep_ = &dk_uStr_entries(_mp->keys)[_mp->keys->nentries];
		ep_->key = _key;
	    ep_->value =  _value;
	}
	else {
		AlifDictKeyEntry* ep_;
		ep_ = &dk_entries(_mp->keys)[_mp->keys->nentries];
		ep_->key = _key;
		ep_->value = _value;
		ep_->hash = _hash;
	}

	_mp->keys->usable = _mp->keys->usable - 1;
	_mp->keys->nentries = _mp->keys->nentries + 1;

	return 0;
}

static int insert_split_dict(AlifInterpreter* _interp, AlifDictObject* _mp,
	size_t _hash, AlifObject* _key, AlifObject* _value) // 1668
{
	AlifDictKeysObject* keys_ = _mp->keys;
	//LOCK_KEYS(keys);
	if (keys_->usable <= 0) {
		//UNLOCK_KEYS(keys);
		int ins_ = insertion_resize(_interp, _mp, 1);
		if (ins_ < 0) {
			return -1;
		}
		return insert_combined_dict(_interp, _mp, _hash, _key, _value);
	}

	int64_t hashPos = find_empty_slot(keys_, _hash);
	dictKeys_set_index(keys_, hashPos, keys_->nentries);

	AlifDictUnicodeEntry* ep_;
	ep_ = &dk_uStr_entries(keys_)[keys_->nentries];
	ep_->key = _key;

	int64_t index_ = keys_->nentries;
	alifDictValues_addToInsertionOrder(_mp->values, index_);
	_mp->values->values[index_] = _value;

	split_keys_entry_added(keys_);
	//UNLOCK_KEYS(keys);
	return 0;
}

static int insertDict(AlifInterpreter* _interp, AlifDictObject* _mp,
	AlifObject* _key, size_t _hash, AlifObject* _value) // 1708
{
	AlifObject* oldValue;
	int64_t ix_{};

	if ((_mp->keys->kind != Dict_Keys_General) && !ALIFUSTR_CHECKEXACT(_key)) {
		if (insertion_resize(_interp, _mp, 0) < 0)
			goto Fail;
	}

	ix_ = alifDict_lookup(_mp, _key, _hash, &oldValue);
	if (ix_ == DKIX_ERROR)
		goto Fail;

	//MAINTAIN_TRACKING(_mp, _key, _value);

	if (ix_ == DKIX_EMPTY) {
		//uint64_t new_version = alifDict_NotifyEvent(
			//interp, AlifDict_EVENT_ADDED, mp, _key, value);
		_mp->keys->version = 0;

		if (!ALIFDICT_HASSPLITTABLE(_mp)) {
			if (insert_combined_dict(_interp, _mp, _hash, _key, _value) < 0) {
				goto Fail;
			}
		}
		else {
			if (insert_split_dict(_interp, _mp, _hash, _key, _value) < 0)
				goto Fail;
		}

		_mp->used++;
		//mp->versionTag = new_version;
		return 0;
	}

	if (oldValue != _value) {
		//uint64_t new_version = alifDict_NotifyEvent(
			//interp, AlifDict_EVENT_MODIFIED, mp, _key, value);
		if (ALIFDICT_HASSPLITTABLE(_mp)) {
			_mp->values->values[ix_] = _value;
			if (oldValue == nullptr) {
				alifDictValues_addToInsertionOrder(_mp->values, ix_);
				_mp->used++;
			}
		}
		else {
			if ((_mp->keys->kind != Dict_Keys_General)) {
				dk_uStr_entries(_mp->keys)[ix_].value = _value;
			}
			else {
				dk_entries(_mp->keys)[ix_].value = _value;
			}
		}
		//mp->versionTag = new_version;
	}
	ALIF_XDECREF(oldValue);
	ALIF_DECREF(_key);
	return 0;

Fail:
	ALIF_DECREF(_value);
	ALIF_DECREF(_key);
	return -1;
}

// للمراجعة
static AlifObject* dictNew_presized(AlifInterpreter* _interp, AlifSizeT _minUsed, bool _unicode) { // 2081
	const uint8_t log2MaxPresize = 17;
	const AlifSizeT maxPresize = ((AlifSizeT)1) << log2MaxPresize;
	uint8_t log2NewSize{};
	AlifDictKeysObject* newKeys{};

	if (_minUsed <= USABLE_FRACTION(ALIFDICT_MINSIZE)) {
		return alifNew_dict();
	}

	if (_minUsed > USABLE_FRACTION(maxPresize)) {
		log2NewSize = log2MaxPresize;
	}
	else {
		//log2NewSize = estimateLog2_keySize(_minUsed);
	}

	newKeys = new_keys_object(_interp, log2NewSize, _unicode);
	if (newKeys == nullptr) return nullptr;

	return new_dict(_interp, newKeys, nullptr, 0, 0);
	//return nullptr; // 
}

AlifObject* alifDict_fromItems(AlifObject* const* _keys, AlifSizeT _keysOffset,
	AlifObject* const* _values, AlifSizeT _valuesOffset, AlifSizeT _length) { // 2116

	bool unicode_ = true;
	AlifObject* const* ks_ = _keys;
	AlifInterpreter* interp_ = alifInterpreter_get();

	for (AlifSizeT i = 0; i < _length; i++) {
		if (!ALIFUSTR_CHECKEXACT(*ks_)) {
			unicode_ = false;
			break;
		}
		ks_ += _keysOffset;
	}

	AlifObject* dict_ = dictNew_presized(interp_, _length, unicode_);
	if (dict_ == nullptr) {
		return nullptr;
	}

	ks_ = _keys;
	AlifObject* const* vs_ = _values;

	for (AlifSizeT i_ = 0; i_ < _length; i_++) {
		AlifObject* key_ = *ks_;
		AlifObject* value_ = *vs_;
		if (setItem_lockHeld((AlifDictObject*)dict_, key_, value_) < 0) {
			ALIF_DECREF(dict_);
			return nullptr;
		}
		ks_ += _keysOffset;
		vs_ += _valuesOffset;
	}

	return dict_;
}

static AlifObject* dict_getitem(AlifObject* _op, AlifObject* _key, const wchar_t* _warnMsg)
{
	if (!ALIFDICT_CHECK(_op)) {
		return nullptr;
	}
	AlifDictObject* mp_ = (AlifDictObject*)_op;

	size_t hash_;
	if (!ALIFUSTR_CHECKEXACT(_key) || (hash_ = ((AlifUStrObject*)_key)->hash_) == -1) {
		hash_ = alifObject_hash(_key);
		if (hash_ == -1) {
			return nullptr;
		}
	}

	AlifObject* value_;
	int64_t ix_; (void)ix_;

	ix_ = alifDict_lookup(mp_, _key, hash_, &value_);

	return value_;  // borrowed reference
}

AlifObject* alifDict_getItem(AlifObject* _op, AlifObject* _key)
{
	return dict_getitem(_op, _key,
		L"Exception ignored in alifDict_getItem(); consider using");
}

AlifObject* alifDictGetItem_knownHash(AlifObject* _op, AlifObject* _key, size_t _hash) // 2200
{
	int64_t ix_; (void)ix_;
	AlifDictObject* mp_ = (AlifDictObject*)_op;
	AlifObject* value_;

	if (!ALIFDICT_CHECK(_op)) {
		return nullptr;
	}

	ix_ = alifDict_lookup(mp_, _key, _hash, &value_);
	return value_;  // borrowed reference
}

int alifDictGetItemRef_knownHash(AlifObject* _op, AlifObject* _key, size_t _hash, AlifObject** _result) // 2227
{
	AlifObject* value_;

	int64_t ix_ = alifDict_lookup((AlifDictObject*)_op, _key, _hash, &value_);
	if (ix_ == DKIX_ERROR) {
		*_result = nullptr;
		return -1;
	}
	if (value_ == nullptr) {
		*_result = nullptr;
		return 0;  // missing _key
	}

	* _result = ALIF_NEWREF(value_);
	return 1;  // _key is present
}

int alifDict_getItemRef(AlifObject* _op, AlifObject* _key, AlifObject** _result) // 2255
{
	if (!ALIFDICT_CHECK(_op)) {
		*_result = nullptr;
		return -1;
	}

	size_t hash_;
	if (!ALIFUSTR_CHECKEXACT(_key)
		or
		(hash_ = ((AlifUStrObject*)_key)->hash_) == -1)
	{
		hash_ = alifObject_hash(_key);
		if (hash_ == -1) {
			*_result = nullptr;
			return -1;
		}
	}

	return alifDictGetItemRef_knownHash(_op, _key, hash_, _result);
}

static int setItem_take2_lockHeld(AlifDictObject* _mp, AlifObject* _key, AlifObject* _value) // 2386
{
	size_t hash_;
	if (!ALIFUSTR_CHECKEXACT(_key) || (hash_ = ((AlifUStrObject*)_key)->hash_) == -1) {
		hash_ = alifObject_hash(_key);
		if (hash_ == -1) {
			ALIF_DECREF(_key);
			ALIF_DECREF(_value);
			return -1;
		}
	}

	AlifInterpreter* interp = alifInterpreter_get();

	if (_mp->keys == ALIF_EMPTY_KEYS) {
		return insert_to_emptyDict(interp, _mp, _key, hash_, _value);
	}
	return insertDict(interp, _mp, _key, hash_, _value);
}


int alifDict_setItem_take2(AlifDictObject* _mp, AlifObject* _key, AlifObject* _value)// 2414
{
	int res_;
	//ALIF_BEGIN_CRITICAL_SECTION(mp);
	res_ = setItem_take2_lockHeld(_mp, _key, _value);
	//ALIF_END_CRITICAL_SECTION();
	return res_;
}

int alifDict_setItem(AlifObject* _op, AlifObject* _key, AlifObject* _value) // 2430
{
	if (!ALIFDICT_CHECK(_op)) {
		return -1;
	}

	return alifDict_setItem_take2((AlifDictObject*)_op,
		ALIF_NEWREF(_key), ALIF_NEWREF(_value));
}

// للمراجعة
static AlifIntT setItemTake2_lockHeld(AlifDictObject* _mp, AlifObject* _key, AlifObject* value) { // 2456

	AlifSizeT hash{};
	if (!ALIFUSTR_CHECKEXACT(_key) /* or (hash = uStr_getHash(_key)) == -1*/) {
		hash = alifObject_hash(_key);
		if (hash == -1) {
			ALIF_DECREF(_key);
			ALIF_DECREF(value);
			return -1;
		}
	}

	AlifInterpreter* interp = alifInterpreter_get();

	if (_mp->keys == ALIF_EMPTY_KEYS) {
		return insert_to_emptyDict(interp, _mp, _key, hash, value);
	}

	return insertDict(interp, _mp, _key, hash, value);
}

static AlifIntT setItem_lockHeld(AlifDictObject* _mp, AlifObject* _key, AlifObject* value) { // 2512
	return setItemTake2_lockHeld(_mp, ALIF_NEWREF(_key), ALIF_NEWREF(value));
}

static void deleteIndex_fromValues(AlifDictValues* _values, int64_t _ix) // 2484
{
	uint8_t* array_ = getInsertion_orderArray(_values);
	int size_ = _values->size;
	int i_;
	for (i_ = 0; array_[i_] != _ix; i_++) {
	}
	size_--;
	for (; i_ < size_; i_++) {
		array_[i_] = array_[i_ + 1];
	}
	_values->size = size_;
}

static int delItem_common(AlifDictObject* mp, size_t hash, int64_t ix,
	AlifObject* old_value, uint64_t new_version) // 2502
{
	AlifObject* old_key;


	int64_t hashpos = lookDict_index(mp->keys, hash, ix);

	mp->used--;
	mp->versionTag = new_version;
	if (ALIFDICT_HASSPLITTABLE(mp)) {
		mp->values->values[ix] = nullptr;

		deleteIndex_fromValues(mp->values, ix);
	}
	else {
		mp->keys->version = 0;
		dictKeys_set_index(mp->keys, hashpos, DKIX_DUMMY);
		if ((mp->keys->kind != Dict_Keys_General)) {
			AlifDictUnicodeEntry* ep = &dk_uStr_entries(mp->keys)[ix];
			old_key = ep->key;
			(ep->key = nullptr);
			(ep->value = nullptr);
		}
		else {
			AlifDictKeyEntry* ep = &dk_entries(mp->keys)[ix];
			old_key = ep->key;
			(ep->key = nullptr);
			(ep->value = nullptr);
			(ep->hash = 0);
		}
		ALIF_DECREF(old_key);
	}
	ALIF_DECREF(old_value);

	return 0;
}


int alifDict_delItem(AlifObject* _op, AlifObject* _key) // 2558
{
	size_t hash_;
	if (!ALIFUSTR_CHECKEXACT(_key) || (hash_ = ((AlifUStrObject*)_key)->hash_) == -1) {
		hash_ = alifObject_hash(_key);
		if (hash_ == -1)
			return -1;
	}

	return alifDictDelItem_knownHash(_op, _key, hash_);
}

static int delItem_knownHash_lockHeld(AlifObject* _op, AlifObject* _key, size_t _hash) // 2562
{
	int64_t ix_;
	AlifDictObject* mp_;
	AlifObject* oldValue;

	if (!ALIFDICT_CHECK(_op)) {
		return -1;
	}

	mp_ = (AlifDictObject*)_op;
	ix_ = alifDict_lookup(mp_, _key, _hash, &oldValue);
	if (ix_ == DKIX_ERROR)
		return -1;
	if (ix_ == DKIX_EMPTY || oldValue == nullptr) {
		return -1;
	}

	AlifInterpreter* interp_ = alifInterpreter_get();

	uint64_t newVersion{} ;
	//uint64_t newVersion = alifDict_NotifyEvent(
		//interp, AlifDict_EVENT_DELETED, mp, key, nullptr);
	return delItem_common(mp_, _hash, ix_, oldValue, newVersion);
}

int alifDictDelItem_knownHash(AlifObject* _op, AlifObject* _key, size_t _hash)
{
	int res;
	//ALIF_BEGIN_CRITICAL_SECTION(op);
	res = delItem_knownHash_lockHeld(_op, _key, _hash);
	//ALIF_END_CRITICAL_SECTION();
	return res;
}

int alifSubDict_next(AlifObject* _op, int64_t* _ppos, AlifObject** _pkey,
	AlifObject** _pvalue, size_t* _phash) // 2723
{
	int64_t i_;
	AlifDictObject* mp_;
	AlifObject* key_, * value_;
	size_t hash_;

	if (!ALIFDICT_CHECK(_op))
		return 0;

	mp_ = (AlifDictObject*)_op;
	i_ = *_ppos;
	if (ALIFDICT_HASSPLITTABLE(mp_)) {
		if (i_ < 0 || i_ >= mp_->used)
			return 0;
		int index = getIndex_fromOrder(mp_, i_);
		value_ = mp_->values->values[index];
		key_ = dk_uStr_entries(mp_->keys)[index].key;
		hash_ = ((AlifUStrObject*)key_)->hash_;
	}
	else {
		int64_t n_ = mp_->keys->nentries;
		if (i_ < 0 or i_ >= n_)
			return 0;
		if ((mp_->keys->kind != Dict_Keys_General)) {
			AlifDictUnicodeEntry* entryPtr = &dk_uStr_entries(mp_->keys)[i_];
			while (i_ < n_ && entryPtr->value == nullptr) {
				entryPtr++;
				i_++;
			}
			if (i_ >= n_)
				return 0;
			key_ = entryPtr->key;
			hash_ = ((AlifUStrObject*)entryPtr->key)->hash_;
			value_ = entryPtr->value;
		}
		else {
			AlifDictKeyEntry* entryPtr = &dk_entries(mp_->keys)[i_];
			while (i_ < n_ and entryPtr->value == nullptr) {
				entryPtr++;
				i_++;
			}
			if (i_ >= n_)
				return 0;
			key_ = entryPtr->key;
			hash_ = entryPtr->hash;
			value_ = entryPtr->value;
		}
	}
	*_ppos = i_ + 1;
	if (_pkey)
		*_pkey = key_;
	if (_pvalue)
		*_pvalue = value_;
	if (_phash)
		*_phash = hash_;
	return 1;
}

int alifDict_next(AlifObject* _op, int64_t* _ppos, AlifObject** _pkey, AlifObject** _pvalue) // 2806
{
	int res;
	//ALIF_BEGIN_CRITICAL_SECTION(op);
	res = alifSubDict_next(_op, _ppos, _pkey, _pvalue, nullptr);
	//ALIF_END_CRITICAL_SECTION();
	return res;
}

static void dict_dealloc(AlifObject* _self) // 3066
{
	AlifDictObject* mp_ = (AlifDictObject*)_self;
	AlifInterpreter* interp_ = alifInterpreter_get();
	ALIFSET_REFCNT(mp_, 1);
	//alifDict_NotifyEvent(interp, AlifDict_EVENT_DEALLOCATED, mp_, nullptr, nullptr);
	if (ALIF_REFCNT(mp_) > 1) {
		ALIFSET_REFCNT(mp_, ALIF_REFCNT(mp_) - 1);
		return;
	}
	ALIFSET_REFCNT(mp_, 0);
	AlifDictValues* values_ = mp_->values;
	AlifDictKeysObject* keys_ = mp_->keys;
	int64_t i_, n_;

	alifObject_gcUnTrack(mp_);
	//ALIFTRASHCAN_BEGIN(mp_, dict_dealloc)
		if (values_ != nullptr) {
			if (values_->embedded == 0) {
				for (i_ = 0, n_ = mp_->keys->nentries; i_ < n_; i_++) {
					ALIF_XDECREF(values_->values[i_]);
				}
				free_values(values_, false);
			}
			dictKeys_decref(interp_, keys_, false);
		}
		else if (keys_ != nullptr) {
			dictKeys_decref(interp_, keys_, false);
		}

	{
		ALIF_TYPE(mp_)->free_((AlifObject*)mp_);
	}
	//ALIF_TRASHCAN_END
}

static AlifObject* dict_subscript(AlifObject* _self, AlifObject* _key) // 3215
{
	AlifDictObject* mp_ = (AlifDictObject*)_self;
	int64_t ix_;
	size_t hash_;
	AlifObject* value_;

	if (!ALIFUSTR_CHECKEXACT(_key) || (hash_ = ((AlifUStrObject*)_key)->hash_) == -1) {
		hash_ = alifObject_hash(_key);
		if (hash_ == -1)
			return nullptr;
	}
	ix_ = alifDict_lookupThreadSafe(mp_, _key, hash_, &value_);
	if (ix_ == DKIX_ERROR)
		return nullptr;
	if (ix_ == DKIX_EMPTY || value_ == nullptr) {
		if (!ALIFDICT_CHECKEXACT(mp_)) {
			AlifObject* missing{}, * res;
			//missing = alifObject_lookupSpecial(
				//(AlifObject*)mp, &_Py_ID(__missing__));
			if (missing != nullptr) {
				res = alifObject_callOneArg(missing, _key);
				ALIF_DECREF(missing);
				return res;
			}
		}
		//alifErr_setKeyError(_key);
		return nullptr;
	}
	return value_;
}

static int dict_ass_sub(AlifObject* _mp, AlifObject* _v, AlifObject* _w)
{
	if (_w == nullptr)
		return alifDict_delItem(_mp, _v);
	else
		return alifDict_setItem(_mp, _v, _w);
}

static AlifObject* keysLock_held(AlifObject* _dict) // 3265
{

	if (_dict == nullptr || !ALIFDICT_CHECK(_dict)) {
		return nullptr;
	}
	AlifDictObject* mp_ = (AlifDictObject*)_dict;
	AlifObject* v_;
	int64_t n_;

again:
	n_ = mp_->used;
	v_ = alifNew_list(n_);
	if (v_ == nullptr)
		return nullptr;
	if (n_ != mp_->used) {
		ALIF_DECREF(v_);
		goto again;
	}

	int64_t j_ = 0, pos_ = 0;
	AlifObject* key_;
	while (alifSubDict_next((AlifObject*)mp_, &pos_, &key_, nullptr, nullptr)) {
		ALIFLIST_SETITEM(v_, j_, ALIF_NEWREF(key_));
		j_++;
	}
	return v_;
}

AlifObject* alifDict_keys(AlifObject* _dict) // 3310
{
	AlifObject* res_;
	//ALIF_BEGIN_CRITICAL_SECTION(_dict);
	res_ = keysLock_held(_dict);
	//ALIF_END_CRITICAL_SECTION();

	return res_;
}


int alifDict_contains(AlifObject* _op, AlifObject* _key)
{
	size_t hash_;

	if (!ALIFUSTR_CHECKEXACT(_key) or (hash_ = ((AlifUStrObject*)_key)->hash_) == -1) {
		hash_ = alifObject_hash(_key);
		if (hash_ == -1)
			return -1;
	}

	return alifDictContains_knownHash(_op, _key, hash_);
}

int alifDict_containsString(AlifObject* _op, const wchar_t* _key)
{
	AlifObject* keyObj = alifUStr_fromString(_key);
	if (keyObj == nullptr) {
		return -1;
	}
	int res_ = alifDict_contains(_op, keyObj);
	ALIF_DECREF(keyObj);
	return res_;
}

int alifDictContains_knownHash(AlifObject* _op, AlifObject* _key, size_t _hash)
{
	AlifDictObject* mp_ = (AlifDictObject*)_op;
	AlifObject* value_;
	int64_t ix_;

	ix_ = alifDict_lookup(mp_, _key, _hash, &value_);
	if (ix_ == DKIX_ERROR)
		return -1;
	if (ix_ != DKIX_EMPTY and value_ != nullptr) {
		return 1;
	}
	return 0;
}

AlifIntT alifDict_setItemLockHeld(AlifDictObject* dict, AlifObject* name, AlifObject* value) { // 6731
	if (value == nullptr) {
		//AlifSizeT hash;
		//if (!ALIFUSTR_CHECKEXACT(name) or (hash = uStr_getHash(name)) == -1) {
		//	hash = alifObject_hash(name);
		//	if (hash == -1)
		//		return -1;
		//}
		//return delItemknownHash_lockHeld((AlifObject*)dict, name, hash);
	}
	else {
		return setItem_lockHeld(dict, name, value);
	}
}

static inline AlifObject* ensure_nonManagedDict(AlifObject* obj, AlifObject** dictptr) { // 7219
	AlifDictKeysObject* cached;

	AlifObject* dict = *dictptr;
	if (dict == nullptr) {
		//AlifTypeObject* tp = ALIF_TYPE(obj);
		//if (alifType_hasFeature(tp, ALIFTPFLAGS_HEAPTYPE) && (cached = CACHED_KEYS(tp))) {
		//	AlifInterpreter* interp = alifInterpreter_get();
		//	dict = new_dict_with_shared_keys(interp, cached);
		//}
		//else {
		//	dict = alifNew_dict();
		//}
		//FT_ATOMIC_STORE_PTR_RELEASE(*dictptr, dict);
	}
	return dict;
}

AlifIntT alifObjectDict_setItem(AlifTypeObject* tp, AlifObject* obj,
	AlifObject** dictptr, AlifObject* key, AlifObject* value)
{ // 7270
	AlifObject* dict{};
	int res;

	dict = ensure_nonManagedDict(obj, dictptr);
	if (dict == nullptr) {
		return -1;
	}

	//ALIF_BEGIN_CRITICAL_SECTION(dict);
	res = alifDict_setItemLockHeld((AlifDictObject*)dict, key, value);
	//ALIF_END_CRITICAL_SECTION();
	return res;
}

//bool dict_contain(AlifObject* dict, AlifObject* _key) {
//
//    AlifDictObject* dictObj = (AlifDictObject*)dict;
//
//    AlifObject* value;
//    size_t hash;
//    if (_key->type_ == &_alifUStrType_) {
//        hash = ((AlifUStrObject*)_key)->hash_;
//    }
//    else {
//        hash = alifObject_hash(_key);
//    }
//
//    int64_t index = dict_lookupItem(dictObj, _key, hash, &value);
//
//    return (index != -1 && value != nullptr);
//
//}
//
//AlifDictObject* deleteItem_fromIndex(AlifDictObject* dict, int64_t index) {
//
//    for (int64_t i = index; i < dict->size_ - 1; ++i)
//    {
//        dict->items_[ i] = dict->items_[ i + 1];
//    }
//
//    dict->size_--;
//
//    //if (dict->size_ < (dict->capacity_ / 2)) {
//    //    
//    //    dict = (AlifDictObject*)dict_shrink(dict);
//
//    //}
//    return dict;
//}

//AlifDictObject* deletItem_common(AlifDictObject* dict, size_t hash, 
//    int64_t index) {
//
//
//    if (index != -1) {
//    
//        dict = deleteItem_fromIndex(dict, index);
//
//    }
//    else {
//        int64_t indexPos = -1;
//        for (int64_t i = 0; i < dict->size_; i++)
//        {
//            if (dict->items_[i].hash == hash) {
//                indexPos = i;
//                break;
//            }
//        }
//
//        if(indexPos != -1){
//            dict = deleteItem_fromIndex(dict, indexPos);
//        }
//    }
//    return dict;
//}
//
//AlifIntT dict_deleteItem(AlifDictObject* dict, AlifObject* _key) {
//
//    size_t hash;
//    if (_key->type_ == &_alifUStrType_) {
//        hash = ((AlifUStrObject*)_key)->hash_;
//    }
//    else {
//        hash = alifObject_hash(_key);
//    }
//
//    AlifObject* oldValue;
//    int64_t index = dict_lookupItem(dict, _key, hash, &oldValue);
//    if (index == -1 || oldValue == nullptr) {
//        return -1;
//    }
//
//    deletItem_common(dict, hash, index);
//    
//    return 1;
//}
// 
//AlifObject* alifDict_popKnownHash(AlifObject* dict, AlifObject* _key, size_t hash, AlifObject* deflt)
//{
//    int64_t ix_;
//    AlifObject* old_value;
//    AlifDictObject* _mp;
//
//    _mp = (AlifDictObject*)dict;
//
//    if (_mp->size_ == 0) {
//        if (deflt) {
//            return deflt;
//        }
//        return nullptr;
//    }
//    ix_ = dict_lookupItem(_mp, _key, hash, &old_value);
//    if (ix_ == -1)
//        return nullptr;
//    if ( old_value == nullptr) {
//        if (deflt) {
//            return deflt;
//        }
//        return nullptr;
//    }
//    deletItem_common(_mp, hash, ix_);
//
//    return old_value;
//}
//
//AlifObject* _alifDict_pop(AlifObject* dict, AlifObject* _key, AlifObject* deflt)
//{
//    size_t hash;
//
//    if (((AlifDictObject*)dict)->size_ == 0) {
//        if (deflt) {
//            return (deflt);
//        }
//        return nullptr;
//    }
//    if (!(_key->type_ == &_alifUStrType_) || (hash = ((AlifUStrObject*)_key)->hash_) == 0) {
//        hash = alifObject_hash(_key);
//        if (hash == -1)
//            return nullptr;
//    }
//    return alifDict_popKnownHash(dict, _key, hash, deflt);
//}
//
//
//static AlifObject* dict___contains__(AlifDictObject* self, AlifObject* _key)
//{
//    register AlifDictObject* _mp = self;
//    size_t hash;
//    int64_t ix_;
//    AlifObject* value;
//
//    if (!(_key->type_ == &_alifUStrType_) || (hash = ((AlifUStrObject*)_key)->hash_) == 0) {
//        hash = alifObject_hash(_key);
//        if (hash == -1)
//            return nullptr;
//    }
//    ix_ = dict_lookupItem(_mp, _key, hash, &value);
//    if ( value == nullptr)
//        return ALIF_FALSE;
//    return ALIF_TRUE;
//}
//
//static AlifObject* dict_get_impl(AlifDictObject* self, AlifObject* _key, AlifObject* default_value);
//
//static AlifObject* dict_get(AlifDictObject* self, AlifObject* const* args, int64_t nargs)
//{
//    AlifObject* return_value = nullptr;
//    AlifObject* _key;
//    AlifObject* default_value = ALIF_NONE;
//
//    if (!_alifArg_checkPositional(L"get", nargs, 1, 2)) {
//        goto exit;
//    }
//    _key = args[0];
//    if (nargs < 2) {
//        goto skip_optional;
//    }
//    default_value = args[1];
//skip_optional:
//    return_value = dict_get_impl(self, _key, default_value);
//
//exit:
//    return return_value;
//}
//
//static AlifObject* dict_get_impl(AlifDictObject* self, AlifObject* _key, AlifObject* default_value)
//{
//    AlifObject* val = nullptr;
//    size_t hash;
//    int64_t ix_;
//
//    if (!(_key->type_ == &_alifUStrType_) || (hash = ((AlifUStrObject*)_key)->hash_) == 0) {
//        hash = alifObject_hash(_key);
//        if (hash == -1)
//            return nullptr;
//    }
//    ix_ = dict_lookupItem(self, _key, hash, &val);
//    if (ix_ == -1)
//        return nullptr;
//    if ( val == nullptr) {
//        val = default_value;
//    }
//    return val;
//}
//
//static AlifObject* dict_popImpl(AlifDictObject* self, AlifObject* _key, AlifObject* default_value);
//
//static AlifObject* dict_pop(AlifDictObject* self, AlifObject* const* args, int64_t nargs)
//{
//    AlifObject* return_value = nullptr;
//    AlifObject* _key;
//    AlifObject* default_value = nullptr;
//
//    if (!_alifArg_checkPositional(L"pop", nargs, 1, 2)) {
//        goto exit;
//    }
//    _key = args[0];
//    if (nargs < 2) {
//        goto skip_optional;
//    }
//    default_value = args[1];
//skip_optional:
//    return_value = dict_popImpl(self, _key, default_value);
//
//exit:
//    return return_value;
//}
//
//static AlifObject* dict_popImpl(AlifDictObject* self, AlifObject* _key, AlifObject* default_value)
//{
//    return _alifDict_pop((AlifObject*)self, _key, default_value);
//}

//int dicts_equal(AlifDictObject* v, AlifDictObject* w) {
//
//    AlifDictObject* dict1 = v,
//        * dict2 = w;
//    int64_t size1 = dict1->size_,
//        size2 = dict2->size_;
//    int compare;
//    if (size1 != size2) {
//        return 0;
//    }
//
//    for (int64_t i = 0; i < size1; i++)
//    {
//        AlifDictValues item1 = dict1->items_[i];
//        AlifDictValues item2 = dict2->items_[i];
//
//        if (!(item1.hash == item2.hash)) {
//            return 0;
//        }
//
//        if (item1.value == nullptr || item2.value == nullptr) {
//            // error
//        }
//
//        compare = alifObject_richCompareBool(item1.value, item2.value, ALIF_EQ);
//
//        if (compare <= 0 ) {
//            return compare;
//        }
//    }
//
//    return 1;
//
//}
//
//AlifObject* dict_compare(AlifObject* v, AlifObject* w, int op) {
//
//    int compare;
//    AlifObject* res = nullptr;
//
//    if ((v->type_ !=& _alifDictType_) || (w->type_ != &_alifDictType_)) {
//        // error
//    }
//    if (op == ALIF_EQ || op == ALIF_NE) {
//    
//        compare = dicts_equal((AlifDictObject*)v, (AlifDictObject*)w);
//        if (compare < 0) {
//            return nullptr;
//        }
//        res = (compare == (op == ALIF_EQ)) ? ALIF_TRUE : ALIF_FALSE;
//
//    }
//    return res;
//
//}
//
//size_t dict_length(AlifDictObject* dict) {
//    return dict->size_;
//}
//
//AlifIntT dict_ass_sub(AlifDictObject* dict, AlifObject* _key, AlifObject* value) {
//
//
//    if (value == nullptr) {
//        return dict_deleteItem(dict, _key);
//    }
//    else {
//        return dict_setItem(dict, _key, value);
//    }
//
//}
//AlifObject* dict_subscript(AlifDictObject* dict, AlifObject* _key) {
//
//    size_t hash;
//    if (_key->type_ == &_alifUStrType_) {
//        hash = ((AlifUStrObject*)_key)->hash_;
//    }
//    else {
//        hash = alifObject_hash(_key);
//    }
//
//    int64_t index;
//    AlifObject* value{};
//
//    index = dict_lookupItem(dict, _key, hash, &value);
//    if (index == -1) {
//        return nullptr;
//    }
//    return value;
//
//}


static AlifMethodDef _mappMethods_[] = {
    //{L"__contains__", (AlifCFunction)dict___contains__, METHOD_O | METHOD_COEXIST},
    //{L"__getitem__", ALIFCFunction_CAST(dict_subscript), METHOD_O | METHOD_COEXIST},
    //{L"get", ALIFCFunction_CAST(dict_get), METHOD_FASTCALL},
    //{L"pop", ALIFCFunction_CAST(dict_pop), METHOD_FASTCALL},
    {nullptr,              nullptr}   /* sentinel */
};

AlifSequenceMethods _dictAsSequence_ = {
    0,                          
    0,                        
    0,                         
    0,                         
    0,                        
    0,                          
    0,                          
    //(ObjObjProc)dict_contain,                        
};

AlifMappingMethods _dictAsMapping_ = {
    //(LenFunc)dict_length,
	nullptr,
    (BinaryFunc)dict_subscript,
    (ObjObjArgProc)dict_ass_sub, 
};

AlifTypeObject _alifDictType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
    L"dict",
    sizeof(AlifDictObject),
    0,
    (Destructor)dict_dealloc,                 
    0,                                         
    0,                                       
    0,                                          
    0,                       
    0,                            
    & _dictAsSequence_,
    & _dictAsMapping_,
    (HashFunc)alifObject_hashNotImplemented,                                     
    0,                                          
    0,            
	alifObject_genericGetAttr,
    0,
    0,                                          
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC |
		ALIFTPFLAGS_BASETYPE | ALIFTPFLAGS_DICT_SUBCLASS |
		ALIFSUBTPFLAGS_MATCH_SELF | ALIFTPFLAGS_MAPPING,  
    0,  
    0,                            
    0,                              
    //dict_compare,                           
    0,                              
    0,                                          
    0,                     
    //_mappMethods_,
    0,                               
    0,                                          
    0,                                         
    0,                                         
    0,                                         
    0,                                          
    0,                                          
    0,                                          
    0,                                 
    0,
	0,
	0,
    alifObject_gcDel,                                  
    0,                      
};
