


#include "alif.h"
#include "alifCore_ceval.h"
#include "alifCore_code.h"          
//#include "alifCore_dtoa.h"         
//#include "alifCore_frame.h"
#include "alifCore_initconfig.h"
#include "alifCore_object.h"       
//#include "alifEore_alifErrors.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifMem.h"      
#include "alifCore_alifState.h"
#include "alifCore_runtimeInit.h"
//#include "alifCore_sysModule.h"
 #include "alifCore_typeObject.h"   
//#include "alifCore_weakref.h"    
















































#ifdef HAVE_THREAD_LOCAL
ALIF_THREAD_LOCAL AlifThreadState* alifTssTState = nullptr;
#endif

static inline AlifThreadState* current_fastGet(AlifRuntimeState* ALIF_UNUSED(_runtime))
{
#ifdef HAVE_THREAD_LOCAL
	return alifTssTState;
#else
#  error "no supported thread-local variable storage classifier"
#endif
}


static inline void current_fastSet(AlifRuntimeState* ALIF_UNUSED(_runtime), AlifThreadState* _tState)
{
	//assert(_tState != NULL);
#ifdef HAVE_THREAD_LOCAL
	alifTssTState = _tState;
#else

#  error "no supported thread-local variable storage classifier"
#endif
}


static inline void current_fastClear(AlifRuntimeState* ALIF_UNUSED(_runtime))
{
#ifdef HAVE_THREAD_LOCAL
	alifTssTState = nullptr;
#else

#  error "no supported thread-local variable storage classifier"
#endif
}


















static inline int tState_tssInitialized(AlifTssT* _key)
{
	return alifThread_tssIsCreated(_key);
}


static inline int tState_tssInit(AlifTssT* _key)
{

	return alifThread_tssCreate(_key);
}


static inline void tState_tssFini(AlifTssT* _key)
{

	alifThread_tssDelete(_key);
}


static inline AlifThreadState* tState_tssGet(AlifTssT* _key)
{
	//assert(tState_tssInitialized(_key));
	return (AlifThreadState*)alifThread_tssGet(_key);
}


















































#define GILSTATE_TSS_INITIALIZED(_runtime) \
    tState_tssInitialized(&(_runtime)->autoTSSKey)
#define GILSTATE_TSS_INIT(_runtime) \
    tState_tssInit(&(_runtime)->autoTSSKey)
#define GILSTATE_TSS_FINI(_runtime) \
    tState_tssFini(&(_runtime)->autoTSSKey)
#define GILSTATE_TSS_GET(_runtime) \
    tState_tssGet(&(_runtime)->autoTSSKey)




































static void bind_gilState_tState(AlifThreadState*);
static void unbind_gilState_tState(AlifThreadState*);

static void bind_tState(AlifThreadState* _tState)
{
	//assert(_tState != nullptr);
	//assert(tstate_is_alive(_tState) && !_tState->_status.bound);
	//assert(!_tState->_status.unbound);  // just in case
	//assert(!_tState->_status.bound_gilstate);
	//assert(_tState != gilstate_tss_get(_tState->interp->runtime));
	//assert(!_tState->_status.active);
	//assert(_tState->thread_id == 0);
	//assert(_tState->native_thread_id == 0);




	_tState->threadID = alifThread_getThreadIdent();
#ifdef ALIFHAVE_THREAD_NATIVEID
	_tState->nativeThreadID = alifThread_getThread_nativeID();
#endif

	_tState->status.bound = 1;
}











































static void bind_gilState_tState(AlifThreadState* _tState)
{
	//assert(_tState != nullptr);
	//assert(tState_isAlive(_tState));
	//assert(tState_isBound(_tState));
	// XXX assert(!_tState->_status.active);
	//assert(!_tState->_status.boundGilState);

	AlifRuntimeState* runtime = _tState->interp->runtime;
	AlifThreadState* tCur = GILSTATE_TSS_GET(runtime);
	//assert(_tState != tCur);

	if (tCur != nullptr) {
		tCur->status.boundGilState = 0;
	}
	GILSTATE_TSS_GET(runtime, _tState);
	_tState->status.boundGilState = 1;
}
























































static const AlifRuntimeState initial = ALIFRUNTIMESTATE_INIT(alifRuntime);
ALIFCOMP_DIAGPOP

#define NUMLOCKS 9
#define LOCKS_INIT(_runtime) \
    { \
        &(_runtime)->alifInterpreters.mutex, \
        &(_runtime)->allocators.mutex, \
    }

