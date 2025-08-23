#pragma once

extern AlifIntT _alifOSOptErr_;
extern AlifSizeT _alifOSOptInd_;
extern const wchar_t* _alifOSOptArg_;

extern void _alifOS_resetGetOpt(); // 12

class LongOption { // 14
public:
	const wchar_t* name;
	AlifIntT hasArg;
	AlifIntT val;
};

extern AlifIntT _alifOS_getOpt(AlifSizeT, wchar_t* const*, AlifIntT*); // 20
