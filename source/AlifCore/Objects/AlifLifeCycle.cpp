#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Exceptions.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_FloatObject.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_Import.h"
#include "AlifCore_PathConfig.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Memory.h"
#include "AlifCore_State.h"
#include "AlifCore_DureRun.h"
#include "AlifCore_DureRunInit.h"



#ifdef HAVE_SIGNAL_H
#  include <signal.h>             // SIG_IGN
#endif





static AlifIntT init_sysStreams(AlifThread* _thread); // 73




AlifDureRun _alifDureRun_ = ALIF_DURERUNSTATE_INIT(_alifDureRun_); // 103

static AlifIntT dureRunInitialized = 0; // 110

AlifIntT alifDureRun_initialize() { // 112

	if (dureRunInitialized) return 1;

	dureRunInitialized = 1;

	return alifDureRunState_init(&_alifDureRun_);
}


char* alif_setLocale(AlifIntT category) {  // 332

	char* res;
#ifdef __ANDROID__
	const char* locale;
	const char** pvar;
#ifdef ALIF_COERCE_C_LOCALE
	const char* coerce_c_locale;
#endif
	const char* utf8_locale = "C.UTF-8";
	const char* env_var_set[] = {
		"LC_ALL",
		"LC_CTYPE",
		"LANG",
		nullptr,
	};

	for (pvar = env_var_set; *pvar; pvar++) {
		locale = getenv(*pvar);
		if (locale != nullptr and *locale != '\0') {
			if (strcmp(locale, utf8_locale) == 0 or
				strcmp(locale, "en_US.UTF-8") == 0) {
				return setlocale(category, utf8_locale);
			}
			return setlocale(category, "C");
		}
	}

#ifdef ALIF_COERCE_C_LOCALE
	coerce_c_locale = getenv("ALIFCOERCECLOCALE");
	if (coerce_c_locale == nullptr or strcmp(coerce_c_locale, "0") != 0) {
		if (setenv("LC_CTYPE", utf8_locale, 1)) {
			fprintf(stderr, "Warning: failed setting the LC_CTYPE "
				"environment variable to %s\n", utf8_locale);
		}
	}
#endif
	res = setlocale(category, utf8_locale);
#else
	res = setlocale(category, "");
#endif

	return res;
}


static AlifIntT interpreter_updateConfig(AlifThread* tstate,
	AlifIntT _onlyUpdatePathConfig) { // 391
	const AlifConfig* config = &tstate->interpreter->config;

	if (!_onlyUpdatePathConfig) {
		AlifIntT status = alifConfig_write(config, tstate->interpreter->dureRun);
		if (status < 1) {
			//_alifErr_setFromAlifStatus(status);
			return -1;
		}
	}

	if (alif_isMainInterpreter(tstate->interpreter)) {
		AlifIntT status = _alifPathConfig_updateGlobal(config);
		if (status < 1) {
			//_alifErr_setFromalifStatus(status);
			return -1;
		}
	}

	tstate->interpreter->longState.maxStrDigits = config->intMaxStrDigits;

	// Update the sys module for the new configuration
	if (_alifSys_updateConfig(tstate) < 0) {
		return -1;
	}
	return 0;
}


static AlifIntT alifCore_initDureRun(AlifDureRun* _dureRun, const AlifConfig* _config) { // 507

	if (_dureRun->initialized) {
		return -1;
	}

	AlifIntT status = alifConfig_write(_config, _dureRun);
	if (status < 1) return status;

	status = alifImport_init();
	if (status < 1) return status;

	status = alifInterpreter_enable(_dureRun);
	if (status < 1) return status;

	return 1;
}

