
#define ALIFDICT_LOG_MINSIZE 3 // 115
#define ALIFDICT_MINSIZE 8 // 116

#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_Dict.h"
#include "AlifCore_GC.h"
#include "AlifCore_Object.h"



 // 204
#define LOCK_KEYS(_keys) alifMutex_lockFlags(&_keys->mutex, AlifLockFlags_::Alif_Lock_Dont_Detach)
#define UNLOCK_KEYS(_keys) ALIFMUTEX_UNLOCK(&_keys->mutex)



#define DECREF_KEYS(_dk)  alifAtomic_addSize(&_dk->dkRefCnt, -1)// 213

static void free_keysObject(AlifDictKeysObject*, bool); // 421


static inline void dictKeys_decref(AlifInterpreter* _interp,
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
			dictKeys_decref(_interp, _keys, false);
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
