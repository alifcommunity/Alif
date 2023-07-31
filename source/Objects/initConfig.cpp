#include "Alif.h"
#include "alifcore_initConfig.h" // _AlifStatus_OK()

AlifStatus alifConfig_setAlifArgv(AlifConfig* config, const AlifArgv* args)
{

	return alifArgv_asWstrList(args, &config->argv);
}


AlifStatus alifConfig_setCharArgv(AlifConfig* config, Alif_ssize_t argc, char* const* argv)
{
	AlifArgv args = {
		.argc = argc,
		.useCharArgv = 1,
		.charArgv = argv,
		.wcharArgv = nullptr
	};
	//return _alifConfig_setAlifArgv(config, &args);
	return (AlifStatus)0;
}


AlifStatus alifConfig_setArgv(AlifConfig* config, Alif_ssize_t argc, wchar_t* const* argv)
{
	AlifArgv args = {
		.argc = argc,
		.useCharArgv = 0,
		.charArgv = nullptr,
		.wcharArgv = argv
	};

	return alifConfig_setAlifArgv(config, &args);
}
