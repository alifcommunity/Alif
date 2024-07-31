#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_Memory.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_DureRun.h"
#include "AlifCore_DureRunInit.h"

#ifdef _WINDOWS
#include <windows.h>
#endif


#if defined(_WINDOWS)

#pragma section("_alifDureRun_", read, write)
__declspec(allocate("_alifDureRun_"))

#elif defined(__APPLE__)

__attribute__((
	section(SEG_DATA ",_alifDureRun_")
	))

#endif


AlifDureRun _alifDureRun_
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((section("._alifDureRun_")))
#endif
= ALIF_DURERUNSTATE_INIT(_alifDureRun_);

static AlifIntT dureRunInitialized = 0;

AlifIntT alifDureRun_initialize() {

	if (dureRunInitialized) return 1;

	dureRunInitialized = 1;

	return alifDureRunState_init(&_alifDureRun_);
}



static inline void current_fastSet(AlifDureRun* _dureRun, AlifThread* _thread) {
#ifdef HAVE_LOCAL_THREAD
	_alifTSSThread_ = _thread;
#else
	error L"خطأ ممر" // need fix
#endif
}


static AlifIntT alifCore_initDureRun(AlifDureRun* _dureRun, const AlifConfig* _config) {

	if (_dureRun->initialized) {
		return -1;
	}

	AlifIntT status = alifConfig_write(_config, _dureRun);
	if (status < 1) return status;

	alif_getVersion();

	status = alifImport_init();

	status = alifInterpreter_enable(_dureRun);
	if (status < 1) return status;

	return 1;
}

static AlifThread* alifThread_new(AlifInterpreter* _interpreter) {
	// يحتاج الى عمل فيما بعد

	AlifThread* thread_ = (AlifThread*)alifMem_dataAlloc(sizeof(AlifThread));

	AlifSizeT id = _interpreter->threads.nextUniquID;
	_interpreter->threads.nextUniquID += 1;

	AlifThread* oldHead = _interpreter->threads.head;

	/* init thread func */
	thread_->interpreter = _interpreter;
	thread_->id = id;
	/* init thread func */

	/* add thread func */
	if (oldHead != nullptr) {
		oldHead->prev = thread_;
	}
	thread_->next = oldHead;
	_interpreter->threads.head = thread_;
	/* add thread func */

	return thread_; // temp
}

static AlifIntT alifCore_createInterpreter(AlifDureRun* _dureRun, const AlifConfig* _config, AlifThread** _threadP) {

	AlifIntT status{};
	AlifInterpreter* interpreter{};

	status = alifInterpreter_new(nullptr, &interpreter);
	if (status < 1) return status;

	interpreter->ready = 1;

	memcpy(&interpreter->config, _config, sizeof(AlifConfig));

	//AlifInterpreterConfig interpConfig = ALIFINTERPRETERCONFIG_LEGACY_INIT;
	//status = initInterpreter_settings(interpreter, &interpConfig);
	//if (status < 1) return status;

	if (alifInterpreterMem_init(interpreter) < 1) {
		return -1;
	}

	AlifThread* thread_ = alifThread_new(interpreter);

	if (thread_ == nullptr) {
		// cant make thread // temp
		std::wcout << L"لا يمكن إنشاء ممر" << std::endl;
	}

	_dureRun->mainThread = thread_;

	//initInterpreter_createGlobalLock(thread_, interpConfig.globalLock);
	alifThread_attach(thread_); // temp and need change it's location

	*_threadP = thread_;
	return 1;
}

static AlifIntT alifCore_builtinsInit(AlifThread* _thread) { 

	AlifObject* modules{};
	AlifObject* builtinsDict{};

	AlifInterpreter* interp = _thread->interpreter;

	AlifObject* biMod = alifBuiltin_init(interp);
	if (biMod == nullptr) goto error;

	//modules = interp->imports.modules_; // alifImport_getModule
	//if (alifImport_fixupBuiltin(_thread, biMod, "builtins", modules) < 0) {
	//	goto error;
	//}

	builtinsDict = alifModule_getDict(biMod);
	if (builtinsDict == nullptr) goto error;

	interp->builtins = ALIF_NEWREF(builtinsDict);

	return 1;

error:
	ALIF_XDECREF(biMod);
	return -1;
}

