#include "Alif.h"
#include "alifcore_initConfig.h"    // AlifArgv



AlifStatus alifArgv_asWstrList(const AlifArgv* _args, AlifWideStringList* _list)
{
	AlifWideStringList wArgv = { .length = 0, .items = nullptr };
	if (_args->useCharArgv) {

	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;

		*_list = wArgv;
	}


	return ALIFSTATUS_OK();
}

/* ___________ AlifPreConfig ___________ */

/* يجب مراجعة الدالتين التاليتين
	وذلك بسبب أنه قد تكوم استخدمت للتوافقية بين النسخ
	القديمة والجديدة فقط وهذا ما لا نحتاجه */

void alifPreConfig_initCompatConfig(AlifPreConfig* _config)
{
	_config->configInit = 1; // INIT_COMPAT = 1, INIT_ALIF = 2, INIT_ISOLATED = 3
	_config->parseArgv = 0;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->configureLocale = 1;
	_config->utf8Mode = 0;
	_config->cppLocale = 0;
	_config->allocator = ALIFMEM_ALLOCATOR_NOT_SET;
#ifdef MS_WINDOWS
	_config->EncodingLegacyWindowsFS = -1;
#endif
}

void alifPreConfig_initAlifConfig(AlifPreConfig* _config)
{
	alifPreConfig_initCompatConfig(_config);

	_config->configInit = 2; // INIT_COMPAT = 1, INIT_ALIF = 2, INIT_ISOLATED = 3
	_config->isolated = 0;
	_config->parseArgv = 1;
	_config->useEnvironment = 1;
	_config->cppLocale = -1;
	_config->utf8Mode = -1;
#ifdef MS_WINDOWS
	_config->EncodingLegacyWindowsFS = 0;
#endif
}


AlifStatus alifPreConfig_initFromPreConfig(AlifPreConfig* _config)
{
	alifPreConfig_initAlifConfig(_config);
	return ALIFSTATUS_OK();
}
