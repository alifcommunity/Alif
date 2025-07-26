#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_Frame.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_ParkingLot.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Memory.h"
#include "AlifCore_State.h"
#include "AlifCore_RuntimeInit.h"



#ifdef HAVE_LOCAL_THREAD
ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_ = nullptr; // 68
#endif

static inline AlifThread* current_fastGet() { // 71
#ifdef HAVE_LOCAL_THREAD
	return _alifTSSThread_;
#else
#  error "خطأ ممر"
#endif
}

static inline void current_fastSet(AlifThread* _thread) { // 82
#ifdef HAVE_LOCAL_THREAD
	_alifTSSThread_ = _thread;
#else
	error "خطأ ممر"
	#endif
}

static inline void current_fastClear() { // 94
#ifdef HAVE_LOCAL_THREAD
	_alifTSSThread_ = nullptr;
#else
	error "خطأ ممر"
	#endif
}

AlifThread* alifThread_getCurrent() { // 110
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

static inline AlifIntT threadTSS_set(AlifTssT* _key, AlifThread* _thread) { // 148
	return alifThreadTSS_set(_key, (void*)_thread);
}




//#define GILSTATE_TSS_INITIALIZED(runtime) \
//    threadTSS_initialized(&(runtime)->autoTSSKey)
#define GILSTATE_TSS_INIT(runtime) \
    threadTSS_init(&(runtime)->autoTSSKey)
//#define GILSTATE_TSS_FINI(runtime) \
//    threadTSS_fini(&(runtime)->autoTSSKey)
#define GILSTATE_TSS_GET(runtime) \
    threadTSS_get(&(runtime)->autoTSSKey)
#define _GILSTATE_TSS_SET(runtime, thread) \
    threadTSS_set(&(runtime)->autoTSSKey, thread)
//#define _GILSTATE_TSS_CLEAR(runtime) \
//    threadTSS_clear(&(runtime)->autoTSSKey)
//#define GILSTATE_TSS_REINIT(runtime) \
//    threadTSS_reinit(&(runtime)->autoTSSKey)


static void thread_mimallocBind(AlifThread*); // 243

static void bind_thread(AlifThread* _thread) { // 245

	_thread->threadID = alifThread_getThreadID();
#ifdef ALIF_HAVE_THREAD_NATIVE_ID
	_thread->nativeThreadID = alifThread_getThreadNativeID();
#endif

	alifBRC_initThread(_thread);

	thread_mimallocBind(_thread);

	_thread->status.bound = 1;
}





static void bind_gilStateThread(AlifThread* tstate) { // 318

	AlifRuntime* runtime = tstate->interpreter->runtime;
	AlifThread* tcur = threadTSS_get(&runtime->autoTSSKey);

	if (tcur != nullptr) {
		tcur->status.boundGILState = 0;
	}
	threadTSS_set(&runtime->autoTSSKey, tstate);
	tstate->status.boundGILState = 1;
}





/* ------------------------------ AlifCycle ------------------------------- */


static const AlifRuntime _initial_ = ALIF_DURERUNSTATE_INIT(_alifRuntime_); // 393



static void init_runtime(AlifRuntime* _runtime, void* _openCodeHook, void* _openCodeUserdata,
	AlifAuditHookEntry* _auditHookHead, AlifSizeT _unicodeNextIndex) { // 411

	_runtime->openCodeHook = (AlifOpenCodeHookFunction)_openCodeHook;
	_runtime->openCodeUserdata = _openCodeUserdata;
	_runtime->auditHooks.head = _auditHookHead;

	alifPreConfig_initAlifConfig(&_runtime->preConfig);

	_runtime->mainThreadID = alifThread_getThreadID();

	_runtime->unicodeState.ids.nextIndex = _unicodeNextIndex;

	_runtime->selfInitialized = 1;
}

AlifStatus _alifRuntimeState_init(AlifRuntime* _runtime) { // 441

	void* openCodeHook = _runtime->openCodeHook;
	void* openCodeUserdata = _runtime->openCodeUserdata;
	AlifAuditHookEntry* auditHookHead = _runtime->auditHooks.head;
	AlifSizeT unicodeNextIndex = _runtime->unicodeState.ids.nextIndex;

	if (_runtime->selfInitialized) {
		memcpy(_runtime, &_initial_, sizeof(*_runtime));
		//memcpy(_runtime->debugOffsets.cookie, ALIF_DEBUG_COOKIE, 8);

	}

	if (GILSTATE_TSS_INIT(_runtime) != 0) {
		_alifRuntimeState_fini(_runtime);
		return ALIFSTATUS_NO_MEMORY();
	}
	if (alifThreadTSS_create(&_runtime->trashTSSKey) != 0) {
		_alifRuntimeState_fini(_runtime);
		return ALIFSTATUS_NO_MEMORY();
	}

	init_runtime(_runtime, openCodeHook, openCodeUserdata, auditHookHead,
		unicodeNextIndex);

	return ALIFSTATUS_OK();
}


void _alifRuntimeState_fini(AlifRuntime* _Runtime) { // 477
	if (alifThreadTSS_isCreated(&_Runtime->autoTSSKey)) {
		alifThreadTSS_delete(&_Runtime->autoTSSKey);
	}

	if (alifThreadTSS_isCreated(&_Runtime->trashTSSKey)) {
		alifThreadTSS_delete(&_Runtime->trashTSSKey);
	}
}



// --------------------
//		lifecycle
// --------------------

AlifStatus alifInterpreter_enable(AlifRuntime* _Runtime) { // 559
	AlifRuntime::AlifInterpreters* interpreters = &_Runtime->interpreters;
	interpreters->nextID = 0;
	return ALIFSTATUS_OK();
}



static AlifStatus init_interpreter(AlifInterpreter* _interpreter,
	AlifRuntime* _Runtime, AlifIntT _id, AlifInterpreter* _next) { // 612

	if (_interpreter->initialized) {
		return ALIFSTATUS_ERR("interpreter already initialized");
	}

	_interpreter->runtime = _Runtime;
	_interpreter->id_ = _id;
	_interpreter->next = _next;
	alifGC_initState(&_interpreter->gc);
	alifConfig_initAlifConfig(&_interpreter->config);

	alifBRC_initState(_interpreter);
	llist_init(&_interpreter->memFreeQueue.head);

	_interpreter->initialized = 1;
	return ALIFSTATUS_OK();
}

AlifStatus alifInterpreter_new(AlifThread* _thread, AlifInterpreter** _interpreterP) { // 674

	*_interpreterP = nullptr;

	AlifRuntime* dureRun = &_alifRuntime_;

	if (_thread != nullptr) {
		//error
		return ALIFSTATUS_ERR("sys.audit failed");
	}

	AlifRuntime::AlifInterpreters* interpreters = &dureRun->interpreters;
	AlifIntT id_ = interpreters->nextID;
	interpreters->nextID += 1;

	// حجز المفسر وإضافته الى وقت التشغيل
	AlifInterpreter* interpreter{};
	AlifStatus status{};
	AlifInterpreter* oldHead = interpreters->head;

	if (oldHead == nullptr) {
		interpreter = &dureRun->mainInterpreter;
		interpreters->main = interpreter;
	}
	else {
		interpreter = (AlifInterpreter*)alifMem_dataAlloc(sizeof(AlifInterpreter));
		if (interpreter == nullptr) {
			status = ALIFSTATUS_NO_MEMORY();
			goto error;
		}

		memcpy(interpreter, &_initial_.mainInterpreter, sizeof(*interpreter));

		if (id_ < 0) {
			status = ALIFSTATUS_ERR("failed to get an interpreter ID");
			goto error;
		}
	}
	interpreters->head = interpreter;

	status = init_interpreter(interpreter, dureRun, id_, oldHead);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto error;
	}

	*_interpreterP = interpreter;
	return ALIFSTATUS_OK();

error:
	if (interpreter != nullptr) {
		//free_interpreter(interpreter);
	}
	return status;
}