static AlifIntT initInterpreter_settings(AlifInterpreter* _interp,
	const AlifInterpreterConfig* _config) { // 551

	if (_config->useMainAlifMem) {
		_interp->featureFlags |= ALIF_RTFLAGS_USE_ALIFMEM;
	}
	else if (!_config->checkMultiInterpExtensions) {
		//return ALIFSTATUS_ERR("per-interpreter alifmem does not support "
		//	"single-phase init extension modules");
		return -1;
	}
	if (!alif_isMainInterpreter(_interp) &&
		!_config->checkMultiInterpExtensions)
	{
		//return ALIFSTATUS_ERR("The free-threaded build does not support "
		//	"single-phase init extension modules in "
		//	"subinterpreters");
		return -1;
	}

	if (_config->allowFork) {
		_interp->featureFlags |= ALIF_RTFLAGS_FORK;
	}
	if (_config->allowExec) {
		_interp->featureFlags |= ALIF_RTFLAGS_EXEC;
	}
	// Note that fork+exec is always allowed.

	if (_config->allowThreads) {
		_interp->featureFlags |= ALIF_RTFLAGS_THREADS;
	}
	if (_config->allowDaemonThreads) {
		_interp->featureFlags |= ALIF_RTFLAGS_DAEMON_THREADS;
	}

	if (_config->checkMultiInterpExtensions) {
		_interp->featureFlags |= ALIF_RTFLAGS_MULTI_INTERP_EXTENSIONS;
	}

	switch (_config->gil) {
	case ALIF_INTERPRETERCONFIG_DEFAULT_GIL: break;
	case ALIF_INTERPRETERCONFIG_SHARED_GIL: break;
	case ALIF_INTERPRETERCONFIG_OWN_GIL: break;
	default:
		//return ALIFSTATUS_ERR("invalid interpreter config 'gil' value");
		return -1;
	}

	return 1;
}

static void initInterpreter_createGIL(AlifThread* tstate, int gil) { // 607
	alifEval_finiGIL(tstate->interpreter);

	AlifIntT ownGIL = (gil == ALIF_INTERPRETERCONFIG_OWN_GIL);

	alifEval_initGIL(tstate, ownGIL);
}

static AlifIntT alifCore_createInterpreter(AlifDureRun* _dureRun,
	const AlifConfig* _config, AlifThread** _threadP) { // 637

	AlifIntT status{};
	AlifInterpreter* interpreter{};

	status = alifInterpreter_new(nullptr, &interpreter);
	if (status < 1) return status;

	interpreter->ready = 1;

	status = alifConfig_copy(&interpreter->config, _config);
	if (status < 1) return status;

	status = alifGILState_init(interpreter);
	if (status < 1) {
		return status;
	}

	AlifInterpreterConfig config = ALIF_INTERPRETERCONFIG_LEGACY_INIT;
	config.gil = ALIF_INTERPRETERCONFIG_OWN_GIL;
	config.checkMultiInterpExtensions = 0;
	status = initInterpreter_settings(interpreter, &config);
	if (status < 1) {
		return status;
	}

	if (alifInterpreterMem_init(interpreter) < 1) {
		// memory error
		return -1;
	}

	AlifThread* thread = alifThreadState_new(interpreter);

	if (thread == nullptr) {
		// cant make thread
		printf("%s \n", "لا يمكن إنشاء ممر");
		return -1;
	}

	_dureRun->mainThread = thread;
	alifThread_bind(thread);

	initInterpreter_createGIL(thread, config.gil);

	*_threadP = thread;
	return 1;
}


static AlifIntT alifCore_initGlobalObjects(AlifInterpreter* _interp) { // 696
	AlifIntT status{};

	//alifFloat_initState(_interp);

	status = alifUStr_initGlobalObjects(_interp);
	if (status < 1) {
		return status;
	}

	//alifUStr_initState(_interp);

	if (alif_isMainInterpreter(_interp)) {
		_alif_getConstantInit();
	}

	return 1;
}