static int alloc_forRuntime(AlifThreadTypeLock _locks[NUMLOCKS])
{


	AlifMemAllocatorEx old_alloc;
	alifMem_setDefaultAllocator(AlifMem_Domain_Raw, &old_alloc);

	for (int i = 0; i < NUMLOCKS; i++) {
		AlifThreadTypeLock lock = alifThread_allocateLock();
		if (lock == nullptr) {
			for (int j = 0; j < i; j++) {
				alifThread_freeLock(_locks[j]);
				_locks[j] = nullptr;
			}
			break;
		}
		_locks[i] = lock;
	}

	alifMem_setAllocator(AlifMem_Domain_Raw, &old_alloc);
	return 0;
}









static void init_runtime(AlifRuntimeState* _runtime,
	////void* _openCodeHook, void* _openCodeUserdata,
	////void* _auditHookHead,
	AlifSizeT _unicodeNextIndex, AlifThreadTypeLock _locks[NUMLOCKS])
{








	////_runtime->openCodeHook = _openCodeHook;
	////_runtime->openCodeUserdata = _openCodeUserdata;
	////_runtime->auditHooks.head = _auditHookHead;

	alifPreConfig_initAlifConfig(&_runtime->preConfig);

	AlifThreadTypeLock* lockptrs[NUMLOCKS] = LOCKS_INIT(_runtime);
	//for (int i = 0; i < NUMLOCKS; i++) {

		//*lockptrs[i] = _locks[i];
	//}


	_runtime->mainThread = alifThread_getThreadIdent();

	//_runtime->unicodeState.ids.next_index = _unicodeNextIndex;

	_runtime->initialized = 1;
}


void alifRuntimeState_init(AlifRuntimeState* _runtime)
{








	//AlifSizeT unicodeNextIndex = _runtime->unicodeState.IDs.nextIndex;

	AlifThreadTypeLock locks[NUMLOCKS];
	alloc_forRuntime(locks);



	if (_runtime->initialized) {


		memcpy(_runtime, &initial, sizeof(*_runtime));
	}

	if (GILSTATE_TSS_INIT(_runtime) != 0) {
		//alifRuntimeState_fini(_runtime);
		exit(-1);
	}

	if (alifThread_tssCreate(&_runtime->trashTSSKey) != 0) {
		//alifRuntimeState_fini(_runtime);
		exit(-1);
	}

	init_runtime(_runtime, 0, locks);


}



//void alifRuntimeState_fini(AlifRuntimeState* _runtime)
//{
//
//
//
//
//
//	if (GILSTATE_TSS_INITIALIZED(_runtime)) {
//		GILSTATE_TSS_FINI(_runtime);
//	}
//
//	if (alifThread_tssIsCreated(&_runtime->trashTSSKey)) {
//		alifThread_tssDelete(&_runtime->trashTSSKey);
//	}
//
//
//	AlifMemAllocatorEx old_alloc;
//	alifMem_setDefaultAllocator(AlifMem_Domain_Raw, &old_alloc);
//#define FREE_LOCK(LOCK) \
//    if (LOCK != nullptr) { \
//        alifThread_freeLock(LOCK); \
//        LOCK = nullptr; \
//    }
//
//	AlifThreadTypeLock* lockptrs[NUMLOCKS] = LOCKS_INIT(_runtime);
//	for (int i = 0; i < NUMLOCKS; i++) {
//		FREE_LOCK(*lockptrs[i]);
//	}
//
//#undef FREE_LOCK
//	alifMem_setAllocator(AlifMem_Domain_Raw, &old_alloc);
//}


































































void alifInterpreterState_enable(AlifRuntimeState* _runtime)
{
	AlifRuntimeState::AlifInterpreters* interpreters = &_runtime->alifInterpreters;
	interpreters->nextID = 0;

	if (interpreters->mutex == nullptr) {
		AlifMemAllocatorEx oldAlloc{};
		alifMem_setDefaultAllocator(AlifMem_Domain_Raw, &oldAlloc);

		interpreters->mutex = alifThread_allocateLock();

		alifMem_setAllocator(AlifMem_Domain_Raw, &oldAlloc);

		if (interpreters->mutex == nullptr) {
			std::cout << ("Can't initialize threads for interpreter") << std::endl;
			exit(-1);
		}
	}


}







static AlifInterpreterState* alloc_interpreter()
{
	return (AlifInterpreterState*)alifMem_rawCalloc(1, sizeof(AlifInterpreterState)); // تم تغيير النوع المرجع بسبب ظهور خطأ
}

static void free_interpreter(AlifInterpreterState* interp)
{


	if (interp != &alifRuntime.mainInterpreter) {
		alifMem_rawFree(interp);
	}
}




















