#include "Alif.h"
#include "alifcore_initConfig.h" // _AlifStatus_OK()

#include <filesystem>






/* Get run_filename absolute path */
static AlifStatus configRun_filenameAbspath(AlifConfig* config)
{
	wchar_t* absFilename;
	std::error_code error;
	std::filesystem::absolute(config->runFilename, error);
	if(error) {
	//if (_Py_abspath(config->run_filename, &abs_filename) < 0) {
		/* failed to get the absolute path of the command line filename:
		   ignore the error, keep the relative path */
		return _AlifStatus_OK();
	}

	config->runFilename = absFilename;

	return _AlifStatus_OK();
}

static AlifStatus configRead_cmdLine(AlifConfig* config)
{
	AlifStatus status;

	status = configRun_filenameAbspath(config);

	status = _AlifStatus_OK();

done:
	return status;
}

AlifStatus alifConfig_read(AlifConfig* config)
{
	AlifStatus status;

	status = configRead_cmdLine(config);

	status = _AlifStatus_OK();

done:
	return status;
}


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