static AlifIntT alifCore_initTypes(AlifInterpreter* _interp) { // 718
	AlifIntT status{};

	status = alifTypes_initTypes(_interp);
	if (status < 0) {
		return status;
	}

	//status = alifLong_initTypes(_interp);
	//if (status < 0) {
	//	return status;
	//}

	//status = alifUstr_InitTypes(_interp);
	//if (status < 0) {
	//	return status;
	//}

	//status = alifFloat_initTypes(_interp);
	//if (status < 0) {
	//	return status;
	//}

	if (_alifExc_initTypes(_interp) < 0) {
		//return ALIFSTATUS_ERR("failed to initialize an exception type");
		return -1;
	}

	//status = alifExc_initGlobalObjects(_interp);
	//if (status < 0) {
	//	return status;
	//}

	//status = alifExc_initState(_interp);
	//if (status < 0) {
	//	return status;
	//}

	//status = alifErr_initTypes(_interp);
	//if (status < 0) {
	//	return status;
	//}

	//status = alifContext_init(_interp);
	//if (status < 0) {
	//	return status;
	//}

	//status = alifXI_initTypes(_interp);
	//if (status < 0) {
	//	return status;
	//}

	return 1;
}


static AlifIntT alifCore_builtinsInit(AlifThread* _thread) { // 775

	AlifObject* modules{};
	AlifObject* builtinsDict{};

	AlifInterpreter* interp = _thread->interpreter;

	AlifObject* biMod = alifBuiltin_init(interp);
	if (biMod == nullptr) goto error;

	modules = _alifImport_getModules(interp);
	if (_alifImport_fixupBuiltin(_thread, biMod, "builtins", modules) < 0) {
		goto error;
	}

	builtinsDict = alifModule_getDict(biMod);
	if (builtinsDict == nullptr) goto error;
	interp->builtins = ALIF_NEWREF(builtinsDict);


	interp->builtinsCopy = alifDict_copy(interp->builtins);
	if (interp->builtinsCopy == nullptr) {
		goto error;
	}
	ALIF_DECREF(biMod);

	if (_alifImport_initDefaultImportFunc(interp) < 0) {
		goto error;
	}

	return 1;

error:
	ALIF_XDECREF(biMod);
	return -1;
}

static AlifIntT alifCore_interpreterInit(AlifThread* _thread) { // 843

	AlifInterpreter* interpreter = _thread->interpreter;
	AlifIntT status = 1;
	const AlifConfig* config{};
	AlifObject* sysMod = nullptr;

	status = alifCore_initGlobalObjects(interpreter);
	if (status < 0) return status;

	status = alifCode_init(interpreter);
	if (status < 0) return status;

	status = _alifDtoa_init(interpreter);
	if (status < 0) return status;

	//status = alifGC_init(interpreter);
	if (status < 0) return status;

	status = alifCore_initTypes(interpreter);
	if (status < 0) goto done;

	//status = alifAtExit_init(interpreter);
	if (status < 0) return status;

	status = alifSys_create(_thread, &sysMod);
	if (status < 0) goto done;

	status = alifCore_builtinsInit(_thread);
	if (status < 0) goto done;

	config = &interpreter->config;

	status = _alifImport_initCore(_thread, sysMod, config->installImportLib);
	if (status < 0) goto done;

done:
	//ALIF_XDECREF(sysmod);
	return status;

}

static AlifIntT alifInit_config(AlifDureRun* _dureRun,
	AlifThread** _threadP, const AlifConfig* _config) { // 917

	AlifIntT status{};

	status = alifCore_initDureRun(_dureRun, _config);
	if (status < 1) return status;

	AlifThread* thread_{};
	status = alifCore_createInterpreter(_dureRun, _config, &thread_);
	if (status < 1) return status;
	*_threadP = thread_;

	status = alifCore_interpreterInit(thread_);
	if (status < 1) return status;

	_dureRun->coreInitialized = 1;
	return 1;
}

