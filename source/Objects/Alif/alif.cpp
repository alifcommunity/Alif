#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifRun.h"

#pragma warning(disable : 4996) // for disable unsafe functions error

#define ALIF_COPYRIGHT L"للمزيد من المعلومات اكتب \"help\", \"copyright\", \"license\" "


/* ----------------------------------- تهيئة اللغة ----------------------------------- */
AlifConfig _tempConfig_{};
static void alifMain_init(AlifArgv* _args) {

	AlifConfig config_{};

	alif_localeInit();

	alif_memoryInit();

	alifArgv_asWStrList(&config_, _args);

	alif_initFromConfig(&config_);
	_tempConfig_ = config_;
}


/* ----------------------------------- تشغيل اللغة ----------------------------------- */
static int alifMain_runFileObj(AlifObj* _pn, AlifObj* _fn) {
	FILE* fp_ = alif_fOpenObj(_fn, "rb");

	if (fp_ == nullptr) {
		wprintf(L"%ls: لا يمك فتح الملف %ls: [Errno %d] %ls\n",
			(const wchar_t*)((AlifUStrObject*)_pn)->UTF,
			(const wchar_t*)((AlifUStrObject*)_fn)->UTF,
			errno, _wcserror(errno));
		return 2;
	}

	int run = alifRun_fileObj(fp_, _fn, 1);
}

static int alifMain_runFile(AlifConfig* _config) {
	AlifObj* fileName = alifUStr_objFromWChar(_config->runFilename);
	AlifObj* programName = alifUStr_objFromWChar(_config->programName);

	int res_ = alifMain_runFileObj(programName, fileName);

	return res_;
}

int alif_runMain()
{
	int exitCode = 0;

	AlifConfig config_ = _tempConfig_;

	if (config_.runCommand) {
		//exitcode = alifMain_runCommand(config_->runCommand);
	}
	else if (config_.runModule) {
		//exitcode = alifMain_runModule(config_->runModule, 1);
	}
	else if (config_.runFilename != nullptr) {
		exitCode = alifMain_runFile(&config_);
	}
	else {	
		//exitcode = alifMain_runStdin(config_);
	}

	return exitCode;
}

/* ----------------------------------- بداية اللغة ----------------------------------- */
static int alifMain_main(AlifArgv* _args) {
	// هذه الدالة مسؤلة عن تهيئة اللغة للبدأ بالتنفيذ
	alifMain_init(_args);

	// هذه الدالة مسؤلة عن تشغيل البرنامج
	return alif_runMain();
}

int alif_mainWchar(int _argc, wchar_t** _argv) {
	AlifArgv args_ = { _argc, 0, nullptr, _argv };
	return alifMain_main(&args_);
}

int alif_mainBytes(int _argc, char** _argv) {
	AlifArgv args_ = { _argc, 1, _argv, nullptr };
	return alifMain_main(&args_);
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
