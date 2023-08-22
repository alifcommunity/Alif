#include "alif.h"
//#include "alifCore_fileUtils.h"
//#include "alifCore_getOpt.h"
#include "alifCore_initConfig.h"
//#include "alifCore_alifLifeCycle.h"
//#include "alifCore_alifMem.h"
//#include "alifCore_runtime.h"

#include <locale.h>
#include <stdlib.h>













































































































































































































































































/* ----- AlifPreConfig ------------------------------------------- */


void alifPreConfig_initCompatConfig(AlifPreConfig* _config)
{
	memset(_config, 0, sizeof(*_config));

	_config->configInit = (int)AlifConfig_Init_Compat;
	_config->parseArgv = 0;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->configureLocale = 1;
	_config->utf8Mode = 0;
	_config->coerceCLocale = 0;
	_config->coerceCLocaleWarn = 0;
	_config->devMode = -1;
	_config->allocator = AlifMem_Allocator_Not_Set;
#ifdef MS_WINDOWS
	_config->legacyWindowsFSEncoding = -1;
#endif
}










void alifPreConfig_initAlifConfig(AlifPreConfig* _config)
{
	alifPreConfig_initCompatConfig(_config);

	_config->configInit = (int)AlifConfig_Init_Alif;
	_config->isolated = 0;
	_config->parseArgv = 1;
	_config->useEnvironment = 1;
	_config->coerceCLocale = -1;
	_config->coerceCLocaleWarn = -1;
	_config->utf8Mode = -1;
#ifdef MS_WINDOWS
	_config->legacyWindowsFSEncoding = 0;
#endif
}
