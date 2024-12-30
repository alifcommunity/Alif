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


extern void _alifType_assignId(AlifHeapTypeObject*); // 52

extern void alifType_releaseID(AlifHeapTypeObject*); // 55





void alifType_incRefSlow(AlifHeapTypeObject*); // 68
