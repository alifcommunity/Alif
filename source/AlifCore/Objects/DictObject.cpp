
#define ALIFDICT_LOG_MINSIZE 3 // 115
#define ALIFDICT_MINSIZE 8 // 116

#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_Dict.h"
#include "AlifCore_GC.h"
#include "AlifCore_Object.h"


#define DECREF_KEYS(dk)  alifAtomic_addSize(&_dk->dkRefCnt, -1)// 213

static void free_keysObject(AlifDictKeysObject*, bool); // 421


static inline void dictKeys_decref(AlifInterpreter* _interp, AlifDictKeysObject* _dk, bool _useqsbr) { // 431
	if (alifAtomic_loadSizeRelaxed(&_dk->dkRefCnt) == ALIF_IMMORTAL_REFCNT) {
		return;
	}
#ifdef ALIF_REF_DEBUG
	alif_DecRefTotal(alifThread_get());
#endif
	if (DECREF_KEYS(_dk) == 1) {
		if (DK_IS_UNICODE(_dk)) {
			AlifDictUnicodeEntry* entries = dk_UStr_entries(_dk);
			AlifSizeT i, n;
			for (i = 0, n = _dk->dkNentries; i < n; i++) {
				ALIF_XDECREF(entries[i].key);
				ALIF_XDECREF(entries[i].value);
			}
		}
		else {
			AlifDictKeyEntry* entries = dk_entries(_dk);
			AlifSizeT i, n;
			for (i = 0, n = _dk->dkNentries; i < n; i++) {
				ALIF_XDECREF(entries[i].key);
				ALIF_XDECREF(entries[i].value);
			}
		}
		free_keysObject(_dk, _useqsbr);
	}
}

static void free_keysObject(AlifDictKeysObject* _keys, bool _useqsbr) { // 804
	if (_useqsbr) {
		//alifMem_freeDelayed(_keys);
		alifMem_objFree(_keys);
		return;
	}
	if (DK_LOG_SIZE(_keys) == ALIFDICT_LOG_MINSIZE && _keys->dkKind == Dict_Kyes_UStr) {
		//ALIF_FREELIST_FREE(dictKeys, DICT, _keys, (FreeFunc)alifMem_objFree); // ظهر خطا وسيتم العمل عليه لاحقا
	}
	else {
		alifMem_objFree(_keys);
	}
}

static inline void free_values(AlifDictValues* _values, bool _useqsbr) { // 848
	if (_useqsbr) {
		//alifMem_freeDelayed(_values);
		alifMem_objFree(_values);
		return;
	}
	alifMem_objFree(_values);
}

static AlifObject* new_dict(AlifInterpreter* interp,
	AlifDictKeysObject* keys, AlifDictValues* values,
	AlifSizeT used, int free_values_on_failure) { // 860
	AlifDictObject* mp_ = ALIF_FREELIST_POP(AlifDictObject, dicts);
	if (mp_ == NULL) {
		mp_ = ALIFOBJECT_GC_NEW(AlifDictObject, &_alifDictType_);
		if (mp_ == NULL) {
			dictKeys_decref(interp, keys, false);
			if (free_values_on_failure) {
				free_values(values, false);
			}
			return NULL;
		}
	}
	mp_->keys = keys;
	mp_->values = values;
	mp_->used = used;
	mp_->versionTag = DICT_NEXT_VERSION(interp);
	return (AlifObject*)mp_;
}


AlifTypeObject _alifDictType_ = {
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "قاموس",
	.basicSize = sizeof(AlifDictObject),
};
