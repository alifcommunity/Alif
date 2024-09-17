#include "alif.h"

#include "AlifCore_HashTable.h"

void* alifHashTable_get(AlifHashTableT* _ht, const void* _key) { // 255
	AlifHashTableEntryT* entry = _ht->getEntryFunc(_ht, _key);
	if (entry != nullptr) {
		return entry->value;
	}
	else {
		return nullptr;
	}
}
