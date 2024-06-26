#pragma once

//bool alifDict_next(AlifObject* dict, int64_t* posPos, AlifObject** posKey, AlifObject** posValue, size_t* posHash); // issue here







AlifObject* alifDict_fromItems(AlifObject* const*, AlifSizeT, AlifObject* const*, AlifSizeT, AlifSizeT);


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
