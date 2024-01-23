#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifRun.h"

#pragma warning(disable : 4996) // for disable unsafe functions error

#define ALIF_COPYRIGHT L"للمزيد من المعلومات اكتب \"help\", \"copyright\", \"license\" "


/* ----------------------------------- تهيئة اللغة ----------------------------------- */
AlifConfig tempConfig{};
static void alifMain_init(AlifArgv* _args) {

	AlifConfig config{};

	alif_localeInit();

	alif_memoryInit();

	alifArgv_asWStrList(&config, _args);

	alif_initFromConfig(&config);
	tempConfig = config;
}


/* ----------------------------------- تشغيل اللغة ----------------------------------- */
static int alifMain_runFileObj(AlifObj* _pn, AlifObj* _fn) {
	FILE* fp = alif_fOpenObj(_fn, "rb");

	if (fp == nullptr) {
		wprintf(L"%ls: لا يمك فتح الملف %ls: [Errno %d] %ls\n",
			(const wchar_t*)((AlifUStrObject*)_pn)->UTF,
			(const wchar_t*)((AlifUStrObject*)_fn)->UTF,
			errno, _wcserror(errno));
		return 2;
	}

	int run = alifRun_fileObj(fp, _fn, 1);
}

static int alifMain_runFile(AlifConfig* _config) {
	AlifObj* fileName = alifUStr_ObjFromWChar(_config->runFilename);
	AlifObj* programName = alifUStr_ObjFromWChar(_config->programName);

	int res = alifMain_runFileObj(programName, fileName);

	return res;
}

int alif_runMain()
{
	int exitcode = 0;

	AlifConfig config = tempConfig;

	if (config.runCommand) {
		//exitcode = alifMain_runCommand(config->runCommand);
	}
	else if (config.runModule) {
		//exitcode = alifMain_runModule(config->runModule, 1);
	}
	else if (config.runFilename != nullptr) {
		exitcode = alifMain_runFile(&config);
	}
	else {	
		//exitcode = alifMain_runStdin(config);
	}

	return exitcode;
}

/* ----------------------------------- بداية اللغة ----------------------------------- */
static int alifMain_main(AlifArgv* args) {
	// هذه الدالة مسؤلة عن تهيئة اللغة للبدأ بالتنفيذ
	alifMain_init(args);

	// هذه الدالة مسؤلة عن تشغيل البرنامج
	return alif_runMain();
}

int alif_mainWchar(int _argc, wchar_t** _argv) {
	AlifArgv args = { _argc, 0, nullptr, _argv };
	return alifMain_main(&args);
}

int alif_mainBytes(int _argc, char** _argv) {
	AlifArgv args = { _argc, 1, _argv, nullptr };
	return alifMain_main(&args);
}

#ifdef _WINDOWS
int wmain(int _argc, wchar_t** _argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"examle.alif" };
	alif_mainWchar(2, argsv);
	return 0;
}
#else
int main(int _argc, char** _argv)
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return alif_mainBytes(2, argsv);
}
#endif