static AlifIntT alifInit_core(AlifDureRun* _dureRun,
	const AlifConfig* _config, AlifThread** _threadPtr) { // 1069

	AlifIntT status{};

	AlifConfig config{};
	alifConfig_initAlifConfig(&config);

	status = alifConfig_copy(&config, _config);
	if (status < 1) goto done;

	status = alifConfig_read(&config);
	if (status < 1) goto done;

	if (!_dureRun->coreInitialized) {
		status = alifInit_config(_dureRun, _threadPtr, &config);
	}
	else {
		//status = alifInit_coreReconfig(_dureRun, _threadPtr, &config);
	}
	if (status < 1) goto done;

done:
	alifConfig_clear(&config);
	return status;
}

static AlifIntT initInterpreter_main(AlifThread* _thread) { // 1156

	AlifIntT status{};
	AlifIntT isMainInterpreter = alif_isMainInterpreter(_thread->interpreter);
	AlifInterpreter* interpreter = _thread->interpreter;
	const AlifConfig* config = alifInterpreter_getConfig(interpreter);

	if (!config->installImportLib) {
		if (isMainInterpreter) {
			interpreter->dureRun->initialized = 1;
		}
		return 1;
	}

	status = _alifConfig_initImportConfig(&interpreter->config);
	if (status < 1) return status;

	if (interpreter_updateConfig(_thread, 1) < 0) {
		return -1;
	}

	if (interpreter_updateConfig(_thread, 1) < 0) {
		//return _ALIFSTATUS_ERR("failed to update the Alif config");
		return -1; //* alif
	}

	//status = alifImport_initExternal(_thread);
	//if (status < 1) return status;

	//status = alifUnicode_initEncoding(_thread);
	//if (status < 1) return status;

	if (isMainInterpreter) {
		if (_alifSignal_init(config->installSignalHandlers) < 0) {
			//return _ALIFSTATUS_ERR("can't initialize signals");
		}

		//if (config_->tracemalloc) {
		//	if (alifTraceMalloc_start(config_->tracemalloc) < 0) {
		//		return -1;
		//	}
		//}
	}

	status = init_sysStreams(_thread);
	if (status < 1) return status;

	//status = init_setBuiltinsOpen();
	//if (status < 1) return status;

	//status = add_mainmodule(interpreter);
	//if (status < 1) return status;

	// need work

	return 1;

}

static AlifIntT alifInit_main(AlifThread* _thread) { // 1363

	AlifInterpreter* interpreter = _thread->interpreter;
	if (!interpreter->dureRun->coreInitialized) {
		// error
		return -1;
	}

	if (interpreter->dureRun->initialized) {
		//return alifInit_mainReconfigure(_thread);
	}

	AlifIntT status = initInterpreter_main(_thread);
	if (status < 1) return status;

	return 1;
}

AlifIntT alif_initFromConfig(const AlifConfig* _config) { // 1383

	if (_config == nullptr) {
		// error
		return -1;
	}

	AlifIntT status{};

	status = alifDureRun_initialize();
	if (status < 1) return status;

	AlifDureRun* dureRun = &_alifDureRun_;
	AlifThread* thread_ = nullptr;

	status = alifInit_core(dureRun, _config, &thread_);
	if (status < 1) return status;

	_config = alifInterpreter_getConfig(thread_->interpreter);

	if (_config->initMain) {
		status = alifInit_main(thread_);
		if (status < 1) return status;
	}

	return 1;
}
















