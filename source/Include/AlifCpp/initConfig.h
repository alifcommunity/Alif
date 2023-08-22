#pragma once












































/* --- AlifPreConfig ----------------------------------------------- */

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






















































void alifPreConfig_initAlifConfig(AlifPreConfig*);
