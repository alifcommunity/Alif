#pragma once




























































/* ----- AlifArgv ------------------------------------------- */

class AlifArgv {
public:
	AlifSizeT argc;
	int useBytesArgv;
	char* const* bytesArgv;
	wchar_t* const* wcharArgv;
};




















































/* --- AlifPreConfig ------------------------------------------- */


ALIFAPI_FUNC(void) alifPreConfig_initCompatConfig(AlifPreConfig*);














/* ----- AlifConfig ------------------------------------------- */

enum AlifConfigInitEnum {
	AlifConfig_Init_Compat = 1,
	AlifConfig_Init_Alif = 2,
	AlifConfig_Init_Isolated = 3
};
