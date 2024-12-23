#pragma once













class AlifMemberDef { // 41
public:
	const char* name{};
	AlifIntT type{};
	AlifSizeT offset{};
	AlifIntT flags{};
};




#define ALIF_T_OBJECT   6  // 59


#define ALIF_READONLY	1 // 83








/* ------------------------------------------------------------------------------------ */









AlifIntT alifDescr_isData(AlifObject*); // 62
