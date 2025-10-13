#pragma once



typedef class AlifSListItemS { // 13
public:
	AlifSListItemS* next{};
} AlifSListItemT;


class AlifSListT { // 17
public:
	AlifSListItemT* head{};
};


#define ALIF_SLIST_ITEM_NEXT(_item) ALIF_RVALUE(((AlifSListItemT *)(_item))->next) // 21

#define ALIF_SLIST_HEAD(_slist) ALIF_RVALUE(((AlifSListT *)(_slist))->head) // 23


class AlifHashTableEntryT { // 28
public:
	AlifSListItemT alifSListItem{};
	AlifUHashT keyHash{};
	void* key{};
	void* value{};
};

// Forward declaration
class AlifHashTableT; // 41 
typedef class AlifHashTableT AlifHashTableT; // 42

typedef AlifUHashT(*AlifHashTableHashFunc) (const void*);
typedef AlifIntT(*AlifHashTableCompareFunc) (const void*, const void*);
typedef void (*AlifHashTableDestroyFunc) (void*);
typedef AlifHashTableEntryT* (*AlifHashTableGetEntryFunc)(AlifHashTableT*, const void*); // 47

class AlifHashTableAllocatorT { // 50
public:
	void* (*malloc) (AlifUSizeT _size);
	void (*free) (void* _ptr);
};

class AlifHashTableT { // 60
public:
	AlifUSizeT nentries{}; // Total number of entries in the table
	AlifUSizeT nbuckets{};

	AlifSListT* buckets{};

	AlifHashTableGetEntryFunc getEntryFunc{};
	AlifHashTableHashFunc hashFunc{};
	AlifHashTableCompareFunc compareFunc{};
	AlifHashTableDestroyFunc keyDestroyFunc{};
	AlifHashTableDestroyFunc valueDestroyFunc{};
	AlifHashTableAllocatorT alloc{};
};


AlifUHashT _alifHashTable_hashPtr(const void*); // 79

AlifIntT _alifHashTable_compareDirect(const void*, const void*); // 82

AlifHashTableT* _alifHashTable_newFull(AlifHashTableHashFunc,
	AlifHashTableCompareFunc, AlifHashTableDestroyFunc,
	AlifHashTableDestroyFunc, AlifHashTableAllocatorT*); // 86

AlifIntT _alifHashTable_set(AlifHashTableT*, const void*, void*); // 114


static inline AlifHashTableEntryT* _alifHashTable_getEntry(AlifHashTableT* _ht, const void* _key) { // 123
	return _ht->getEntryFunc(_ht, _key);
}



void* alifHashTable_get(AlifHashTableT*, const void*); // 134
