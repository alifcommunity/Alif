#pragma once








union AlifUniqueIDEntry { // 29
	AlifUniqueIDEntry* next;
	AlifObject* obj;
};

class AlifUniqueIDPool { // 37
public:
	AlifMutex mutex{};
	AlifUniqueIDEntry* table{};
	AlifUniqueIDEntry* freelist{};
	AlifSizeT size{};
};


extern AlifSizeT _alifObject_assignUniqueId(AlifObject*); // 52

extern void _alifObject_releaseUniqueId(AlifSizeT); // 55





void alifType_incRefSlow(AlifHeapTypeObject*); // 68
