#include "Alif.h"
#include "alifcore_initConfig.h" // _AlifStatus_OK()

#include <filesystem>






/* Get run_filename absolute path */
static AlifStatus configRun_filenameAbspath(AlifConfig* _config)
{
	std::error_code error;
	std::filesystem::path absPath = std::filesystem::absolute(_config->runFilename, error);
	if(error) {
		return ALIFSTATUS_OK();
	}

	return ALIFSTATUS_OK();
}

static AlifStatus configRead_cmdLine(AlifConfig* _config)
{
	AlifStatus status;

	//status = configParse_cmdLine(config, &cmdline_warnoptions, &opt_index);
	_config->runFilename = _config->argv.items[1]; // temp

	status = configRun_filenameAbspath(_config);

	status = ALIFSTATUS_OK();

	return status;
}

AlifStatus alifConfig_read(AlifConfig* _config)
{
	AlifStatus status;

	status = configRead_cmdLine(_config);

	status = ALIFSTATUS_OK();


	return status;
}


AlifStatus alifConfig_setAlifArgv(AlifConfig* _config, const AlifArgv* _args)
{

	return alifArgv_asWstrList(_args, &_config->argv);
}


AlifStatus alifConfig_setCharArgv(AlifConfig* _config, alif_size_t _argc, char* const* _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useCharArgv = 1,
		.charArgv = _argv,
		.wcharArgv = nullptr
	};
	//return _alifConfig_setAlifArgv(config, &args);
	return (AlifStatus)0;
}


AlifStatus alifConfig_setArgv(AlifConfig* _config, alif_size_t _argc, wchar_t* const* _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useCharArgv = 0,
		.charArgv = nullptr,
		.wcharArgv = _argv
	};

	return alifConfig_setAlifArgv(_config, &args);
}


