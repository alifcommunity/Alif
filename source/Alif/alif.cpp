#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
//#include "AlifCore_Run.h"


 // 26
#define COPYRIGHT \
    "Type \"help\", \"copyright\", \"credits\" or \"license\" " \
    "for more information."

/* ----------------------------------- تهيئة اللغة ----------------------------------- */
static AlifIntT alifMain_init(AlifArgv* _args) {

	AlifIntT status{};
	AlifConfig config{};

	status = alifDureRun_initialize();
	if (status < 1) {
		return status;
	}

	alifConfig_initAlifConfig(&config);

	/*
		- بما أن لغة ألف لغة عربية بالتالي
		يجب ترميزها قبل البدأ بالبرنامج
		- يجب تهيئة ذاكرة ألف العامة قبل البدأ بالبرنامج
		لأنه سيتم أستخدامها خلال كامل البرنامج
	*/
	status = alif_preInitFromConfig(&config);
	if (status < 1) {
		goto done;
	}

	status = alifArgv_asWStringList(&config, _args);
	if (status < 1) {
		goto done;
	}

	status = alif_initFromConfig(&config);
	if (status < 1) {
		goto done;
	}

	status = 1;

done:
	alifConfig_clear(&config);
	return status;
}


/* ----------------------------------- تشغيل اللغة ----------------------------------- */
static AlifIntT alifMain_runFileObj(AlifObject* _pn, AlifObject* _fn, AlifIntT _skipFirstLine) { // 365
	FILE* fp_ = alif_fOpenObj(_fn, "rb");

	if (fp_ == nullptr) {
		//alifErr_clear();
		//alifSys_formatStderr("%S: can't open file %R: [Errno %d] %s\n",
		//	_pn, _fn, errno, strerror(errno));
		printf("%ls: لا يمكن فتح الملف %ls: [Errno %d] %s\n",
			(const char*)((AlifUStrObject*)_pn)->data.any,
			(const char*)((AlifUStrObject*)_fn)->data.any,
			errno, "لا يوجد ملف او مسار بهذا الاسم");
		return 2;
	}

	if (_skipFirstLine) {
		AlifIntT ch{};
		while ((ch = getc(fp_)) != EOF) {
			if (ch == '\n') {
				(void)ungetc(ch, fp_);
				break;
			}
		}
	}

	AlifIntT run = alifRun_fileObj(fp_, _fn, 1);
	return (run != 0);
}

static AlifIntT alifMain_runFile(const AlifConfig* _config) { // 414
	AlifObject* fileName = alifUStr_fromWideChar(_config->runFilename, -1);
	if (fileName == nullptr) {
		//alifErr_print();
		return -1;
	}
	AlifObject* programName = alifUStr_fromWideChar(_config->programName, -1);
	if (programName == nullptr) {
		ALIF_DECREF(fileName);
		//alifErr_print();
		return -1;
	}

	AlifIntT res_ = alifMain_runFileObj(programName, fileName, _config->skipFirstLine);

	ALIF_DECREF(fileName);
	ALIF_DECREF(programName);
	return res_;
}



static void alifMain_header(const AlifConfig* _config) { // 183
	if (_config->quiet) {
		return;
	}

	//if (!_config->verbose and (config_runCode(_config) or !stdin_isInteractive(_config))) {
	//	return;
	//}

	//fprintf(stderr, "Alif %s on %s\n", alif_getVersion(), alif_getPlatform());
	//if (_config->siteImport) {
	//	fprintf(stderr, "%s\n", COPYRIGHT);
	//}
}


static void alifMain_runAlif(AlifIntT* _exitcode) { // 614
	AlifObject* mainImporterPath = nullptr;
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifConfig* config = (AlifConfig*)alifInterpreter_getConfig(interp);

	//if (ALIFSTATUS_EXCEPTION(_alifPathConfig_updateGlobal(config))) {
	//	goto error;
	//}

	//if (config->runFilename != nullptr) {
	//	if (alifMain_getImporter(config->runFilename, &main_importer_path,
	//		_exitcode)) {
	//		return;
	//	}
	//}

	//alifMain_importReadline(config);

	//AlifObject* path0 = nullptr;
	//if (mainImporterPath != nullptr) {
	//	path0 = ALIF_NEWREF(mainImporterPath);
	//}
	//else if (!config->safePath) {
	//	AlifIntT res = alifPathConfig_computeSysPath0(&config->argv, &path0);
	//	if (res < 0) {
	//		goto error;
	//	}
	//	else if (res == 0) {
	//		ALIF_CLEAR(path0);
	//	}
	//}
	//if (path0 != nullptr) {
	//	wchar_t* wstr = alifUStr_asWideCharString(path0, nullptr);
	//	if (wstr == nullptr) {
	//		ALIF_DECREF(path0);
	//		goto error;
	//	}
	//	config->sysPath0 = alifMem_rawWcsdup(wstr);
	//	alifMem_dataFree(wstr);
	//	if (config->sysPath0 == nullptr) {
	//		ALIF_DECREF(path0);
	//		goto error;
	//	}
	//	AlifIntT res = alifMain_sysPathAddPath0(interp, path0);
	//	ALIF_DECREF(path0);
	//	if (res < 0) {
	//		goto error;
	//	}
	//}

	alifMain_header(config);

	alifInterpreter_setRunningMain(interp);

	if (config->runCommand) {
		//*_exitcode = alifMain_runCommand(config->runCommand);
	}
	else if (config->runModule) {
		//*_exitcode = alifMain_runModule(config->runModule, 1);
	}
	else if (mainImporterPath != nullptr) {
		//*_exitcode = alifMain_runModule(L"__main__", 0);
	}
	else if (config->runFilename != nullptr) {
		*_exitcode = alifMain_runFile(config);
	}
	else {
		//*_exitcode = alifMain_runStdin(config);
	}

	//alifMain_repl(config, _exitcode);
	goto done;

error:
	//*_exitcode = alifMain_exitErrPrint();

done:
	alifInterpreter_setNotRunningMain(interp);
	ALIF_XDECREF(mainImporterPath);
}


AlifIntT alif_runMain() { // 770
	AlifIntT exitcode = 0;

	alifMain_runAlif(&exitcode);

	//if (alif_finalizeEx() < 0) {
	//	exitcode = 120;
	//}

	//alifMain_free();

	//if (_alifDureRun_.signals.unhandledKeyboardInterrupt) {
	//	exitcode = exit_sigint();
	//}

	return exitcode;
}

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
	return alif_runMain();
}

AlifIntT alif_mainWchar(AlifIntT _argc, wchar_t** _argv) {
	AlifArgv args_ = {
		.argc = _argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv
	};
	return alifMain_main(&args_);
}

AlifIntT alif_mainBytes(AlifIntT _argc, char** _argv) {
	AlifArgv args_ = {
		.argc = _argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr
	};
	return alifMain_main(&args_);
}

#ifdef _WINDOWS
AlifIntT wmain(AlifIntT _argc, wchar_t** _argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	return alif_mainWchar(2, argsv);
}
#else
AlifIntT main(AlifIntT _argc, char** _argv)
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return alif_mainBytes(2, argsv);
}
#endif
