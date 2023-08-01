#pragma once


/* ___________ AlifStatus ___________ */

/* تستخدم هذه الحالات للتحقق تقريباً من معظم أخطاء تهيئة لغة ألف */

#  define AlifStatus_GET_FUNC() __func__

// تستخدم هذه الحالة للتحقق في كل دالة ما إذا تمت بشكل صحيح أم لا
#define AlifStatus_OK() {.type = 0,} // 0 -> Ok , 1 -> Error ,  2 -> Exit

#define AlifStatus_ERR(ERR_MSG) (AlifStatus) {		\
			.type = 1,								\
			.func = AlifStatus_GET_FUNC(),			\
			.mesError = (ERR_MSG)					\
		}

#define AlifStatus_NO_MEMORY() AlifStatus_ERR(L"الحجز من الذاكرة فشل")

#define AlifStatus_EXIT(EXITCODE) (AlifStatus) {	\
			.type = 2,								\
			.exitCode = (EXITCODE)					\
		}

#define AlifStatus_IS_ERROR(err) ((err).type == 1)
#define AlifStatus_IS_EXIT(err) ((err).type == 2)
#define AlifStatus_EXCEPTION(err) ((err).type != 0)
#define AlifStatus_UPDATE_FUNC(err) do { (err).func = AlifStatus_GET_FUNC(); } while (0)


/* ___________ AlifArgv ___________ */

class AlifArgv { // صنف يحتوي عدد المعاملات الممررة من الطرفية وقيمها
public:
	alif_size_t argc;
	int useCharArgv;
	char* const* charArgv;
	wchar_t* const* wcharArgv;
};

AlifStatus alifArgv_asWstrList(const AlifArgv* args, AlifWideStringList* list);


/* ___________ AlifConfig ___________ */

extern AlifStatus alifConfig_read(AlifConfig*);
extern AlifStatus alifConfig_setAlifArgv(AlifConfig* config, const AlifArgv* args);
