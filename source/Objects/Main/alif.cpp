#include "alif.h"
#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_AlifCycle.h"


#define ALIF_COPYRIGHT L"للمزيد من المعلومات اكتب \"help\", \"copyright\", \"license\" "


/* ----------------------------------- تهيئة اللغة ----------------------------------- */

static void alifMain_init(AlifArgv* _args) {

	/*
		تهيئة معلومات اثناء عمل اللغة
		تقوم بتهيئة المسار الرئيسي فقط في الوقت الحالي
	*/
	alifRuntime_init();


	alif_consoleInit();

	AlifConfig config{};
	alifConfig_initAlifConfig(&config);

	alifArgv_asWStrList(&config, _args);

	alif_initFromConfig(&config);
}



static int alifMain_main(AlifArgv* args) {
	alifMain_init(args);

	return 0;
	//return alif_runMain();
}


int alif_mainWchar(int _argc, wchar_t** _argv) {
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv
	};
	return alifMain_main(&args);
}



int alif_mainBytes(int _argc, char** _argv) {
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr
	};
	return alifMain_main(&args);
}

#ifdef _WINDOWS
int wmain(int _argc, wchar_t** _argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	alif_memoryInit();
	alif_mainWchar(2, argsv);
	wprintf(L"%ls",ALIF_COPYRIGHT);
	return 0;
	//wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	//return alif_mainWchar(2, argsv);
}
#else
int main(int _argc, char** _argv)
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return alif_mainBytes(2, argsv);
}
#endif
