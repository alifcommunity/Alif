#include "alif.h"

#include "AlifCore_HashTable.h"
#include "AlifCore_Hash.h"



// 51
#define HASHTABLE_MIN_SIZE 16
#define HASHTABLE_HIGH 0.50
#define HASHTABLE_LOW 0.10
#define HASHTABLE_REHASH_FACTOR 2.0 / (HASHTABLE_LOW + HASHTABLE_HIGH)

// 56
#define BUCKETS_HEAD(_slist) \
        ((AlifHashTableEntryT *)ALIF_SLIST_HEAD(&(_slist)))
#define TABLE_HEAD(_ht, _bucket) \
        ((AlifHashTableEntryT *)ALIF_SLIST_HEAD(&(_ht)->buckets[_bucket]))
#define ENTRY_NEXT(_entry) \
        ((AlifHashTableEntryT *)ALIF_SLIST_ITEM_NEXT(_entry))





AlifUHashT alifHashTable_hashPtr(const void* _key) { // 92
	return (AlifUHashT)alif_hashPointerRaw(_key);
}


AlifIntT alifHashTable_compareDirect(const void* _key1, const void* _key2) { // 99
	return (_key1 == _key2);
}


AlifHashTableEntryT* alifHashTable_getEntryGeneric(AlifHashTableT* ht, const void* key) { // 139
	AlifUHashT keyHash = ht->hashFunc(key);
	AlifUSizeT index = keyHash & (ht->nbuckets - 1);
	AlifHashTableEntryT* entry = TABLE_HEAD(ht, index);
	while (1) {
		if (entry == nullptr) {
			return nullptr;
		}
		if (entry->keyHash == keyHash and ht->compareFunc(key, entry->key)) {
			break;
		}
		entry = ENTRY_NEXT(entry);
	}
	return entry;
}


static AlifHashTableEntryT* alifHashTable_getEntryPtr(AlifHashTableT* _ht, const void* _key) { // 161
	AlifUHashT keyHash = alifHashTable_hashPtr(_key);
	size_t index = keyHash & (_ht->nbuckets - 1);
	AlifHashTableEntryT* entry = TABLE_HEAD(_ht, index);
	while (1) {
		if (entry == nullptr) {
			return nullptr;
		}
		if (entry->key == _key) {
			break;
		}
		entry = ENTRY_NEXT(entry);
	}
	return entry;
}



void* alifHashTable_get(AlifHashTableT* _ht, const void* _key) { // 255
	AlifHashTableEntryT* entry = _ht->getEntryFunc(_ht, _key);
	if (entry != nullptr) {
		return entry->value;
	}
	else {
		return nullptr;
	}
}






AlifHashTableT* alifHashTable_newFull(
	AlifHashTableHashFunc _hashFunc,
	AlifHashTableCompareFunc _compareFunc,
	AlifHashTableDestroyFunc _keyDestroyFunc,
	AlifHashTableDestroyFunc _valueDestroyFunc,
	AlifHashTableAllocatorT* _allocator) { // 322

	AlifHashTableAllocatorT alloc{};
	if (_allocator == nullptr) {
		alloc.malloc = alifMem_dataAlloc;
		alloc.free = alifMem_dataFree;
	}
	else {
		alloc = *_allocator;
	}

	AlifHashTableT* ht = (AlifHashTableT*)alloc.malloc(sizeof(AlifHashTableT));
	if (ht == nullptr) {
		return ht;
	}

	ht->nbuckets = HASHTABLE_MIN_SIZE;
	ht->nentries = 0;

	AlifUSizeT buckets_size = ht->nbuckets * sizeof(ht->buckets[0]);
	ht->buckets = (AlifSListT*)alloc.malloc(buckets_size);
	if (ht->buckets == nullptr) {
		alloc.free(ht);
		return nullptr;
	}
	memset(ht->buckets, 0, buckets_size);

	ht->getEntryFunc = alifHashTable_getEntryGeneric;
	ht->hashFunc = _hashFunc;
	ht->compareFunc = _compareFunc;
	ht->keyDestroyFunc = _keyDestroyFunc;
	ht->valueDestroyFunc = _valueDestroyFunc;
	ht->alloc = alloc;
	if (ht->hashFunc == alifHashTable_hashPtr
		and ht->compareFunc == alifHashTable_compareDirect)
	{
		ht->getEntryFunc = alifHashTable_getEntryPtr;
	}
	return ht;
}
