#pragma once

/* ------------------------------------------- AlifArgv ------------------------------------------- */

class AlifArgv { // 69
public:
	AlifIntT argc;
	bool useBytesArgv;
	char* const* bytesArgv;
	wchar_t* const* wcharArgv;
};

//AlifIntT alifArgv_asStringList(AlifConfig*, AlifArgv*); // 76


/* ------------------------------------------ AlifConfig ------------------------------------------- */

enum ConfigInitEnum_ { // 149
	AlifConfig_Init_Alif = 1,
};


AlifIntT alif_preInitFromConfig(AlifConfig*); // alif
//AlifIntT alifConfig_read(AlifConfig*); // 178
//AlifIntT alifConfig_write(const AlifConfig*, class AlifDureRun*); // 179
