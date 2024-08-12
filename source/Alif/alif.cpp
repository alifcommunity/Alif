#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_AlifCycle.h"
//#include "AlifCore_AlifState.h"
//#include "AlifCore_AlifRun.h"


#define ALIF_COPYRIGHT "للمزيد من المعلومات اكتب \"help\", \"copyright\", \"license\" "


/* ----------------------------------- تهيئة اللغة ----------------------------------- */

static AlifIntT alifMain_init(AlifArgv* _args) {

	AlifIntT status = 1;
	AlifConfig config{};

	status = alifDureRun_initialize();
	if (status < 1) {
		return status;
	}

	alifConfig_initAlifConfig(&config);

	status = alif_mainMemoryInit();
	if (status < 1) {
		goto done;
	}

	status = alifArgv_asStringList(&config, _args);
	if (status < 1) {
		goto done;
	}

	//status = alif_initFromConfig(&config_);
	if (status < 1) {
		goto done;
	}

	status = 1;

done:
	//alifConfig_clear(&config);
	return status;
}


///* ----------------------------------- تشغيل اللغة ----------------------------------- */
//static AlifIntT alifMain_runFileObj(AlifObject* _pn, AlifObject* _fn, AlifIntT _skipFirstLine) {
//	FILE* fp_ = alif_fOpenObj(_fn, "r");
//
//	if (fp_ == nullptr) {
//		printf("%ls: لا يمكن فتح الملف %ls: [Errno %d] %s\n",
//			(const char*)((AlifUStrObject*)_pn)->UTF,
//			(const char*)((AlifUStrObject*)_fn)->UTF,
//			errno, "لا يوجد ملف او مسار بهذا الاسم");
//		return 2;
//	}
//
//	if (_skipFirstLine) {
//		AlifIntT ch{};
//		while ((ch = getwc(fp_)) != WEOF) {
//			if (ch == L'\n') {
//				(void)ungetwc(ch, fp_);
//				break;
//			}
//		}
//	}
//
//	AlifIntT run = alifRun_fileObj(fp_, _fn, 1);
//
//	return run;
//}
//
//static int alifMain_runFile(AlifConfig* _config) {
//	AlifObject* fileName = alifUStr_objFromWChar(_config->runFilename);
//	if (fileName == nullptr) {
//		// error
//		return -1;
//	}
//	AlifObject* programName = alifUStr_objFromWChar(_config->programName);
//	if (programName == nullptr) {
//		// error
//		return -1;
//	}
//
//	int res_ = alifMain_runFileObj(programName, fileName, _config->skipFirstLine);
//
//	ALIF_DECREF(fileName);
//	ALIF_DECREF(programName);
//	return res_;
//}
//
//int alif_runMain() 
//{
//	int exitCode = 0;
//
//	AlifInterpreter* interpreter = alifInterpreter_get();
//
//	AlifConfig config_ = interpreter->config;
//
//	if (config_.runCommand) {
//		//exitcode = alifMain_runCommand(config_->runCommand);
//	}
//	else if (config_.runModule) {
//		//exitcode = alifMain_runModule(config_->runModule, 1);
//	}
//	else if (config_.runFilename != nullptr) {
//		exitCode = alifMain_runFile(&config_);
//	}
//	else {
//		//exitcode = alifMain_runStdin(config_);
//	}
//
//	return exitCode;
//}

/* ----------------------------------- بداية اللغة ----------------------------------- */
static AlifIntT alifMain_main(AlifArgv* _args) {
	// هذه الدالة مسوؤلة عن تهيئة اللغة للبدأ بالتنفيذ
	AlifIntT status = alifMain_init(_args);
	if (status < 0) {
		//alifMem_freeInit();
		exit(-2);
	}
	else if (status < 1) {
		//alifMem_freeInit();
		return status;
	}

	// هذه الدالة مسؤلة عن تشغيل البرنامج
	//return alif_runMain();
	return 0; // temp
}

AlifIntT alif_mainWchar(int _argc, wchar_t** _argv) {
	AlifArgv args_ = {
		.argc = _argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv
	};
	return alifMain_main(&args_);
}

AlifIntT alif_mainBytes(int _argc, char** _argv) {
	AlifArgv args_ = {
		.argc = _argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr
	};
	return alifMain_main(&args_);
}

#ifdef _WINDOWS
int wmain(int _argc, wchar_t** _argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	return alif_mainWchar(2, argsv);
}
#else
int main(int _argc, char** _argv)
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return alif_mainBytes(2, argsv);
}
#endif
