#pragma once













class AlifMemberDef { // 41
public:
	const char* name{};
	AlifIntT type{};
	AlifSizeT offset{};
	AlifIntT flags{};
};

AlifIntT alifDescr_isData(AlifObject*); // 62