static AlifIntT alifCore_interpreterInit(AlifThread* _thread) {

	AlifInterpreter* interpreter = _thread->interpreter;
	AlifIntT status = 1;
	const AlifConfig* config_;
	AlifObject* sysMod = nullptr;

	status = alifSubGC_init(interpreter);
	if (status < 0) return status;

	//status = alifCore_initTypes(interpreter);
	if (status < 0) goto done;

	//status = alifAtExit_init(interpreter);
	if (status < 0) return status;

	status = alifSys_create(_thread, &sysMod);
	if (status < 0) goto done;

	status = alifCore_builtinsInit(_thread);
	if (status < 0) goto done;

	config_ = &interpreter->config;

	//status = alifImport_initCore(thread, sysMod, config_->installImportLib);
	if (status < 0) goto done;

done:
	//Py_XDECREF(sysmod);
	return status;

}

static AlifIntT alifInit_config(AlifDureRun* _dureRun, AlifThread** _threadP, const AlifConfig* _config) {

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

static AlifIntT alifInit_core(AlifDureRun* _dureRun, const AlifConfig* _config, AlifThread** _tStateP) {

	AlifIntT status{};

	AlifConfig config_{};
	alifConfig_initAlifConfig(&config_);

	// نسخ التهيئة المصدرية
	memcpy(&config_, _config, sizeof(config_)); // يجب مراجعتها

	status = alifConfig_read(&config_);
	if (status < 1) goto done;

	if (!_dureRun->coreInitialized) {
		status = alifInit_config(_dureRun, _tStateP, &config_);
	}
	else {
		//status = alifInit_coreReconfig(_dureRun, _tStateP, &config_);
	}

	if (status < 1) goto done;

done:
	//alifConfig_clear(&config_);
	return status;
}

static AlifIntT initInterpreter_main(AlifThread* _thread) {

	AlifIntT status = 1;
	AlifIntT isMainInterpreter = _thread->interpreter == _alifDureRun_.interpreters.main;
	AlifInterpreter* interpreter = _thread->interpreter;
	const AlifConfig* config_ = &interpreter->config;

	//if (!config_->installImportLib) {
	//	if (isMainInterpreter) {
	//		interpreter->dureRun->initialized = 1;
	//	}
	//	return 1;
	//}

	//status = alifConfig_initImportConfig(&interpreter->config);
	if (status < 1) return status;

	//if (interpreter_updateConfig(_thread, 1) < 0) {
	//	return -1;
	//}

	//status = alifImport_initExternal(_thread);
	if (status < 1) return status;

	//status = alifUnicode_initEncoding(_thread);
	if (status < 1) return status;

	if (isMainInterpreter) {

		if (config_->tracemalloc) {
			//if (alifTraceMalloc_start(config_->tracemalloc) < 0) {
			//	return -1;
			//}
		}
	}

	//status = init_sysStream(_thread);
	if (status < 1) return status;

	//status = init_setBuiltinsOpen();
	if (status < 1) return status;

	//status = add_mainmodule(interpreter);
	if (status < 1) return status;

	// need work

	return 1;

}

static AlifIntT alifInit_main(AlifThread* _thread) {

	AlifInterpreter* interpreter = _thread->interpreter;
	if (!interpreter->dureRun->coreInitialized) {
		return -1;
	}

	if (interpreter->dureRun->initialized) {
		//return alifInit_mainReconfigure(_thread);
	}

	AlifIntT status = initInterpreter_main(_thread);
	if (status < 1) return status;

	return 1;
}

AlifIntT alif_initFromConfig(AlifConfig* _config) {

	if (_config == nullptr) {
		return -1;
	}

	AlifIntT status{};

	status = alifDureRun_initialize();
	if (status < 1) return status;

	AlifDureRun* dureRun = &_alifDureRun_;
	AlifThread* thread_ = nullptr;

	status = alifInit_core(dureRun, _config, &thread_);
	if (status < 1) return status;

	//_config = alifInterpreterState_getConfig(thread_->interpreter);
	_config = &thread_->interpreter->config;

	if (_config->initMain) {
		status = alifInit_main(thread_);
		if (status < 1) return status;
	}

	return 1;
}


void alifThread_attach(AlifThread* _thread)
{
	current_fastSet(&_alifDureRun_, _thread);
}
