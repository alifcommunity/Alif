#pragma once

/* ------------------------------------------- AlifArgv ------------------------------------------- */

class AlifArgv {
public:
	AlifIntT argc;
	bool useBytesArgv;
	char* const* bytesArgv;
	wchar_t* const* wcharArgv;
};



/* ------------------------------------------ AlifConfig ------------------------------------------- */

enum ConfigInitEnum {
	AlifConfig_Init_Alif = 1,
};



AlifIntT alifArgv_asStringList(AlifConfig*, AlifArgv*);
AlifIntT alifConfig_read(AlifConfig*);
AlifIntT alifConfig_write(const AlifConfig*, class AlifDureRun*);
