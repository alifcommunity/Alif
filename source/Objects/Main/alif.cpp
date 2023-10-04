

#include "alif.h"
//#include "alifCore_call.h"
#include "alifCore_initConfig.h"
#include "alifCore_interp.h"
//#include "alifCore_pathConfig.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifState.h"









#ifdef MS_WINDOWS
#  include <windows.h>
#endif










/* ----- alifMain_init() ------------------------------------------- */

static void alifMain_init(const AlifArgv* _args) {



	alifRuntime_initialize();




	AlifPreConfig preConfig{};
	alifPreConfig_initAlifConfig(&preConfig);

	alif_preInitializeFromAlifArgv(&preConfig, _args);




	AlifConfig config{};
	alifConfig_initAlifConfig(&config);

	if (_args->useBytesArgv) {
		alifConfig_setBytesArgv(&config, _args->argc, _args->bytesArgv);
	}
	else {
		alifConfig_setArgv(&config, _args->argc, _args->wcharArgv);
	}




	alif_initializeFromConfig(&config);








}





/* ___________ alifMain_runAlif() ___________ */





















































































































































































































































































































































































































































































static void alifMain_runAlif(int* _exitcode)
{

}


































































































































static int alif_runMain()
{
	int exitcode = 0;

	alifMain_runAlif(&exitcode);

	return exitcode;
}















static int alifMain_main(AlifArgv* _args)
{
	// مرحلة تهيئة البرنامج قبل تشغيله
	alifMain_init(_args);

	// مرحلة تشغيل البرنامج
	return alif_runMain();
}

int alif_mainWchar(int _argc, wchar_t** _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv
	};

	return alifMain_main(&args);
}

int alif_mainBytes(int _argc, char** _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr
	};

	return alifMain_main(&args);
}


#ifdef MS_WINDOWS
int wmain(int _argc, wchar_t** _argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif"};
	return alif_mainWchar(2, argsv);
}
#else
int main(int _argc, char** _argv)	
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return alif_mainBytes(2, argsv);
}
#endif


