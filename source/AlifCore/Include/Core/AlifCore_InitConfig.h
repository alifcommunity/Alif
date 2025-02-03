#pragma once









extern void alifWStringList_clear(AlifWStringList*); // 59
extern AlifIntT alifWStringList_copy(AlifWStringList*, const AlifWStringList*); // 60


extern AlifObject* _alifWStringList_asList(const AlifWStringList*); // 64

/* ------------------------------------------- AlifArgv ------------------------------------------- */

class AlifArgv { // 69
public:
	AlifIntT argc;
	bool useBytesArgv;
	char* const* bytesArgv;
	wchar_t* const* wcharArgv;
};

AlifIntT alifArgv_asWStringList(AlifConfig*, AlifArgv*); // 76


/* ------------------------------------------ AlifConfig ------------------------------------------- */

enum ConfigInitEnum_ { // 149
	AlifConfig_Init_Alif = 1,
};

enum AlifConfigGIL_ { // 156
	AlifConfig_GIL_Default = -1,
	AlifConfig_GIL_Disable = 0,
	AlifConfig_GIL_Enable = 1,
};

extern AlifIntT alifConfig_copy(AlifConfig*, const AlifConfig*); // 171

extern AlifIntT alif_preInitFromConfig(AlifConfig*); //* alif
extern AlifIntT alifConfig_read(AlifConfig*); // 178
extern AlifIntT alifConfig_write(const AlifConfig*, class AlifDureRun*); // 179
