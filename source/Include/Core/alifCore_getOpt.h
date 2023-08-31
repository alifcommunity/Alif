#pragma once






extern int alifOSOptErr;
extern AlifSizeT alifOSOptind;
extern const wchar_t* alifOSOptArg;

extern void alifOS_resetGetOpt(void);

class AlifOSLongOption{
public:
	const wchar_t* name;
	int hasArg;
	int val;
};
extern int alifOS_getOpt(AlifSizeT, wchar_t* const*, int*);