static AlifObject* create_stdio(const AlifConfig* config, AlifObject* io,
	AlifIntT fd, AlifIntT write_mode, const char* name,
	const wchar_t* encoding, const wchar_t* errors) { // 2568
	AlifObject* buf = nullptr, * stream = nullptr, * text = nullptr, * raw = nullptr, * res;
	const char* mode{};
	const char* newline{};
	AlifObject* line_buffering{}, * write_through{};
	AlifIntT buffering{}, isatty{};
	const AlifIntT buffered_stdio = config->bufferedStdio;

	if (!_alif_isValidFD(fd)) {
		return ALIF_NONE;
	}

	if (!buffered_stdio && write_mode)
		buffering = 0;
	else
		buffering = -1;
	if (write_mode)
		mode = "wb";
	else
		mode = "rb";
	buf = _alifObject_callMethod(io, &ALIF_STR(Open), "isiOOOO",
		fd, mode, buffering,
		ALIF_NONE, ALIF_NONE, /* encoding, errors */
		ALIF_NONE, ALIF_FALSE); /* newline, closefd */
	if (buf == nullptr)
		goto error;

	if (buffering) {
		raw = alifObject_getAttr(buf, &ALIF_STR(Raw));
		if (raw == nullptr)
			goto error;
	}
	else {
		raw = ALIF_NEWREF(buf);
	}

#ifdef HAVE_WINDOWS_CONSOLE_IO
	/* Windows console IO is always UTF-8 encoded */
	AlifTypeObject* winconsoleio_type;
	winconsoleio_type = (AlifTypeObject*)_alifImport_getModuleAttr(
		&ALIF_STR(_io), &ALIF_ID(_WindowsConsoleIO));
	if (winconsoleio_type == nullptr) {
		goto error;
	}
	AlifIntT is_subclass;
	is_subclass = alifObject_typeCheck(raw, winconsoleio_type);
	ALIF_DECREF(winconsoleio_type);
	if (is_subclass) {
		encoding = L"utf-8";
	}
#endif

	text = alifUStr_fromString(name);
	if (text == nullptr or alifObject_setAttr(raw, &ALIF_STR(Name), text) < 0)
		goto error;
	res = alifObject_callMethodNoArgs(raw, &ALIF_STR(IsAtty));
	if (res == nullptr)
		goto error;
	isatty = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (isatty == -1)
		goto error;
	if (!buffered_stdio)
		write_through = ALIF_TRUE;
	else
		write_through = ALIF_FALSE;
	if (buffered_stdio and (isatty or fd == fileno(stderr)))
		line_buffering = ALIF_TRUE;
	else
		line_buffering = ALIF_FALSE;

	ALIF_CLEAR(raw);
	ALIF_CLEAR(text);

#ifdef _WINDOWS
	newline = nullptr;
#else
	newline = "\n";
#endif

	AlifObject* encoding_str;
	encoding_str = alifUStr_fromWideChar(encoding, -1);
	if (encoding_str == nullptr) {
		ALIF_CLEAR(buf);
		goto error;
	}

	AlifObject* errors_str;
	errors_str = alifUStr_fromWideChar(errors, -1);
	if (errors_str == nullptr) {
		ALIF_CLEAR(buf);
		ALIF_CLEAR(encoding_str);
		goto error;
	}

	stream = _alifObject_callMethod(io, &ALIF_ID(TextIOWrapper), "OOOsOO",buf,
		encoding_str, errors_str, newline, line_buffering, write_through);
	ALIF_CLEAR(buf);
	ALIF_CLEAR(encoding_str);
	ALIF_CLEAR(errors_str);
	if (stream == nullptr)
		goto error;

	if (write_mode)
		mode = "w";
	else
		mode = "r";
	text = alifUStr_fromString(mode);
	if (!text or alifObject_setAttr(stream, &ALIF_STR(Mode), text) < 0)
		goto error;
	ALIF_CLEAR(text);
	return stream;

error:
	ALIF_XDECREF(buf);
	ALIF_XDECREF(stream);
	ALIF_XDECREF(text);
	ALIF_XDECREF(raw);

	//if (alifErr_exceptionMatches(_alifExcOSError_) and !_alif_isValidFD(fd)) {
	//	alifErr_clear();
	//	return ALIF_NONE;
	//}
	return nullptr;
}