static inline void set_mainThread(AlifInterpreter* _interp,
	AlifThread* _thread) { // 1044
	alifAtomic_storePtrRelaxed(&_interp->threads.main, _thread);
}

static inline AlifThread* get_mainThread(AlifInterpreter* _interp) { // 1050
	return (AlifThread*)alifAtomic_loadPtrRelaxed(&_interp->threads.main);
}

AlifIntT alifInterpreter_setRunningMain(AlifInterpreter* _interp) { // 1056
	if (alifInterpreter_failIfRunningMain(_interp) < 0) {
		return -1;
	}
	AlifThread* tstate = current_fastGet();
	ALIF_ENSURETHREADNOTNULL(tstate);
	if (tstate->interpreter != _interp) {
		//alifErr_setString(_alifExcDureRunError_,
		//	"current tstate has wrong interpreter");
		return -1;
	}
	set_mainThread(_interp, tstate);

	return 0;
}

void _alifInterpreter_setNotRunningMain(AlifInterpreter* _interp) { // 1074
	set_mainThread(_interp, nullptr);
}

AlifIntT _alifThread_isRunningMain(AlifThread* _thread) { // 1092
	AlifInterpreter* interp = _thread->interpreter;
	return get_mainThread(interp) == _thread;
}

AlifIntT alifInterpreter_failIfRunningMain(AlifInterpreter* _interp) { // 1105
	if (get_mainThread(_interp) != nullptr) {
		//alifErr_setString(_alifExcInterpreterError_,
		//	"interpreter already running");
		return -1;
	}
	return 0;
}


