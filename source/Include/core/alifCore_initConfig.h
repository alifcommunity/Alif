#pragma once

// تستخدم هذه الحالة للتحقق في كل دالة ما إذا تمت بشكل صحيح أم لا
#define AlifStatus_OK() {.type = 0,} // 0 -> Ok , 1 -> Error ,  2 -> Exit

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
