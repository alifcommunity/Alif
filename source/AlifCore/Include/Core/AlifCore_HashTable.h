#pragma once



typedef class AlifSListItemS {
public:
	AlifSListItemS* next{};
} AlifSListItemT;


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

typedef AlifHashTableEntryT* (*AlifHashTableGetEntryFunc)(AlifHashTableT*, const void*); // 47

class AlifHashTableT { // 60
public:
	AlifUSizeT nentries{}; // Total number of entries in the table
	AlifUSizeT nbuckets{};

	AlifHashTableGetEntryFunc getEntryFunc{};

};


void* alifHashTable_get(AlifHashTableT*, const void*); // 134