int64_t alifInterpreter_getID(AlifInterpreter* _interp) { // 1226
	if (_interp == nullptr) {
		//alifErr_setString(_alifExcDureRunError_, "no interpreter provided");
		return -1;
	}
	return _interp->id_;
}


AlifInterpreter* alifInterpreter_get() { // 1331
	AlifThread* tstate = current_fastGet();
	ALIF_ENSURETHREADNOTNULL(tstate);
	AlifInterpreter* interp = tstate->interpreter;
	if (interp == nullptr) {
		//alif_fatalError("no current interpreter");
		return nullptr; //* alif //* delete
	}
	return interp;
}

#define DATA_STACK_CHUNK_SIZE (16*1024) // 1414

static AlifStackChunk* allocate_chunk(AlifIntT _sizeInBytes, AlifStackChunk* _previous) { // 1417
	AlifStackChunk* res_ = (AlifStackChunk*)alifMem_dataAlloc(_sizeInBytes); //* alif
	if (res_ == nullptr) {
		return nullptr;
	}
	res_->previous = _previous;
	res_->size = _sizeInBytes;
	res_->top = 0;
	return res_;
}



static void init_thread(AlifThreadImpl* _thread, AlifInterpreter* _interpreter, AlifUSizeT _id) { // 1460
	AlifThread* thread = (AlifThread*)_thread;
	if (thread->status.initialized) {
		// error
		return;
	}

	thread->interpreter = _interpreter;

	thread->id = _id;

	thread->alifRecursionLimit = _interpreter->eval.recursionLimit,
		thread->alifRecursionRemaining = _interpreter->eval.recursionLimit,
		thread->cppRecursionRemaining = ALIFCPP_RECURSION_LIMIT;

	thread->excInfo = &thread->excState;

	thread->currentFrame = nullptr;
	thread->dataStackChunk = nullptr;
	thread->dataStackTop = nullptr;
	thread->dataStackLimit = nullptr;


	llist_init(&_thread->memFreeQueue);

	thread->status.initialized = 1;
}

static void add_thread(AlifInterpreter* _interpreter,
	AlifThread* _thread, AlifThread* next) { // 1519
	if (next != nullptr) {
		next->prev = _thread;
	}
	_thread->next = next;
	_interpreter->threads.head = _thread;
}

static AlifThread* new_thread(AlifInterpreter* _interpreter) { // 1533

	AlifThreadImpl* thread{};

	AlifRuntime* dureRun = _interpreter->runtime;
	AlifThreadImpl* newThread = (AlifThreadImpl*)alifMem_dataAlloc(sizeof(AlifThreadImpl));
	AlifIntT usedNewThread{};
	if (newThread == nullptr) {
		return nullptr;
	}

	AlifSizeT qsbrIDx = alifQSBR_reserve(_interpreter);
	if (qsbrIDx < 0) {
		alifMem_dataFree(newThread);
		return nullptr;
	}

	HEAD_LOCK(dureRun);

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

	HEAD_UNLOCK(dureRun);
	if (!usedNewThread) {
		alifMem_dataFree(newThread);
	}

	// Must be called with lock unlocked to avoid lock ordering deadlocks.
	alifQSBR_register(thread, _interpreter, qsbrIDx);

	return (AlifThread*)thread;
}


AlifThread* alifThreadState_new(AlifInterpreter* _interpreter) { // 1622
	return new_thread(_interpreter);
}