static void init_interpreter(
	AlifInterpreterState* _interp, AlifRuntimeState* _runtime,
	int64_t _id,
	AlifInterpreterState* _next,
	AlifThreadTypeLock _pendingLock)
{

	if (_interp->initialized) {
		std::cout << ("interpreter already initialized") << std::endl; exit(-1);
	}

	//assert(_runtime != nullptr);
	_interp->runtime = _runtime;

	//assert(_id > 0 || (_id == 0 && _interp == _runtime->alifInterpreters.main));
	_interp->iD = _id;

	//assert(_runtime->alifInterpreters.head == _interp);
	//assert(_next != nullptr || (_interp == _runtime->alifInterpreters.main));
	_interp->next = _next;



	if (_interp != &_runtime->mainInterpreter) {
		PoolP temp[OBMALLOC_USEDPOOLS_SIZE] = ALIFMEMORY_ALIGNMENTS_INIT(_interp->obmalloc.pools);
		memcpy(&_interp->obmalloc.pools.used, temp, sizeof(temp));
	}
	//alifObject_initState(_interp);





	alifEval_initState(_interp, _pendingLock);
	alifGC_initState(&_interp->gc);
	alifConfig_initAlifConfig(&_interp->config);
	alifType_initCache(_interp);
	for (int i = 0; i < 15; i++) {
		_interp->monitors.tools[i] = 0;
	}
	for (int t = 0; t < ALIFMONITORING_TOOL_IDS; t++) {
		for (int e = 0; e < ALIFMONITORING_EVENTS; e++) {
			_interp->monitoringCallables[t][e] = nullptr;

		}
	}
	_interp->sysProfileInitialized = false;
	_interp->sysTraceInitialized = false;
	//_interp->optimizer = &alifOptimizerDefault;
	//_interp->optimizerBackedgeThreshold = alifOptimizerDefault.backedgeThreshold;
	//_interp->optimizerResumeThreshold = alifOptimizerDefault.backedgeThreshold;
	if (_interp != &_runtime->mainInterpreter) {
		//_interp->dtoa = (DtoaState)DTOA_STATE_INIT(_interp);
	}
	_interp->fOpcodeTraceSet = false;
	_interp->initialized = 1;

}




void alifInterpreterState_new(AlifThreadState* _tState, AlifInterpreterState** _pInterp)
{

	*_pInterp = nullptr;

	AlifRuntimeState* runtime = &alifRuntime;




	if (_tState != nullptr) {
		//if (AlifSys_audit(_tState, "alif.AlifInterpreterStateNew", nullptr) < 0) {
		//	std::cout << "sys.audit failed" << std::endl; exit(-1);
		//}
	}

	AlifThreadTypeLock pendingLock = alifThread_allocateLock();








	HEAD_LOCK(runtime);

	class AlifRuntimeState::AlifInterpreters* interpreters = &runtime->alifInterpreters;
	int64_t iD = interpreters->nextID;
	interpreters->nextID += 1;


	AlifInterpreterState* interp;

	AlifInterpreterState* oldHead = interpreters->head;
	if (oldHead == nullptr) {

		//assert(interpreters->main == nullptr);
		//assert(iD == 0);

		interp = &runtime->mainInterpreter;
		//assert(interp->iD == 0);
		//assert(interp->next == nullptr);

		interpreters->main = interp;
	}
	else {
		//assert(interpreters->main != nullptr);
		//assert(iD != 0);

		interp = alloc_interpreter();
		if (interp == nullptr) {

			goto error;
		}

		memcpy(interp, &initial.mainInterpreter, sizeof(*interp));

		if (iD < 0) {

			std::cout << "failed to get an interpreter ID" << std::endl;
			goto error;
		}
	}
	interpreters->head = interp;

	init_interpreter(interp, runtime,
		iD, oldHead, pendingLock);



	pendingLock = nullptr;

	HEAD_UNLOCK(runtime);

	//assert(interp != nullptr);
	*_pInterp = interp;


error:
	HEAD_UNLOCK(runtime);

	if (pendingLock != nullptr) {
		alifThread_freeLock(pendingLock);
	}
	if (interp != nullptr) {
		free_interpreter(interp);
	}
}

















































































































































































static inline void tState_deactivate(AlifThreadState*);
























































































































































































































































































































static AlifThreadState* alloc_threadState(void)
{
	return (AlifThreadState*)alifMem_rawCalloc(1, sizeof(AlifThreadState));
}




















static void init_threadState(AlifThreadState* _TState,
	AlifInterpreterState* _interp, uint64_t _iD)
{
	if (_TState->status.initialized) {
		std::cout << ("thread state already initialized") << std::endl; exit(-1);
	}

	//assert(_interp != nullptr);
	_TState->interp = _interp;


	//assert(_TState->next == nullptr);
	//assert(_TState->prev == nullptr);

	//assert(_iD > 0);
	_TState->iD = _iD;



	_TState->alifRecursionLimit = _interp->cEval.recursionLimit,
	_TState->alifRecursionRemaining = _interp->cEval.recursionLimit,
	_TState->cRecursionRemaining = ALIFC_RECURSION_LIMIT;

	_TState->excInfo = &_TState->excState;



	_TState->gilStateCounter = 1;

	//_TState->currentFrame = nullptr;
	//_TState->datastackChunk = nullptr;
	//_TState->datastackTop = nullptr;
	//_TState->datastackLimit = nullptr;
	_TState->whatEvent = -1;

	_TState->status.initialized = 1;
}


