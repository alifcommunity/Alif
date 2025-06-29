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



/* Forward declaration */
static AlifIntT hashTable_rehash(AlifHashTableT*); // 63


static void alifSList_prepend(AlifSListT* _list, AlifSListItemT* _item) { // 74 
	_item->next = _list->head;
	_list->head = _item;
}



AlifUHashT alifHashTable_hashPtr(const void* _key) { // 92
	return (AlifUHashT)alif_hashPointerRaw(_key);
}


AlifIntT alifHashTable_compareDirect(const void* _key1, const void* _key2) { // 99
	return (_key1 == _key2);
}

static AlifUSizeT round_size(AlifUSizeT _s) { // 108
	AlifUSizeT i_{};
	if (_s < HASHTABLE_MIN_SIZE)
		return HASHTABLE_MIN_SIZE;
	i_ = 1;
	while (i_ < _s)
		i_ <<= 1;
	return i_;
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
	AlifUSizeT index = keyHash & (_ht->nbuckets - 1);
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

AlifIntT alifHashTable_set(AlifHashTableT* _ht, const void* _key, void* _value) { // 217
	AlifHashTableEntryT* entry{};

	entry = (AlifHashTableEntryT*)_ht->alloc.malloc(sizeof(AlifHashTableEntryT));
	if (entry == nullptr) {
		return -1;
	}

	entry->keyHash = _ht->hashFunc(_key);
	entry->key = (void*)_key;
	entry->value = _value;

	_ht->nentries++;
	if ((float)_ht->nentries / (float)_ht->nbuckets > HASHTABLE_HIGH) {
		if (hashTable_rehash(_ht) < 0) {
			_ht->nentries--;
			_ht->alloc.free(entry);
			return -1;
		}
	}

	AlifUSizeT index = entry->keyHash & (_ht->nbuckets - 1);
	alifSList_prepend(&_ht->buckets[index], (AlifSListItemT*)entry);
	return 0;
}


static AlifIntT hashTable_rehash(AlifHashTableT* _ht) { // 287
	AlifUSizeT newSize = round_size((AlifUSizeT)(_ht->nentries * HASHTABLE_REHASH_FACTOR));
	if (newSize == _ht->nbuckets) {
		return 0;
	}

	AlifUSizeT bucketsSize = newSize * sizeof(_ht->buckets[0]);
	AlifSListT* newBuckets = (AlifSListT*)_ht->alloc.malloc(bucketsSize);
	if (newBuckets == nullptr) {
		return -1;
	}
	memset(newBuckets, 0, bucketsSize);

	for (AlifUSizeT bucket = 0; bucket < _ht->nbuckets; bucket++) {
		AlifHashTableEntryT* entry = BUCKETS_HEAD(_ht->buckets[bucket]);
		while (entry != nullptr) {
			AlifHashTableEntryT* next = ENTRY_NEXT(entry);
			AlifUSizeT entry_index = entry->keyHash & (newSize - 1);

			alifSList_prepend(&newBuckets[entry_index], (AlifSListItemT*)entry);

			entry = next;
		}
	}

	_ht->alloc.free(_ht->buckets);
	_ht->nbuckets = newSize;
	_ht->buckets = newBuckets;
	return 0;
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
