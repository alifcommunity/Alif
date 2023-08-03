#include "Alif.h"
#include "alifcore_initConfig.h"    // AlifArgv


static AlifStatus alifMain_init(const AlifArgv* args) {

	AlifStatus status{};

	AlifPreConfig preConfig{};
	AlifPreConfig_InitConfig(&preConfig);



	AlifConfig config{};


	if (args->useCharArgv) {
		//status = alifConfig_setCharArgv(&config, args->argc, args->char_argv);
	}
	else {
		status = alifConfig_setArgv(&config, args->argc, args->wcharArgv);
	}

	status = alifInit_fromConfig(&config);

	return status;
}


static void alifMain_run(int* exitcode)
{

}


static int alif_runMain()
{
	int exitcode = 0;

	alifMain_run(&exitcode);

	return exitcode;
}


static int alifMain_main(AlifArgv* args)
{
	// مرحلة تهيئة البرنامج قبل تشغيله
	AlifStatus status = alifMain_init(args);


	// مرحلة تشغيل البرنامج
	return alif_runMain();
}


int alif_mainWchar(int argc, wchar_t** argv)
{
	AlifArgv args = {
		.argc = argc,
		.useCharArgv = 0,
		.charArgv = nullptr,
		.wcharArgv = argv
	};

	return alifMain_main(&args);
}


int alif_mainChar(int argc, char** argv)
{
	AlifArgv args = {
		.argc = argc,
		.useCharArgv = 1,
		.charArgv = argv,
		.wcharArgv = nullptr
	};

	return alifMain_main(&args);
}


#ifdef MS_WINDOWS
int wmain(int _argc, wchar_t** _argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	return alif_mainWchar(2, argsv);
}
#else
int main(int _argc, char** _argv)
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return Alif_MainChar(2, argsv);
}
#endif


