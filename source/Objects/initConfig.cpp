#include "Alif.h"
#include "alifcore_initConfig.h" // _AlifStatus_OK()

#include <filesystem>






/* Get run_filename absolute path */
static AlifStatus configRun_filenameAbspath(AlifConfig* config)
{
	std::error_code error;
	std::filesystem::path absPath = std::filesystem::absolute(config->runFilename, error);
	if(error) {
		return ALIFSTATUS_OK();
	}

	return ALIFSTATUS_OK();
}

static AlifStatus configRead_cmdLine(AlifConfig* config)
{
	AlifStatus status;

	//status = configParse_cmdLine(config, &cmdline_warnoptions, &opt_index);
	config->runFilename = config->argv.items[1]; // temp

	status = configRun_filenameAbspath(config);

	status = ALIFSTATUS_OK();

	return status;
}

AlifStatus alifConfig_read(AlifConfig* config)
{
	AlifStatus status;

	status = configRead_cmdLine(config);

	status = ALIFSTATUS_OK();


	return status;
}


AlifStatus alifConfig_setAlifArgv(AlifConfig* config, const AlifArgv* args)
{

	return alifArgv_asWstrList(args, &config->argv);
}


AlifStatus alifConfig_setCharArgv(AlifConfig* config, alif_size_t argc, char* const* argv)
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


AlifStatus alifConfig_setArgv(AlifConfig* config, alif_size_t argc, wchar_t* const* argv)
{
	AlifArgv args = {
		.argc = argc,
		.useCharArgv = 0,
		.charArgv = nullptr,
		.wcharArgv = argv
	};

	return alifConfig_setAlifArgv(config, &args);
}