static AlifIntT init_sysStreams(AlifThread* _thread) { // 2742
	AlifObject* iomod = nullptr;
	AlifObject* std = nullptr;
	AlifIntT fd{};
	AlifObject* encoding_attr;
	AlifIntT res = 1;
	const AlifConfig* config = alifInterpreter_getConfig(_thread->interpreter);

#ifndef _WINDOWS
	class AlifStatStruct sb;
	if (_alifFStat_noraise(fileno(stdin), &sb) == 0 and
		S_ISDIR(sb.st_mode)) {
		//return ALIFSTATUS_ERR("<stdin> is a directory, cannot continue");
		return -1; //* alif //* delete
	}
#endif

	if (!(iomod = alifImport_importModule("التبادل"))) {
		goto error;
	}

	/* Set sys.stdin */
	fd = fileno(stdin);
	std = create_stdio(config, iomod, fd, 0, "<stdin>",
		config->stdioEncoding,
		config->stdioErrors);
	if (std == nullptr)
		goto error;
	//alifSys_setObject("__stdin__", std);
	//_alifSys_setAttr(&ALIF_ID(stdin), std);
	//ALIF_DECREF(std);

	///* Set sys.stdout */
	//fd = fileno(stdout);
	//std = create_stdio(config, iomod, fd, 1, "<stdout>",
	//	config->stdioEncoding,
	//	config->stdioErrors);
	//if (std == nullptr)
	//	goto error;
	//alifSys_setObject("__stdout__", std);
	//_alifSys_setAttr(&ALIF_ID(stdout), std);
	//ALIF_DECREF(std);

	///* Set sys.stderr, replaces the preliminary stderr */
	//fd = fileno(stderr);
	//std = create_stdio(config, iomod, fd, 1, "<stderr>",
	//	config->stdioEncoding,
	//	L"backslashreplace");
	//if (std == nullptr)
	//	goto error;

	//encoding_attr = alifObject_getAttrString(std, "encoding");
	//if (encoding_attr != nullptr) {
	//	const char* std_encoding = alifUStr_asUTF8(encoding_attr);
	//	if (std_encoding != nullptr) {
	//		AlifObject* codec_info = _alifCodec_lookup(std_encoding);
	//		ALIF_XDECREF(codec_info);
	//	}
	//	ALIF_DECREF(encoding_attr);
	//}
	//_alifErr_clear(_thread);  /* Not a fatal error if codec isn't available */

	//if (alifSys_setObject("__stderr__", std) < 0) {
	//	ALIF_DECREF(std);
	//	goto error;
	//}
	//if (_alifSys_setAttr(&ALIF_ID(stderr), std) < 0) {
	//	ALIF_DECREF(std);
	//	goto error;
	//}
	//ALIF_DECREF(std);

	goto done;

error:
	//res = ALIFSTATUS_ERR("can't initialize sys standard streams");
	//res = -1; //* alif //* delete

done:
	ALIF_XDECREF(iomod);
	return res;
}





void ALIF_NO_RETURN alif_exit(AlifIntT _sts) { // 3383
	AlifThread* thread = _alifThread_get();
	if (thread != nullptr and _alifThread_isRunningMain(thread)) {
		_alifInterpreter_setNotRunningMain(thread->interpreter);
	}
	//if (_alif_finalize(&_alifDureRun_) < 0) {
	//	_sts = 120;
	//}

	exit(_sts);
}



AlifOSSigHandlerT alifOS_setSig(AlifIntT sig, AlifOSSigHandlerT handler) { // 3475
#ifdef HAVE_SIGACTION
	sigaction context, ocontext;
	context.sa_handler = handler;
	sigemptyset(&context.sa_mask);
	context.sa_flags = SA_ONSTACK;
	if (sigaction(sig, &context, &ocontext) == -1)
		return SIG_ERR;
	return ocontext.sa_handler;
#else
	AlifOSSigHandlerT oldhandler{};
	oldhandler = signal(sig, handler);
#ifdef HAVE_SIGINTERRUPT
	siginterrupt(sig, 1);
#endif
	return oldhandler;
#endif
}
