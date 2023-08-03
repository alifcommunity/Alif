#include "Alif.h"
#include "alifcore_initConfig.h"    // _AlifArgv



AlifStatus alifArgv_asWstrList(const AlifArgv* args, AlifWideStringList* list)
{
	AlifWideStringList wArgv = { .length = 0, .items = nullptr };
	if (args->useCharArgv) {

	}
	else {
		wArgv.length = args->argc;
		wArgv.items = (wchar_t**)args->wcharArgv;

		*list = wArgv;
	}


	return ALIFSTATUS_OK();
}


void alifPreConfig_initConfig(AlifPreConfig* config)
{
	alifPreConfig_initCompatConfig(config);

	config->_config_init = 2; // INIT_COMPAT = 1, INIT_ALIF = 2, INIT_ISOLATED = 3
	config->isolated = 0;
	config->parse_argv = 1;
	config->use_environment = 1;
	config->cpp_locale = -1;
	config->utf8_mode = -1;
#ifdef MS_WINDOWS
	config->legacy_windows_fs_encoding = 0;
#endif
}