static void add_threadState(AlifInterpreterState* _interp, AlifThreadState* _tState,
	AlifThreadState* _next)
{
	//assert(_interp->thread.head != _tState);
	//assert((_next != nullptr && _tState->id != 1) ||
		//(_next == nullptr && _tState->id == 1));
	if (_next != nullptr) {
		//assert(_next->prev == nullptr || _next->prev == _tState);
		_next->prev = _tState;
	}
	_tState->next = _next;
	//assert(_tState->prev == nullptr);
	_interp->thread.head = _tState;
}


static AlifThreadState* new_threadState(AlifInterpreterState* _interp)
{
	AlifThreadState* tState;
	AlifRuntimeState* runtime = _interp->runtime;




	AlifThreadState* newTState = alloc_threadState();
	int usedNewTState;
	if (newTState == nullptr) {
		return nullptr;
	}

	HEAD_LOCK(runtime);

	_interp->thread.nextUniqueID += 1;
	uint64_t iD = _interp->thread.nextUniqueID;


	AlifThreadState* oldHead = _interp->thread.head;
	if (oldHead == nullptr) {

		//assert(iD == 1);
		usedNewTState = 0;
		tState = &_interp->initialThread;
	}
	else {

		//assert(iD > 1);
		//assert(old_head->prev == nullptr);
		usedNewTState = 1;
		tState = newTState;

		memcpy(tState,
			&initial.mainInterpreter.initialThread,
			sizeof(*tState));
	}

	init_threadState(tState, _interp, iD);
	add_threadState(_interp, tState, oldHead);

	HEAD_UNLOCK(runtime);
	if (!usedNewTState) {

		alifMem_rawFree(newTState);
	}
	return tState;
}


















AlifThreadState* alifThreadState_new(AlifInterpreterState* _interp)
{
	return new_threadState(_interp);
}















































































































































































































































































































static inline void tState_activate(AlifThreadState* _tState)
{
	//assert(tstate != NULL);

	//assert(tstate_is_bound(tstate));
	//assert(!tstate->_status.active);

	//assert(!tstate->_status.bound_gilstate ||
		//tstate == gilstate_tss_get((tstate->interp->runtime)));
	if (!_tState->status.boundGilState) {
		bind_gilState_tState(_tState);
	}

	_tState->status.active = 1;
}


static inline void tState_deactivate(AlifThreadState* _tState)
{
	//assert(_tState != nullptr);

	//assert(tstate_isBound(tstate));
	//assert(tstate->status.active);

	_tState->status.active = 0;



}











































































static void swap_threadStates(AlifRuntimeState* _runtime,
	AlifThreadState* _oldTs, AlifThreadState* _newts)
{

	current_fastClear(_runtime);

	if (_oldTs != nullptr) {

		tState_deactivate(_oldTs);
	}

	if (_newts != nullptr) {


		current_fastSet(_runtime, _newts);
		tState_activate(_newts);
	}
}

AlifThreadState* alifThreadState_swapNoGIL(AlifThreadState* _newts)
{
#if defined(ALIF_DEBUG)



	int err = errno;
#endif

	AlifThreadState* oldts = current_fastGet(&alifRuntime);
	swap_threadStates(&alifRuntime, oldts, _newts);

#if defined(ALIF_DEBUG)
	errno = err;
#endif
	return oldts;
}




















void alifThreadState_bind(AlifThreadState* _tState)
{


	//assert(alifThreadState_checkConsistency(_tState));

	bind_tState(_tState);


	if (GILSTATE_TSS_GET(_tState->interp->runtime) == nullptr) {
		bind_gilState_tState(_tState);
	}
}










AlifInterpreterState* alifInterpreterState_head()
{
	return alifRuntime.alifInterpreters.head;
}

AlifInterpreterState* alifInterpreterState_main()
{
	return alifInterpreter_state_main();
}



AlifInterpreterState* alifInterpreterState_next(AlifInterpreterState* _interp) {
	return _interp->next;
}























































































































































void alifGILState_init(AlifInterpreterState* _interp)
{
	//if (!alif_isMainInterpreter(_interp)) {
	//	return;
	//}
	AlifRuntimeState* runtime = _interp->runtime;
	//assert(GILSTATE_TSS_GET(runtime) == nullptr);
	//assert(runtime->gilstate.autoInterpreterState == nullptr);
	//runtime->gilstate.autoInterpreterState = _interp;

}

















































































































































































































































































































































































































































































































































































































































































































const AlifConfig* alifInterpreterState_getConfig(AlifInterpreterState* _interp)
{
	return &_interp->config;
}
