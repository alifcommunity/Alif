#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_AlifRun.h"

#pragma warning(disable : 4996) // for disable unsafe functions error

#define ALIF_COPYRIGHT L"للمزيد من المعلومات اكتب \"help\", \"copyright\", \"license\" "


/* ----------------------------------- تهيئة اللغة ----------------------------------- */

static AlifIntT alifMain_init(AlifArgv* _args) {

	AlifIntT status = 1;

	status = alifDureRun_initialize();
	if (status < 1) {
		// error
	}

	status = alif_setLocaleAndWChar();
	if (status < 1) {
		// error
	}

	if (alif_mainMemoryInit() == nullptr) {
		// error
	}

	AlifConfig config_{};
	alifConfig_initAlifConfig(&config_);

	status = alifArgv_asWStrList(&config_, _args);
	if (status < 1) {
		goto done;
	}

	status = alif_initFromConfig(&config_);
	if (status < 1) {
		goto done;
	}

	status = 1;

done:
	return status;
}


/* ----------------------------------- تشغيل اللغة ----------------------------------- */
static int alifMain_runFileObj(AlifObject* _pn, AlifObject* _fn) {
	FILE* fp_ = alif_fOpenObj(_fn, "r"); // لا تستخدم "rb"

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
	AlifObject* fileName = alifUStr_objFromWChar(_config->runFilename);
	AlifObject* programName = alifUStr_objFromWChar(_config->programName);

	int res_ = alifMain_runFileObj(programName, fileName);

	return res_;
}

int alif_runMain()
{
	int exitCode = 0;

	AlifInterpreter* interpreter = alifInterpreter_get();

	AlifConfig config_ = interpreter->config;

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
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
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

