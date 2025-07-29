#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_PathConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Run.h"


// 26
#define COPYRIGHT \
    "Type \"help\", \"copyright\", \"credits\" or \"license\" " \
    "for more information."

/* ----------------------------------- تهيئة اللغة ----------------------------------- */
static AlifStatus alifMain_init(AlifArgv* _args) {

	AlifStatus status{};

	status = _alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	// يجب تهيئة ذاكرة ألف العامة قبل البدأ بالبرنامج
	//	لأنه سيتم أستخدامها خلال كامل البرنامج
	if (alif_mainMemoryInit() < 1) {
		return ALIFSTATUS_NO_MEMORY();
	}

	AlifPreConfig preconfig{};
	alifPreConfig_initAlifConfig(&preconfig);

	status = _alif_preInitializeFromAlifArgv(&preconfig, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifConfig config{};
	alifConfig_initAlifConfig(&config);

	if (_args->useBytesArgv) {
		status = alifConfig_setBytesArgv(&config, _args->argc, _args->bytesArgv);
	}
	else {
		status = alifConfig_setArgv(&config, _args->argc, _args->wcharArgv);
	}
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = alif_initFromConfig(&config); //* here
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = ALIFSTATUS_OK();

done:
	alifConfig_clear(&config);
	return status;
}


/* ----------------------------------- تشغيل اللغة ----------------------------------- */

static AlifIntT stdin_isInteractive(const AlifConfig* config) { // 90
	return (isatty(fileno(stdin)) or config->interactive);
}

static AlifIntT alifMain_runFileObj(AlifObject* _pn, AlifObject* _fn, AlifIntT _skipFirstLine) { // 365
	FILE* fp_ = alif_fOpenObj(_fn, "rb");

	if (fp_ == nullptr) {
		alifErr_clear();
		//alifSys_formatStderr("%S: can't open file %R: [Errno %d] %s\n",
		//	_pn, _fn, errno, strerror(errno));
		printf("%s: لا يمكن فتح الملف %s: [Errno %d] %s\n",
			(const char*)ALIFUSTR_DATA(_pn), (const char*)ALIFUSTR_DATA(_fn), errno, "لا يوجد ملف او مسار بهذا الاسم");
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

	AlifCompilerFlags cf = ALIFCOMPILERFLAGS_INIT;
	AlifIntT run = alifRun_fileObject(fp_, _fn, 1, &cf);
	return (run != 0);
}

static AlifIntT alifMain_runCommand(wchar_t* _command) { // 231
	AlifObject* unicode{}, * bytes{};
	AlifIntT ret{};
	AlifCompilerFlags cf{};

	unicode = alifUStr_fromWideChar(_command, -1);
	if (unicode == nullptr) {
		goto error;
	}

	if (alifSys_audit("alif.run_command", "O", unicode) < 0) {
		return alifMainExit_errPrint();
	}

	bytes = _alifUStr_asUTF8String(unicode, nullptr);
	ALIF_DECREF(unicode);
	if (bytes == nullptr) {
		goto error;
	}

	cf = ALIFCOMPILERFLAGS_INIT;
	cf.flags |= ALIFCF_IGNORE_COOKIE;
	ret = _alifRun_simpleStringFlagsWithName(alifBytes_asString(bytes), "<string>", &cf);
	ALIF_DECREF(bytes);
	return (ret != 0);

error:
	alifSys_writeStderr("غير قادر على فك تشفير الأمر من سطر الأوامر:\n");
	return alifMainExit_errPrint();
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




static AlifIntT alifMain_sysPathAddPath0(AlifInterpreter* interp, AlifObject* path0) { // 156
	AlifObject* sysPath{};
	AlifObject* sysdict = interp->sysDict;
	if (sysdict != nullptr) {
		sysPath = alifDict_getItemWithError(sysdict, &ALIF_ID(Path));
		if (sysPath == nullptr and alifErr_occurred()) {
			return -1;
		}
	}
	else {
		sysPath = nullptr;
	}
	if (sysPath == nullptr) {
		//alifErr_setString(_alifExc_runtimeError_, "unable to get sys.path");
		return -1;
	}

	if (alifList_insert(sysPath, 0, path0)) {
		return -1;
	}
	return 0;
}

static void alifMain_header(const AlifConfig* _config) { // 183
	//if (_config->quiet) {
	//	return;
	//}

	//if (!_config->verbose and (config_runCode(_config) or !stdin_isInteractive(_config))) {
	//	return;
	//}

	//fprintf(stderr, "Alif %s on %s\n", alif_getVersion(), alif_getPlatform());
	//if (_config->siteImport) {
	//	fprintf(stderr, "%s\n", COPYRIGHT);
	//}
}


static AlifIntT alifMain_runStartup(AlifConfig* _config, AlifIntT* _exitcode) { // 436
	AlifIntT ret{};
	AlifCompilerFlags cf{}; //* alif
	FILE* fp{}; //* alif

	if (!_config->useEnvironment) {
		return 0;
	}
	AlifObject* startup = nullptr;
	//#ifdef _WINDOWS
	//	const wchar_t* env = _wgetenv(L"ALIFSTARTUP");
	//	if (env == nullptr or env[0] == L'\0') {
	//		return 0;
	//	}
	//	startup = alifUStr_fromWideChar(env, wcslen(env));
	//	if (startup == nullptr) {
	//		goto error;
	//	}
	//#else
	//	const char* env = _alif_getEnv(_config->useEnvironment, "ALIFSTARTUP");
	//	if (env == nullptr) {
	//		return 0;
	//	}
	//	startup = alifUStr_decodeFSDefault(env);
	//	if (startup == nullptr) {
	//		goto error;
	//	}
	//#endif
		//if (alifSys_audit("alif.run_startup", "O", startup) < 0) {
		//	goto error;
		//}

	fp = alif_fOpenObj(startup, "r");
	if (fp == nullptr) {
		AlifIntT save_errno = errno;
		//alifErr_clear();
		//alifSys_writeStderr("Could not open ALIFSTARTUP\n");

		errno = save_errno;
		//alifErr_setFromErrnoWithFilenameObjects(_alifExcOSError_, startup, nullptr);
		goto error;
	}

	cf = ALIFCOMPILERFLAGS_INIT;
	(void)alifRun_simpleFileObject(fp, startup, 0, &cf);
	//alifErr_clear();
	fclose(fp);
	ret = 0;

done:
	ALIF_XDECREF(startup);
	return ret;

error:
	//ret = alifMain_errPrint(_exitcode);
	goto done;
}


static AlifIntT alifMain_runStdin(AlifConfig* _config) { // 542
	if (stdin_isInteractive(_config)) {
		// do exit on SystemExit
		//alifMain_setInspect(_config, 0);

		AlifIntT exitcode{};
		if (alifMain_runStartup(_config, &exitcode)) {
			return exitcode;
		}

		//if (alifMain_runInteractiveHook(&exitcode)) {
		//	return exitcode;
		//}
	}

	/* call pending calls like signal handlers (SIGINT) */
	//if (alif_makePendingCalls() == -1) {
	//	return alifMain_exitErrPrint();
	//}

	////if (alifSys_audit("alif.run_stdin", nullptr) < 0) {
	////	return alifMain_exitErrPrint();
	////}

	//if (!isatty(fileno(stdin))
	//	or _alif_getEnv(_config->useEnvironment, "ALIF_BASIC_REPL")) {
	//	AlifCompilerFlags cf = ALIFCOMPILERFLAGS_INIT;
	//	AlifIntT run = alifRun_anyFileExFlags(stdin, "<stdin>", 0, &cf);
	//	return (run != 0);
	//}
	//AlifIntT run = alifMain_runModule(L"_alifRepl", 0);
	//return (run != 0);

	return 2; //* alif //* delete
}


static void alifMain_runAlif(AlifIntT* _exitcode) { // 614
	AlifObject* mainImporterPath = nullptr;
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifConfig* config = (AlifConfig*)alifInterpreter_getConfig(interp);

	AlifObject* path0{}; //* alif

	if (ALIFSTATUS_EXCEPTION(_alifPathConfig_updateGlobal(config))) {
		goto error;
	}

	//if (config->runFilename != nullptr) {
	//	if (alifMain_getImporter(config->runFilename, &mainImporterPath,
	//		_exitcode)) {
	//		return;
	//	}
	//}

	//alifMain_importReadline(config);

	path0 = nullptr;
	if (mainImporterPath != nullptr) {
		path0 = ALIF_NEWREF(mainImporterPath);
	}
	else if (!config->safePath) {
		AlifIntT res = _alifPathConfig_computeSysPath0(&config->argv, &path0);
		if (res < 0) {
			goto error;
		}
		else if (res == 0) {
			ALIF_CLEAR(path0);
		}
	}
	if (path0 != nullptr) {
		wchar_t* wstr = alifUStr_asWideCharString(path0, nullptr);
		if (wstr == nullptr) {
			ALIF_DECREF(path0);
			goto error;
		}
		config->sysPath0 = alifMem_wcsDup(wstr);
		alifMem_dataFree(wstr);
		if (config->sysPath0 == nullptr) {
			ALIF_DECREF(path0);
			goto error;
		}
		AlifIntT res = alifMain_sysPathAddPath0(interp, path0);
		ALIF_DECREF(path0);
		if (res < 0) {
			goto error;
		}
	}

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
		*_exitcode = alifMain_runStdin(config);
	}

	//alifMain_repl(config, _exitcode);
	goto done;

error:
	//*_exitcode = alifMain_exitErrPrint();

done:
	_alifInterpreter_setNotRunningMain(interp);
	ALIF_XDECREF(mainImporterPath);
}


AlifIntT alif_runMain() { // 770
	AlifIntT exitcode = 0;

	alifMain_runAlif(&exitcode);

	//if (alif_finalizeEx() < 0) {
	//	exitcode = 120;
	//}

	//alifMain_free();

	//if (_alifRuntime_.signals.unhandledKeyboardInterrupt) {
	//	exitcode = exit_sigint();
	//}

	return exitcode;
}

/* ----------------------------------- بداية اللغة ----------------------------------- */

static void alifMain_free(void) {
	//_alifImport_fini2();
	//_alifPathConfig_clearGlobal();
	_alifWStringList_clear(&_alifRuntime_.origArgv);
	_alifRuntime_finalize();
}

static void ALIF_NO_RETURN alifMain_exitError(AlifStatus _status) {
	if (ALIFSTATUS_IS_EXIT(_status)) {
		alifMain_free();
	}
	alif_exitStatusException(_status);
}

static AlifIntT alifMain_main(AlifArgv* _args) {
	// هذه الدالة مسوؤلة عن تهيئة اللغة للبدأ بالتنفيذ
	AlifStatus status = alifMain_init(_args);
	if (ALIFSTATUS_IS_EXIT(status)) {
		alifMain_free();
		return status.exitcode;
	}
	else if (ALIFSTATUS_EXCEPTION(status)) {
		alifMain_exitError(status);
	}

	// هذه الدالة مسؤلة عن تشغيل البرنامج
	return alif_runMain();
}

static int alif_mainWchar(int _argc, wchar_t** _argv) {
	AlifArgv args_ = {
		.argc = (AlifIntT)_argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv
	};
	return (int)alifMain_main(&args_);
}

static int alif_mainBytes(int _argc, char** _argv) {
	AlifArgv args_ = {
		.argc = (AlifIntT)_argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr
	};
	return (int)alifMain_main(&args_);
}

#ifdef _WINDOWS
int wmain(int _argc, wchar_t** _argv) {
	return alif_mainWchar(_argc, _argv);
}
#else
int main(int _argc, char** _argv) {
	return alif_mainBytes(_argc, _argv);
}
#endif
