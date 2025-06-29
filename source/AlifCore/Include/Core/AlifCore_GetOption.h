#pragma once

extern AlifSizeT _optIdx_;
extern const wchar_t* _optArg_;

extern void alif_resetGetOption(); // 12

class LongOption { // 14
public:
	const wchar_t* name;
	AlifIntT hasArg;
	AlifIntT val;
};

extern AlifIntT alif_getOption(AlifSizeT, wchar_t* const*, AlifIntT*); // 20
