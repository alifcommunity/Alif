#pragma once






/* ----- AlifStatus ------------------------------------------- */

class AlifStatus {
public:
	enum {
		AlifStatus_Type_Ok = 0,
		AlifStatus_Type_Error = 1,
		AlifStatus_Type_Exit = 2
	} type;
	const char* func;
	const char* errMsg;
	int exitcode;
};

























/* ----- AlifPreConfig ------------------------------------------- */

class AlifPreConfig {
public:
	int configInit;

	int parseArgv;

	int isolated;

	int useEnvironment;

	int configureLocale;

	int coerceCLocale;

	int coerceCLocaleWarn;

#ifdef MS_WINDOWS
	int legacyWindowsFSEncoding;
#endif

	int utf8Mode;

	int devMode;

	int allocator;
};






















































ALIFAPI_FUNC(void) alifPreConfig_initAlifConfig(AlifPreConfig*);
