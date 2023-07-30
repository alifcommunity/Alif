#include "Alif.h"
#include "alifcore_initConfig.h" // _AlifStatus_OK()

AlifStatus _alifConfig_setAlifArgv(AlifConfig* config, const _AlifArgv* args)
{
	//AlifStatus status = _Py_PreInitializeFromConfig(config, args);
	//if (_PyStatus_EXCEPTION(status)) {
	//	return status;
	//}

	return _alifArgv_asWstrList(args, &config->argv);
}


/* Set config.argv: decode argv using Py_DecodeLocale(). Pre-initialize Python
   if needed to ensure that encodings are properly configured. */
AlifStatus alifConfig_setBytesArgv(AlifConfig* config, int argc, char* const* argv)
{
	_AlifArgv args = {
		.argc = argc,
		.use_bytes_argv = 1,
		.bytes_argv = argv,
		.wchar_argv = nullptr
	};
	//return _pyConfig_setAlifArgv(config, &args);
	return (AlifStatus)0;
}


AlifStatus alifConfig_setArgv(AlifConfig* config, Alif_ssize_t argc, wchar_t* const* argv)
{
	_AlifArgv args = {
		.argc = argc,
		.use_bytes_argv = 0,
		.bytes_argv = nullptr,
		.wchar_argv = argv
	};

	return _alifConfig_setAlifArgv(config, &args);
}