AlifObject* _alifThreadState_getDict(AlifThread* _thread) { // 1941
	if (_thread->dict == nullptr) {
		_thread->dict = alifDict_new();
		if (_thread->dict == nullptr) {
			//_alifErr_clear(_thread);
		}
	}
	return _thread->dict;
}

AlifObject* alifThreadState_getDict() { // 1955
	AlifThread* thread = current_fastGet();
	if (thread == nullptr) {
		return nullptr;
	}
	return _alifThreadState_getDict(thread);
}


static inline void thread_activate(AlifThread* _thread) { // 1998
	if (!_thread->status.boundGILState) {
		bind_gilStateThread(_thread);
	}

	_thread->status.active = 1;
}

static inline void thread_deactivate(AlifThread* _thread) { // 2015
	_thread->status.active = 0;
}

static AlifIntT thread_tryAttach(AlifThread* tstate) { // 2029
	AlifIntT expected = ALIF_THREAD_DETACHED;
	return alifAtomic_compareExchangeInt(&tstate->state,
		&expected, ALIF_THREAD_ATTACHED);
}


static void thread_waitAttach(AlifThread* _thread) { // 2055
	do {
		AlifIntT expected = ALIF_THREAD_SUSPENDED;

		alifParkingLot_park(&_thread->state, &expected, sizeof(_thread->state),
			/*timeout=*/-1, nullptr, /*detach=*/0);

	}
	while (!thread_tryAttach(_thread));
}

static void thread_setDetached(AlifThread* _thread, AlifIntT _detachedState) { // 2044
	alifAtomic_storeInt(&_thread->state, _detachedState);
}


void alifThread_attach(AlifThread* _thread) { // 2070
	ALIF_ENSURETHREADNOTNULL(_thread);
	if (current_fastGet() != nullptr) {
		//alif_fatalError("non-nullptr old thread state");
		return; // temp
	}


	while (1) {
		alifEval_acquireLock(_thread);

		current_fastSet(_thread);
		thread_activate(_thread);

		if (!thread_tryAttach(_thread)) {
			thread_waitAttach(_thread);
		}

		if (alifEval_isGILEnabled(_thread) and !_thread->status.holdsGIL) {
			thread_setDetached(_thread, ALIF_THREAD_DETACHED);
			thread_deactivate(_thread);
			current_fastClear();
			continue;
		}
		alifQSBR_attach(((AlifThreadImpl*)_thread)->qsbr);
		break;
	}

	if (_thread->criticalSection != 0) {
		alifCriticalSection_resume(_thread);
	}
}

static void detach_thread(AlifThread* _thread, AlifIntT detachedState) { // 2122

	if (_thread->criticalSection != 0) {
		alifCriticalSection_suspendAll(_thread);
	}

	alifQSBR_detach(((AlifThreadImpl*)_thread)->qsbr);

	thread_deactivate(_thread);
	thread_setDetached(_thread, detachedState);
	current_fastClear();
	alifEval_releaseLock(_thread->interpreter, _thread, 0);
}

void alifThread_detach(AlifThread* _thread) { // 2140
	detach_thread(_thread, ALIF_THREAD_DETACHED);
}

static AlifInterpreter* interp_forStopTheWorld(StopTheWorldState* _stw) { // 2196
	return (_stw->isGlobal
		? alifInterpreter_head()
		: ALIF_CONTAINER_OF(_stw, AlifInterpreter, stopTheWorld));
}

#define ALIF_FOR_EACH_THREAD(stw, i, t)                                       \
    for (i = interp_forStopTheWorld((stw));                              \
            i != nullptr; i = ((stw->isGlobal) ? i->next : nullptr))             \
        for (t = i->threads.head; t; t = t->next) // 2206


static bool park_detachedThreads(StopTheWorldState* stw) { // 2214
	AlifIntT numParked = 0;
	AlifInterpreter* i{};
	AlifThread* t{};
	ALIF_FOR_EACH_THREAD(stw, i, t) {
		AlifIntT state = alifAtomic_loadIntRelaxed(&t->state);
		if (state == ALIF_THREAD_DETACHED) {
			if (alifAtomic_compareExchangeInt(&t->state,
				&state, ALIF_THREAD_SUSPENDED)) {
				numParked++;
			}
		}
		else if (state == ALIF_THREAD_ATTACHED and t != stw->requester) {
			alifSet_evalBreakerBit(t, ALIF_EVAL_PLEASE_STOP_BIT);
		}
	}
	stw->threadCountDown -= numParked;
	return numParked > 0 and stw->threadCountDown == 0;
}


