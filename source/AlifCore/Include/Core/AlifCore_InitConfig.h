#pragma once


/* ------------------------------------------ AlifStatus ------------------------------------------ */

#ifdef _MSC_VER
   /* Visual Studio 2015 doesn't implement C99 __func__ in C */
#define ALIFSTATUS_GET_FUNC() __FUNCTION__
#else
#define ALIFSTATUS_GET_FUNC() __func__
#endif

#define ALIFSTATUS_OK() \
    {.type = AlifStatus::AlifStatus_Type_OK}
#define ALIFSTATUS_ERR(_errMsg) \
    { \
        .type = AlifStatus::AlifStatus_Type_ERROR, \
        .func = ALIFSTATUS_GET_FUNC(), \
        .errMsg = (_errMsg)		\
	}
#define ALIFSTATUS_NO_MEMORY_ERRMSG "فشل الحجز في الذاكرة"
#define ALIFSTATUS_NO_MEMORY() ALIFSTATUS_ERR(ALIFSTATUS_NO_MEMORY_ERRMSG)
#define ALIFSTATUS_IS_EXIT(err) \
    (err.type == AlifStatus::AlifStatus_Type_EXIT)
#define ALIFSTATUS_EXCEPTION(err) \
    (err.type != AlifStatus::AlifStatus_Type_OK)
#define ALIFSTATUS_IS_ERROR(err) \
    (err.type == AlifStatus::AlifStatus_Type_ERROR)


/* ---------------------------------------- AlifWStringList --------------------------------------- */

extern void _alifWStringList_clear(AlifWStringList*); // 59
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

AlifStatus alifArgv_asWStringList(AlifConfig*, AlifArgv*); // 76




/* -------------------------------------- Helper functions ------------------------------------- */

extern const char* _alif_getEnv(AlifIntT, const char*); // 88


/* --------------------------------------- AlifPreCmdline -------------------------------------- */

class AlifPreCmdline { // 102
public:
	AlifWStringList argv{};
	AlifWStringList xoptions{};
	AlifIntT isolated{};
	AlifIntT useEnvironment{};
	AlifIntT devMode{};
	AlifIntT warnDefaultEncoding{};
};


/* --------------------------------------- AlifPreConfig --------------------------------------- */


extern AlifStatus _alifPreConfig_initFromPreConfig(AlifPreConfig*, const AlifPreConfig*); // 136

extern AlifStatus _alifPreConfig_read(AlifPreConfig*, const AlifArgv*); // 142

/* ---------------------------------------- AlifConfig ----------------------------------------- */

enum ConfigInitEnum_ { // 149
	AlifConfig_Init_Alif = 1,
};

enum AlifConfigGIL_ { // 156
	AlifConfig_GIL_Default = -1,
	AlifConfig_GIL_Disable = 0,
	AlifConfig_GIL_Enable = 1,
};

extern AlifStatus alifConfig_copy(AlifConfig*, const AlifConfig*); // 171

extern AlifStatus _alifConfig_initPathConfig(AlifConfig*, AlifIntT); // 174

extern AlifStatus alif_preInitFromConfig(AlifConfig*); //* alif
extern AlifStatus _alifConfig_initImportConfig(AlifConfig*); // 177
extern AlifStatus alifConfig_read(AlifConfig*); // 178
extern AlifStatus alifConfig_write(const AlifConfig*, class AlifRuntime*); // 179
