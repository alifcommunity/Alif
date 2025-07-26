#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Runtime.h"


AlifStatus alifArgv_asWStringList(AlifConfig* _config, AlifArgv* _args) { // 78


	AlifWStringList wArgv = { .length = 0, .items = nullptr };
	if (_args->useBytesArgv) {
		wArgv.items = (wchar_t**)alifMem_dataAlloc(_args->argc * sizeof(wchar_t*));
		if (wArgv.items == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}

		for (AlifIntT i = 0; i < _args->argc; i++) {
			AlifUSizeT len{};
			wchar_t* arg = alif_decodeLocale(_args->bytesArgv[i], &len);
			if (arg == nullptr) {
				_alifWStringList_clear(&wArgv);
				DECODE_LOCALE_ERR("command line arguments", len);
			}
			wArgv.items[i] = arg;
			wArgv.length++;
		}

		_alifWStringList_clear(&_config->argv);
		_config->argv = wArgv;
	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;
		if (alifWStringList_copy(&_config->argv, &wArgv)) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	return ALIFSTATUS_OK();
}







/* ---------------------------------- AlifPreConfig ---------------------------------- */


void _alifPreConfig_initCompatConfig(AlifPreConfig* _config) { // 280
	memset(_config, 0, sizeof(*_config));

	//_config->configInit = (AlifIntT)AlifConfig_Init_COMPAT;
	_config->parseArgv = 0;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->configureLocale = 1;

	_config->utf8Mode = 0;
	_config->coerceCLocale = 0;
	_config->coerceCLocaleWarn = 0;

	_config->devMode = -1;
#ifdef _WINDOWS
	_config->legacyWindowsFSEncoding = -1;
#endif
}


void alifPreConfig_initAlifConfig(AlifPreConfig* _config) { // 311
	_alifPreConfig_initCompatConfig(_config);

	//_config->configInit = (AlifIntT)AlifConfig_Init_ALIF;
	_config->isolated = 0;
	_config->parseArgv = 1;
	_config->useEnvironment = 1;
	_config->coerceCLocale = -1;
	_config->coerceCLocaleWarn = -1;
	_config->utf8Mode = -1;
#ifdef _WINDOWS
	_config->legacyWindowsFSEncoding = 0;
#endif
}








const char* _alif_getEnv(AlifIntT useEnvironment, const char* name) { // 526
	if (!useEnvironment) {
		return nullptr;
	}

	const char* var = getenv(name);
	if (var and var[0] != '\0') {
		return var;
	}
	else {
		return nullptr;
	}
}