static void stop_theWorld(StopTheWorldState* _stw) { // 2239
	AlifRuntime* dureRun = &_alifRuntime_;

	ALIFMUTEX_LOCK(&_stw->mutex);
	if (_stw->isGlobal) {
		alifRWMutex_lock(&dureRun->stopTheWorldMutex);
	}
	else {
		alifRWMutex_rLock(&dureRun->stopTheWorldMutex);
	}

	HEAD_LOCK(dureRun);
	_stw->requested = 1;
	_stw->threadCountDown = 0;
	_stw->stopEvent = AlifEvent();
	_stw->requester = _alifThread_get();

	AlifInterpreter* i{};
	AlifThread* t{};
	ALIF_FOR_EACH_THREAD(_stw, i, t) {
		if (t != _stw->requester) {
			_stw->threadCountDown++;
		}
	}

	if (_stw->threadCountDown == 0) {
		HEAD_UNLOCK(dureRun);
		_stw->worldStopped = 1;
		return;
	}

	for (;;) {
		bool stoppedAllThreads = park_detachedThreads(_stw);
		HEAD_UNLOCK(dureRun);

		if (stoppedAllThreads) {
			break;
		}

		AlifTimeT waitNS = 1000 * 1000;
		int detach = 0;
		if (alifEvent_waitTimed(&_stw->stopEvent, waitNS, detach)) {
			break;
		}

		HEAD_LOCK(dureRun);
	}
	_stw->worldStopped = 1;
}


static void start_theWorld(class StopTheWorldState* _stw) { // 2294
	AlifRuntime* dureRun = &_alifRuntime_;
	HEAD_LOCK(dureRun);
	_stw->requested = 0;
	_stw->worldStopped = 0;
	AlifInterpreter* i{};
	AlifThread* t{};
	ALIF_FOR_EACH_THREAD(_stw, i, t) {
		if (t != _stw->requester) {
			alifAtomic_storeInt(&t->state, ALIF_THREAD_DETACHED);
			alifParkingLot_unparkAll(&t->state);
		}
	}
	_stw->requester = nullptr;
	HEAD_UNLOCK(dureRun);
	if (_stw->isGlobal) {
		alifRWMutex_unlock(&dureRun->stopTheWorldMutex);
	}
	else {
		alifRWMutex_rUnlock(&dureRun->stopTheWorldMutex);
	}
	alifMutex_unlock(&_stw->mutex);
}


void alifEval_stopTheWorld(AlifInterpreter* _interp) { // 2342
	stop_theWorld(&_interp->stopTheWorld);
}

void alifEval_startTheWorld(AlifInterpreter* _interp) { // 2350
	start_theWorld(&_interp->stopTheWorld);
}


AlifThread* alifThread_get() { // 2419
	AlifThread* thread = current_fastGet();
	ALIF_ENSURETHREADNOTNULL(thread);
	return thread;
}


void alifThread_bind(AlifThread* _thread) { // 2447

	bind_thread(_thread);

	if (threadTSS_get(&_thread->interpreter->runtime->autoTSSKey) == nullptr) {
		bind_gilStateThread(_thread);
	}
}

uintptr_t alif_getThreadLocalAddr(void) { // 2463
#ifdef HAVE_LOCAL_THREAD
	return (uintptr_t)&_alifTSSThread_;
#else
#  error "no supported thread-local variable storage classifier"
#endif
}


AlifInterpreter* alifInterpreter_head() { // 2485
	return _alifRuntime_.interpreters.head;
}




AlifStatus alifGILState_init(AlifInterpreter* _interp) { // 2652
	if (!alif_isMainInterpreter(_interp)) {
		return ALIFSTATUS_OK();
	}
	AlifRuntime* dureRun = _interp->runtime;
	dureRun->gilState.autoInterpreterState = _interp;
	return ALIFSTATUS_OK();
}








const AlifConfig* alifInterpreter_getConfig(AlifInterpreter* _interpreter) { // 2881
	return &_interpreter->config;
}


const AlifConfig* alif_getConfig() { // 2903
	AlifThread* threadState = current_fastGet();
	ALIF_ENSURETHREADNOTNULL(threadState);
	return alifInterpreter_getConfig(threadState->interpreter);
}


