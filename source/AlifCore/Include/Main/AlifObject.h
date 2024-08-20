#pragma once












#define ALIFOBJECT_VAR_HEAD      AlifVarObject base // 101




class AlifObject { // 140
public:
	uintptr_t ThreadID{};
	uint16_t padding{};
	AlifMutex mutex{};
	uint8_t gcBits{};
	uint32_t refLocal{};
	AlifSizeT refShared{};
	AlifTypeObject* type{};
};

#define ALIFOBJECT_CAST(_op) ALIF_CAST(AlifObject*, (_op)) // 155

class AlifVarObject { // 157
public:
	AlifObject base;
	AlifSizeT size;
};








/* -------------------------------------------------------------------------------------------------------------- */







class AlifTypeObject { // 147
public:
	ALIFOBJECT_VAR_HEAD;
	const char* name{};
	AlifSizeT basicSize{}, itemSize{};

};
