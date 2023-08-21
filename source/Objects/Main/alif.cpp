

#include "Alif.h"
//#include "alifCore_call.h"
#include "alifcore_initConfig.h"
#include "alifCore_interp.h"
//#include "alifCore_pathConfig.h"
#include "alifCore_alifCycle.h"
#include "alifCore_alifState.h"









#ifdef MS_WINDOWS
#  include <windows.h>
#endif










/* ___________ alifMain_init() ___________ */

static AlifStatus alifMain_init(const AlifArgv* _args) {

	AlifStatus status{};

	//status = alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifPreConfig preConfig{};
	//alifPreConfig_initAlifConfig(&preConfig);

	//status = alif_preInitializeFromAlifArgv(&preConfig, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifConfig config{};
	//alifConfig_initAlifConfig(&config);

	if (_args->useBytesArgv) {
		//status = alifConfig_setCharArgv(&config, _args->argc, _args->bytesArgv);
	}
	else {
		//status = alifConfig_setArgv(&config, _args->argc, _args->wcharArgv);
	}
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	//status = alifInit_fromConfig(&config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}
	status = ALIFSTATUS_OK();

done:
	//alifConfig_clear(&config);
	return status;
}





/* ___________ alifmain_runAlif() ___________ */





















































































































































































































































































































































































































































































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
	AlifStatus status = alifMain_init(_args);
	//if (ALIFSTATUS_IS_EXIT(status)) {
	//	pyMain_free();
	//	return status.exitCode;
	//}
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	alifMain_exitError(status);
	//}

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
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	return alif_mainWchar(2, argsv);
}
#else
int main(int _argc, char** _argv)
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return Alif_MainBytes(2, argsv);
}
#endif