#define MINIMUM_OVERHEAD 1000 // 2919

static AlifObject** push_chunk(AlifThread* _tState, AlifIntT _size) { // 2922
	AlifIntT allocateSize = DATA_STACK_CHUNK_SIZE;
	while (allocateSize < (AlifIntT)sizeof(AlifObject*) * (_size + MINIMUM_OVERHEAD)) {
		allocateSize *= 2;
	}
	AlifStackChunk* new_ = allocate_chunk(allocateSize, _tState->dataStackChunk);
	if (new_ == nullptr) {
		return nullptr;
	}
	if (_tState->dataStackChunk) {
		_tState->dataStackChunk->top = _tState->dataStackTop -
			&_tState->dataStackChunk->data[0];
	}
	_tState->dataStackChunk = new_;
	_tState->dataStackLimit = (AlifObject**)(((char*)new_) + allocateSize);
	AlifObject** res_ = &new_->data[new_->previous == nullptr];
	_tState->dataStackTop = res_ + _size;
	return res_;
}

AlifInterpreterFrame* _alifThreadState_pushFrame(AlifThread* _thread, AlifUSizeT _size) { // 2946
	if (_alifThreadState_hasStackSpace(_thread, (AlifIntT)_size)) {
		AlifInterpreterFrame* res = (AlifInterpreterFrame*)_thread->dataStackTop;
		_thread->dataStackTop += _size;
		return res;
	}
	return (AlifInterpreterFrame*)push_chunk(_thread, (AlifIntT)_size);
}


void _alifThreadState_popFrame(AlifThread* tstate, AlifInterpreterFrame* _frame) { // 2958
	AlifObject** base = (AlifObject**)_frame;
	if (base == &tstate->dataStackChunk->data[0]) {
		AlifStackChunk* chunk = tstate->dataStackChunk;
		AlifStackChunk* previous = chunk->previous;

		tstate->dataStackTop = &previous->data[previous->top];
		tstate->dataStackChunk = previous;
		alifMem_dataFree(chunk); //* alif
		tstate->dataStackLimit = (AlifObject**)(((char*)previous) + previous->size);
	}
	else {
		tstate->dataStackTop = base;
	}
}



AlifIntT alifThreadState_mustExit(AlifThread* _thread) { // 3004
	unsigned long finalizing_id = alifDureRunState_getFinalizingID(&_alifRuntime_);
	AlifThread* finalizing = alifDureRunState_getFinalizing(&_alifRuntime_);

	if (finalizing == nullptr) {
		finalizing = alifInterpreterState_getFinalizing(_thread->interpreter);
		finalizing_id = alifInterpreterState_getFinalizingID(_thread->interpreter);
	}
	if (finalizing == nullptr) return 0;
	else if (finalizing == _thread) return 0;
	else if (finalizing_id == alifThread_getThreadID()) return 0;
	return 1;
}




/* mimalloc memory support */

static void thread_mimallocBind(AlifThread* _thread) { // 3037
	MimallocThreadState* mts = &((AlifThreadImpl*)_thread)->mimalloc;

	mi_tld_t* tld = &mts->tld;
	_mi_tld_init(tld, &mts->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Mem]);
	llist_init(&mts->pageList);

	tld->segments.abandoned = &_thread->interpreter->mimalloc.abandonedPool;

	AlifUSizeT baseOffset = offsetof(AlifObject, type);
	//if (alifMem_debugEnabled()) {
	//	baseOffset += 2 * sizeof(AlifUSizeT);
	//}
	AlifUSizeT debug_offsets[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Count] = {
		{},
		baseOffset,
		baseOffset,
		baseOffset + 2 * sizeof(AlifObject*),
	};

	// Initialize each heap
	for (uint8_t i = 0; i < AlifMimallocHeapID_::Alif_Mimalloc_Heap_Count; i++) {
		_mi_heap_init_ex(&mts->heaps[i], tld, _mi_arena_id_none(), false, i);
		mts->heaps[i].debug_offset = (uint8_t)debug_offsets[i];
	}

	mts->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Object].page_use_qsbr = true;
	mts->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_GC].page_use_qsbr = true;
	mts->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_GCPre].page_use_qsbr = true;

	mts->currentObjectHeap = &mts->heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Object];

	alifAtomic_storeInt(&mts->initialized, 1);
}
