#pragma once


extern int alifOsOptErr;
extern AlifSizeT alifOsOptInd;
extern const wchar_t* alifOsOptArg;

extern void alifOS_resetGetOpt();

class AlifOSLongOption {
public:
	const wchar_t* name;
	int hasArg;
	int val;
};

extern int alifOS_getOpt(AlifSizeT, wchar_t* const*,int*);
