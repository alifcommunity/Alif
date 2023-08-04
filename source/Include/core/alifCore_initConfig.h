#pragma once


/* ___________ AlifStatus ___________ */

/* تستخدم هذه الحالات للتحقق تقريباً من معظم أخطاء تهيئة لغة ألف */

#  define ALIFSTATUS_GET_FUNC() __func__

// تستخدم هذه الحالة للتحقق في كل دالة ما إذا تمت بشكل صحيح أم لا
#define ALIFSTATUS_OK() {.type = 0,} // 0 -> Ok , 1 -> Error ,  2 -> Exit

#define ALIFSTATUS_ERR(_errMsg) (AlifStatus) {		\
			.type = 1,								\
			.func = ALIFSTATUS_GET_FUNC(),			\
			.mesError = (_errMsg)					\
		}

#define ALIFSTATUS_NO_MEMORY() ALIFSTATUS_ERR(L"فشل الحجز من الذاكرة")

#define ALIFSTATUS_EXIT(_exitCode) (AlifStatus) {	\
			.type = 2,								\
			.exitCode = (_exitCode)					\
		}

#define ALIFSTATUS_IS_ERROR(_error) ((_error).type == 1)
#define ALIFSTATUS_IS_EXIT(_error) ((_error).type == 2)
#define ALIFSTATUS_EXCEPTION(_error) ((_error).type != 0)
#define ALIFSTATUS_UPDATE_FUNC(_error) do { (_error).func = ALIFSTATUS_GET_FUNC(); } while (0)


/* ___________ AlifArgv ___________ */

class AlifArgv { // صنف يحتوي عدد المعاملات الممررة من الطرفية وقيمها
public:
	alif_size_t argc;
	int useCharArgv;
	char* const* charArgv;
	wchar_t* const* wcharArgv;
};

AlifStatus alifArgv_asWstrList(const AlifArgv* args, AlifWideStringList* list);


/* ___________ ALifPreConfig ___________ */

ALIFAPI_FUNC(void) alifPreConfig_initCompatConfig(AlifPreConfig*);
extern void alifPreConfig_initFromConfig(AlifPreConfig*, const AlifConfig*);
extern AlifStatus alifPreConfig_initFromPreConfig(AlifPreConfig*);
//extern AlifObject* alifPreConfig_asDict(const AlifPreConfig*);
//extern void alifPreConfig_getConfig(AlifPreConfig*, const AlifConfig*);
//extern AlifStatus alifPreConfig_read(AlifPreConfig*, const AlifArgv*);
//extern AlifStatus alifPreConfig_write(const AlifPreConfig*);


/* ___________ AlifConfig ___________ */

extern AlifStatus alifConfig_read(AlifConfig*);
extern AlifStatus alifConfig_setAlifArgv(AlifConfig* config, const AlifArgv* args);
