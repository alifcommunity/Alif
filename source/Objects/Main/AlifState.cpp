#include "alif.h"

#include "AlifCore_AlifEval.h"
#include "AlifCore_Code.h"
#include "AlifCore_Frame.h"
#include "AlifCore_InitConfig.h"
#include "AlifObject.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_Memory.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_DureRunInit.h"
#include "AlifCore_LList.h"



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
	alifSubGC_initState(&_interpreter->gc);
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




#define DATA_STACK_CHUNK_SIZE (16*1024)

static AlifStackChunk* allocate_chunk(AlifIntT _sizeInBytes, AlifStackChunk* _previous) { // 1414
	//AlifStackChunk* res = alifObject_virtualAlloc(_sizeInBytes);
	AlifStackChunk* res = (AlifStackChunk*)alifMem_dataAlloc(_sizeInBytes);
	if (res == nullptr) return nullptr;

	res->previous = _previous;
	res->size = _sizeInBytes;
	res->top = 0;
	return res;
}


#define MINIMUM_OVERHEAD 1000

static AlifObject** push_chunk(AlifThread* _thread, AlifIntT _size) { // 2902
	AlifIntT allocateSize = DATA_STACK_CHUNK_SIZE;
	while (allocateSize < (int)sizeof(AlifObject*) * (_size + MINIMUM_OVERHEAD)) {
		allocateSize *= 2;
	}
	AlifStackChunk* newChunck = allocate_chunk(allocateSize, _thread->dataStackChunk);
	if (newChunck == nullptr) {
		return nullptr;
	}
	if (_thread->dataStackChunk) {
		_thread->dataStackChunk->top = _thread->dataStackTop -
			&_thread->dataStackChunk->data[0];
	}
	_thread->dataStackChunk = newChunck;
	_thread->dataStackLimit = (AlifObject**)(((char*)newChunck) + allocateSize);
	AlifObject** res = &newChunck->data[newChunck->previous == nullptr];
	_thread->dataStackTop = res + _size;
	return res;
}

AlifInterpreterFrame* alifThread_pushFrame(AlifThread* _thread, AlifUSizeT _size) { // 2929
	if (alifThread_hasStackSpace(_thread, (int)_size)) {
		AlifInterpreterFrame* res = (AlifInterpreterFrame*)_thread->dataStackTop;
		_thread->dataStackTop += _size;
		return res;
	}
	return (AlifInterpreterFrame*)push_chunk(_thread, (int)_size);
}
