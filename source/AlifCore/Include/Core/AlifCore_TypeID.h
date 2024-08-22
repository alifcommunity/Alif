#pragma once








union AlifTypeIDEntry { // 29
	AlifTypeIDEntry* next;
	AlifHeapTypeObject* type;
};

class AlifTypeIDPool { // 37
public:
	AlifMutex mutex{};
	AlifTypeIDEntry* table{};
	AlifTypeIDEntry* freelist{};
	AlifSizeT size{};
};










void alifType_incRefSlow(AlifHeapTypeObject*); // 68
