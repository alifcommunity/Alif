#include "alif.h"

//#include "AlifCore_AlifEval.h"
//#include "AlifCore_Code.h"
//#include "AlifCore_Frame.h"
//#include "AlifCore_FreeList.h"
#include "AlifCore_InitConfig.h"
//#include "AlifObject.h"
//#include "AlifCore_LifeCycle.h"
//#include "AlifCore_Memory.h"
#include "AlifCore_State.h"
#include "AlifCore_DureRunInit.h"



#ifdef HAVE_LOCAL_THREAD
	ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_ = nullptr; // 68
#endif

static inline AlifThread* current_fastGet(void)
{
#ifdef HAVE_LOCAL_THREAD
	return _alifTSSThread_;
#else
#  error "no supported thread-local variable storage classifier"
#endif
	}

AlifThread*	alifThread_getCurrent(void) { // 110
	return current_fastGet();
}

//	-------------------------------------------------
//	 the thread state bound to the current OS thread
//	-------------------------------------------------


static inline AlifIntT threadTSS_init(AlifTssT* _key) { // 127
	return alifThreadTSS_create(_key);
}

static inline AlifThread* threadTSS_get(AlifTssT* _key) { // 141
	return (AlifThread*)alifThreadTSS_get(_key);
}


static void bind_thread(AlifThread* tstate) { // 245

	tstate->threadID = alifThread_getThreadID();
#ifdef ALIF_HAVE_THREAD_NATIVE_ID
	tstate->nativeThreadID = alifThread_getThreadNativeID();
#endif

	//alifBRC_initThread(tstate);

	//threadMimalloc_bind(tstate);

	tstate->status.bound = 1;
}
/* ---------------------------------------- AlifCycle ---------------------------------------- */


static const AlifDureRun _initial_ = ALIF_DURERUNSTATE_INIT(_alifDureRun_); // 393



static void init_dureRun(AlifDureRun* _dureRun) { // 411

	_dureRun->mainThreadID = alifThread_getThreadID();

	_dureRun->selfInitialized = 1;
}

AlifIntT alifDureRunState_init(AlifDureRun* _dureRun) { // 441

	if (_dureRun->selfInitialized) {
		memcpy(_dureRun, &_initial_, sizeof(*_dureRun));
	}

	if (threadTSS_init(&_dureRun->autoTSSKey) != 0) {
		alifDureRunState_fini(_dureRun);
		return -1;
	}
	if (alifThreadTSS_create(&_dureRun->trashTSSKey) != 0) {
		alifDureRunState_fini(_dureRun);
		return -1;
	}

	init_dureRun(_dureRun);

	return 1;
}


void alifDureRunState_fini(AlifDureRun* _dureRun) { // 477
	if (alifThreadTSS_isCreated(&_dureRun->autoTSSKey)) {
		alifThreadTSS_delete(&_dureRun->autoTSSKey);
	}

	if (alifThreadTSS_isCreated(&_dureRun->trashTSSKey)) {
		alifThreadTSS_delete(&_dureRun->trashTSSKey);
	}
}



// --------------------
//		lifecycle
// --------------------

AlifIntT alifInterpreter_enable(AlifDureRun* _dureRun) { // 559
	AlifDureRun::AlifInterpreters* interpreters = &_dureRun->interpreters;
	interpreters->nextID = 0;
	return 1;
}



static AlifIntT init_interpreter(AlifInterpreter* _interpreter,
	AlifDureRun* _dureRun, AlifIntT _id, AlifInterpreter* _next) { // 612

	if (_interpreter->initialized) {
		// error
		return -1;
	}

	_interpreter->dureRun = _dureRun;
	_interpreter->id_ = _id;
	_interpreter->next = _next;
	//alifGC_initState(&_interpreter->gc);
	alifConfig_initAlifConfig(&_interpreter->config);


	_interpreter->initialized = 1;
	return 1;
}

AlifIntT alifInterpreter_new(AlifThread* _thread, AlifInterpreter** _interpreterP) { // 674

	*_interpreterP = nullptr;

	AlifDureRun* dureRun = &_alifDureRun_;

	if (_thread != nullptr) {
		//error
		return -1;
	}

	AlifDureRun::AlifInterpreters* interpreters = &dureRun->interpreters;
	AlifIntT id_ = interpreters->nextID;
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
			// memory error
			status = -1;
			goto error;
		}

		memcpy(interpreter, &_initial_.mainInterpreter, sizeof(*interpreter));

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
		//free_interpreter(interpreter);
	}
	return status;
}







static void init_thread(AlifThreadImpl* _thread, AlifInterpreter* _interpreter, AlifUSizeT _id) { // 1460
	AlifThread* thread = (AlifThread*)_thread;
	if (thread->status.initialized) {
		// error
		return;
	}

	thread->interpreter = _interpreter;

	thread->id = _id;

	//thread->alifRecursionLimit = _interpreter->cEval.recursionLimit,
	//thread->alifRecursionRemaining = _interpreter->cEval.recursionLimit,
	thread->cppRecursionRemaining = ALIFCPP_RECURSION_LIMIT;

	thread->currentFrame = nullptr;
	//thread->dataStackChunk = nullptr;
	//thread->dataStackTop = nullptr;
	//thread->dataStackLimit = nullptr;


	llist_init(&_thread->memFreeQueue);

	thread->status.initialized = 1;
}

static void add_thread(AlifInterpreter* _interpreter, AlifThread* _thread, AlifThread* next) { // 1519
	if (next != nullptr) {
		next->prev = _thread;
	}
	_thread->next = next;
	_interpreter->threads.head = _thread;
}

static AlifThread* new_thread(AlifInterpreter* _interpreter) { // 1533

	AlifThreadImpl* thread{};

	AlifDureRun* dureRun = _interpreter->dureRun;
	AlifThreadImpl* newThread = (AlifThreadImpl*)alifMem_dataAlloc(sizeof(AlifThreadImpl));
	AlifIntT usedNewThread{};
	if (newThread == nullptr) {
		return nullptr;
	}

	_interpreter->threads.nextUniquID += 1;
	AlifSizeT id = _interpreter->threads.nextUniquID;

	AlifThread* oldHead = _interpreter->threads.head;
	if (oldHead == nullptr) {
		// It's the interpreter's initial thread state.
		usedNewThread = 0;
		thread = &_interpreter->initialThread;
	}
	else {
		usedNewThread = 1;
		thread = newThread;
		memcpy(thread, &_initial_.mainInterpreter.initialThread, sizeof(*thread));
	}

	init_thread(thread, _interpreter, id);
	add_thread(_interpreter, (AlifThread*)thread, oldHead);

	if (!usedNewThread) {
		alifMem_dataFree(newThread);
	}

	return (AlifThread*)thread;
}


AlifThread* alifThreadState_new(AlifInterpreter* _interpreter) { // 1622
	return new_thread(_interpreter);
}




void alifThread_bind(AlifThread* _thread) { // 2447
	
	bind_thread(_thread);

	if (threadTSS_get(&_thread->interpreter->dureRun->autoTSSKey) == nullptr) {
		//bind_gilstate_tstate(_thread);
	}
}











const AlifConfig* alifInterpreter_getConfig(AlifInterpreter* _interpreter) { // 2881
	return &_interpreter->config;
}
