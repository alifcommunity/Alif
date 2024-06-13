#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifObject.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_Memory.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_DureRunInit.h"



#ifdef HAVE_LOCAL_THREAD
	ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_ = nullptr;
#endif

static const AlifDureRun initial = ALIF_DURERUNSTATE_INIT(_dureRun_);



static void init_dureRun(AlifDureRun* _dureRun) {
	
	//_dureRun->mainThread = alifThread_getThreadIdent();
	_dureRun->mainThreadID = GetCurrentThreadId();

	_dureRun->selfInitialized = 1;
	
}

AlifIntT alifDureRunState_init(AlifDureRun* _dureRun) {

	if (_dureRun->selfInitialized) {
		memcpy(_dureRun, &initial, sizeof(*_dureRun));
	}

	// code here

	init_dureRun(_dureRun);

	return 1;
}



AlifIntT alifInterpreter_enable(AlifDureRun* _dureRun) {
	AlifDureRun::AlifInterpreters * interpreters = &_dureRun->interpreters;
	interpreters->nextID = 0;
	return 1;
}


static AlifIntT init_interpreter(AlifInterpreter* _interpreter,
	AlifDureRun* _dureRun, AlifSizeT _id, AlifInterpreter* _next) {

	if (_interpreter->initialized) {
		// error
	}

	_interpreter->dureRun = _dureRun;
	_interpreter->id_ = _id;
	_interpreter->next = _next;

	alifConfig_initAlifConfig(&_interpreter->config);

	_interpreter->initialized = 1;
	return 1;
}


AlifIntT alifInterpreter_new(AlifThread* _thread, AlifInterpreter** _interpreterP) {

	*_interpreterP = nullptr;

	AlifDureRun* dureRun = &_alifDureRun_;

	if (_thread != nullptr) {
		//error
	}

	AlifDureRun::AlifInterpreters* interpreters = &dureRun->interpreters;
	AlifSizeT id_ = interpreters->nextID;
	interpreters->nextID += 1;

	// حجز المفسر وإضافته الى وقت التشغيل
	AlifInterpreter* interpreter{};
	AlifIntT status{};
	AlifInterpreter* oldHead = interpreters->head;

	if (oldHead == nullptr) {
		interpreter = &dureRun->mainInterpreter;
		interpreters->main = interpreter;
	}
	else {
		interpreter = (AlifInterpreter*)alifMem_dataAlloc(sizeof(AlifInterpreter));
		if (interpreter == nullptr) {
			status = -1;
			goto error;
		}

		memcpy(interpreter, &initial.mainInterpreter, sizeof(*interpreter));

		if (id_ < 0) {
			status = -1;
			goto error;
		}
	}
	interpreters->head = interpreter;

	status = init_interpreter(interpreter, dureRun, id_, oldHead);
	if (status < 1) {
		goto error;
	}

	*_interpreterP = interpreter;
	return 1;

error:
	if (interpreter != nullptr) {
		// clear
	}
	return status;
}




const AlifConfig* alifConfig_get() {
	AlifThread* thread_ = _alifTSSThread_;
	return &thread_->interpreter->config;
}
