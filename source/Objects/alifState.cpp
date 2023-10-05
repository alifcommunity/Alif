


#include "alif.h"
//#include "alifCore_ceval.h"
#include "alifCore_code.h"          
//#include "alifCore_dtoa.h"         
//#include "alifCore_frame.h"
#include "alifCore_initconfig.h"
//#include "alifCore_object.h"       
//#include "alifEore_pyerrors.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifMem.h"      
#include "alifCore_alifState.h"
#include "alifCore_runtimeInit.h"
//#include "alifCore_sysModule.h"    
//#include "alifCore_weakref.h"    















































#ifdef HAVE_THREAD_LOCAL
ALIF_THREAD_LOCAL AlifThreadState* alifTssTState = nullptr;
#endif

static inline AlifThreadState* current_fastGet(AlifRuntimeState* ALIF_UNUSED(runtime))
{
#ifdef HAVE_THREAD_LOCAL
	return alifTssTState;
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

























































#define GILSTATE_TSS_INITIALIZED(_runtime) \
    tState_tssInitialized(&(_runtime)->autoTSSKey)
#define GILSTATE_TSS_INIT(_runtime) \
    tState_tssInit(&(_runtime)->autoTSSKey)
#define GILSTATE_TSS_FINI(_runtime) \
    tState_tssFini(&(_runtime)->autoTSSKey)





















































































































































































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
	if (_runtime->initialized) {
		// runtime already initialized
	}





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
	if (alloc_forRuntime(locks) != 0) exit(-1);



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































static void init_interpreter(AlifInterpreterState* _interp, AlifRuntimeState* _runtime, int64_t _id, AlifInterpreterState* _next, AlifThreadTypeLock _pendingLock)
{
	//if (_interp->initialized) {
		//alif_fatalError("interpreter already initialized");
	//}

	//assert(_runtime != nullptr);
	//_interp->runtime = _runtime;

	//assert(_id > 0 || (_id == 0 && _interp == _runtime->alifInterpreters.main));
	//_interp->id = _id;

	//assert(_runtime->alifInterpreters.head == _interp);
	//assert(_next != nullptr || (_interp == _runtime->alifInterpreters.main));
	//_interp->next = _next;

	//if (_interp != &_runtime->mainInterpreter) {
		//PoolP temp[OBMALLOC_USEDPOOLS_SIZE] = OBMALLOC_POOLS_INIT(_interp->obmalloc.pools);
		//memcpy(&_interp->obmalloc.pools.used, temp, sizeof(temp));
	//}
	//alifObject_initState(_interp);

	//alifEval_initState(_interp, _pendingLock);
	//alifGC_initState(&_interp->gc);
	//alifConfig_initAlifConfig(&_interp->config);
	//alifType_initCache(_interp);
	//for (int i = 0; i < ALIF_MONITORING_UNGROUPED_EVENTS; i++) {
		//_interp->monitors.tools[i] = 0;
	//}
	//for (int t = 0; t < ALIF_MONITORING_TOOL_IDS; t++) {
	//	for (int e = 0; e < ALIF_MONITORING_EVENTS; e++) {
	//		_interp->monitoringCallables[t][e] = nullptr;

	//	}
	//}
	//_interp->sysProfileInitialized = false;
	//_interp->sysTraceInitialized = false;
	//_interp->optimizer = &alifOptimizerDefault;
	//_interp->optimizerBackedgeThreshold = alifOptimizerDefault.backedgeThreshold;
	//_interp->optimizerResumeThreshold = alifOptimizerDefault.backedgeThreshold;
	//if (_interp != &_runtime->mainInterpreter) {
	//	_interp->dtoa = (DtoaState)DTOA_STATE_INIT(_interp);
	//}
	//_interp->fOpcodeTraceSet = false;
	//_interp->initialized = 1;
}







AlifInterpreterState* alifInterpreterState_new()
{
	AlifInterpreterState* interp{};
	AlifRuntimeState* runtime = &alifRuntime;
	AlifThreadState* tState = current_fastGet(runtime);

	//if (alifSys_audit(tState, "alifcpp.alifInterpreterState_new", nullptr) < 0) {
	//	return nullptr;
	//}

	AlifThreadTypeLock pendingLock = alifThread_allocateLock();
	if (pendingLock == nullptr) {
		if (tState != nullptr) {
			//alifErr_noMemory(tState);
		}
		return nullptr;
	}

	AlifRuntimeState::AlifInterpreters* interpreters = &runtime->alifInterpreters;

	HEAD_LOCK(runtime);

	int64_t id = interpreters->nextID;
	interpreters->nextID += 1;

	AlifInterpreterState* oldHead = interpreters->head;
	if (oldHead == nullptr) {
		//assert(interpreters->main == nullptr);
		//assert(id == 0);

		interp = &runtime->mainInterpreter;
		//assert(_interp->id == 0);
		//assert(_interp->next == nullptr);

		interpreters->main = interp;
	}
	else {
		//assert(interpreters->main != nullptr);
		//assert(id != 0);

		interp = alloc_interpreter();
		if (interp == nullptr) {
			goto error;
		}
		memcpy(interp, &initial.mainInterpreter,
			sizeof(*interp));

		if (id < 0) {
			if (tState != nullptr) {
				//alifErr_setString(tState, alifExcRuntimeError,
				//	"failed to get an interpreter ID");
			}
			goto error;
		}
	}
	interpreters->head = interp;

	init_interpreter(interp, runtime, id, oldHead, pendingLock);

	HEAD_UNLOCK(runtime);
	return interp;

error:
	HEAD_UNLOCK(runtime);

	alifThread_freeLock(pendingLock);
	if (interp != nullptr) {
		//free_interpreter(_interp);
	}
	return nullptr;
}











































































































































































































































































































































































































































































































































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

	//assert(_id > 0);
	_TState->iD = _iD;



	//_TState->alifRecursionLimit = _interp->ceval.recursionLimit,
	//	_TState->alifRecursionRemaining = _interp->ceval.recursionLimit,
	//	_TState->cRecursionRemaining = alif_C_RECURSION_LIMIT;

	//_TState->excInfo = &_TState->excState;



	_TState->gilstateCounter = 1;

	//_TState->currentFrame = nullptr;
	//_TState->datastackChunk = nullptr;
	//_TState->datastackTop = nullptr;
	//_TState->datastackLimit = nullptr;
	_TState->whatEvent = -1;

	_TState->status.initialized = 1;
}


















static AlifThreadState* new_threadState(AlifInterpreterState* _interp)
{
	AlifThreadState* TState;
	AlifRuntimeState* runtime = _interp->runtime;




	AlifThreadState* newTState = alloc_threadState();
	int usedNewTState;
	if (newTState == nullptr) {
		return nullptr;
	}

	HEAD_LOCK(runtime);

	_interp->thread.nextUniqueID += 1;
	uint64_t id = _interp->thread.nextUniqueID;


	AlifThreadState* oldHead = _interp->thread.head;
	if (oldHead == nullptr) {

		//assert(id == 1);
		usedNewTState = 0;
		TState = &_interp->initialThread;
	}
	else {

		//assert(id > 1);
		//assert(old_head->prev == nullptr);
		usedNewTState = 1;
		TState = newTState;

		memcpy(TState,
			&initial.mainInterpreter.initialThread,
			sizeof(*TState));
	}

	init_threadState(TState, _interp, id);
	//add_threadState(_interp, TState, oldHead);

	//HEAD_UNLOCK(runtime);
	if (!usedNewTState) {

		alifMem_rawFree(newTState);
	}
	return TState;
}


















AlifThreadState* alifThreadState_new(AlifInterpreterState* _interp)
{
	return new_threadState(_interp);
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
