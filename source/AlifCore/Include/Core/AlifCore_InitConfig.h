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
#define ALIFSTATUS_IS_EXIT(_err) \
    (_err.type == AlifStatus::AlifStatus_Type_EXIT)
#define ALIFSTATUS_EXCEPTION(_err) \
    (_err.type != AlifStatus::AlifStatus_Type_OK)
#define ALIFSTATUS_IS_ERROR(_err) \
    (_err.type == AlifStatus::AlifStatus_Type_ERROR)
#define ALIFSTATUS_EXIT(_exitcode) \
    { .type = AlifStatus::AlifStatus_Type_EXIT, \
        .exitcode = (_exitcode) }


/* ---------------------------------------- AlifWStringList --------------------------------------- */

#define ALIFWIDESTRINGLIST_INIT {.length = 0, .items = nullptr}

extern void _alifWStringList_clear(AlifWStringList*); // 59
extern AlifIntT _alifWStringList_copy(AlifWStringList*, const AlifWStringList*); // 60

extern AlifStatus _alifWStringList_extend(AlifWStringList*, const AlifWStringList*); // 62
extern AlifObject* _alifWStringList_asList(const AlifWStringList*); // 64

/* ------------------------------------------- AlifArgv ------------------------------------------- */

class AlifArgv { // 69
public:
	AlifSizeT argc{};
	AlifIntT useBytesArgv{};
	char* const* bytesArgv{};
	wchar_t* const* wcharArgv{};
};

AlifStatus _alifArgv_asWStrList(const AlifArgv*, AlifWStringList*); // 76




/* -------------------------------------- Helper functions ------------------------------------- */

extern const wchar_t* _alif_getXOption(const AlifWStringList*, const wchar_t*); // 85

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

#define ALIFPRECMDLINE_INIT \
    {	.isolated = -1, \
		.useEnvironment = -1, \
        .devMode = -1 }

extern void _alifPreCMDLine_clear(AlifPreCmdline*); // 118
extern AlifStatus _alifPreCMDLine_setArgv(AlifPreCmdline*, const AlifArgv*); // 119
extern AlifStatus _alifPreCMDLine_setConfig(const AlifPreCmdline*, AlifConfig*); // 121
extern AlifStatus _alifPreCMDLine_read(AlifPreCmdline*, const AlifPreConfig*); // 124

/* --------------------------------------- AlifPreConfig --------------------------------------- */

extern void _alifPreConfig_initFromConfig(AlifPreConfig*, const AlifConfig*); // 133
extern AlifStatus _alifPreConfig_initFromPreConfig(AlifPreConfig*, const AlifPreConfig*); // 136

extern void _alifPreConfig_getConfig(AlifPreConfig*, const AlifConfig*); // 140
extern AlifStatus _alifPreConfig_read(AlifPreConfig*, const AlifArgv*); // 142

extern AlifStatus _alifPreConfig_write(const AlifPreConfig*); // 144

/* ---------------------------------------- AlifConfig ----------------------------------------- */

enum ConfigInitEnum_ { // 149
	AlifConfig_Init_COMPAT = 1,
	AlifConfig_Init_ALIF = 2,
	AlifConfig_Init_ISOLATED = 3,
};

enum AlifConfigGIL_ { // 156
	AlifConfig_GIL_Default = -1,
	AlifConfig_GIL_Disable = 0,
	AlifConfig_GIL_Enable = 1,
};

extern AlifStatus _alifConfig_copy(AlifConfig*, const AlifConfig*); // 171

extern AlifStatus _alifConfig_initPathConfig(AlifConfig*, AlifIntT); // 174

extern AlifStatus _alifConfig_initImportConfig(AlifConfig*); // 177
extern AlifStatus _alifConfig_read(AlifConfig*, AlifIntT); // 178
extern AlifStatus alifConfig_write(const AlifConfig*, class AlifRuntime*); // 179
