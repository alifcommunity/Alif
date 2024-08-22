#pragma once




enum AlifErrorHandler_ { // 26
	Alif_Error_Unknown = 0,
	Alif_Error_Strict,
	Alif_Error_SurrogateEscape,
	Alif_Error_Replace,
	Alif_Error_Ignore,
	Alif_Error_BackSlashReplace,
	Alif_Error_SurrogatePass,
	Alif_Error_XMLCharRefReplace,
	Alif_Error_Other
};


AlifErrorHandler_ alif_getErrorHandler(const char*); // 39


AlifIntT alif_decodeLocaleEx(const char*, wchar_t**, AlifUSizeT*,
	const char**, AlifIntT, AlifErrorHandler_); // 42








extern AlifIntT alif_decodeUTF8Ex(const char*, AlifSizeT,
	wchar_t**, AlifUSizeT*, const char**, AlifErrorHandler_); // 210

//wchar_t* alifUniversal_newLineFGetsWithSize(wchar_t*, int, FILE*, AlifSizeT*);
