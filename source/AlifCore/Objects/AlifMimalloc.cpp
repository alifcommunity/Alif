#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"


/*
سيتم عمل الاصدار الجديد من الذاكرة يستعمل خيوط متعددة لتسريع عمليات الذاكرة
*/




static bool alifMemMiPage_maybeFree(mi_page_t*, mi_page_queue_t*, bool); // 18



#include "AlifCore_Mimalloc.h"
#include "mimalloc/static.c"
#include "mimalloc/mimalloc/internal.h"  // for stats







static bool alifMemMiPage_maybeFree(mi_page_t* page, mi_page_queue_t* pq, bool force) { // 126
#ifdef ALIF_GIL_DISABLED
	if (page->use_qsbr) {
		AlifThreadImpl* tstate = (AlifThreadImpl*)alifThread_get();
		if (page->qsbr_goal != 0 and alifQSBR_goalReached(tstate->qsbr, page->qsbr_goal)) {
			alifMemMiPage_clearQSBR(page);
			_mi_page_free(page, pq, force);
			return true;
		}

		alifMemMiPage_clearQSBR(page);
		page->retire_expire = 0;
		page->qsbr_goal = alifQSBR_deferredAdvance(tstate->qsbr);
		llist_insertTail(&tstate->mimalloc.pageList, &page->qsbr_node);
		return false;
	}
#endif
	_mi_page_free(page, pq, force);
	return true;
}




#define WORK_ITEMS_PER_CHUNK 254 // 1077

class MemWorkItem { // 1080
public:
	uintptr_t ptr{};
	uint64_t qsbrGoal{};
};

class MemWorkChunk { // 1086
public:
	class LListNode node {};

	AlifSizeT rdIDx{};
	AlifSizeT wrIDx{};
	class MemWorkItem array[WORK_ITEMS_PER_CHUNK]{};
};

static void free_workItem(AlifUSizeT _ptr) {  // 1096
	if (_ptr & 0x01) {
		alifMem_objFree((char*)(_ptr - 1));
	}
	else {
		alifMem_dataFree((void*)_ptr);
	}
}

static void free_delayed(uintptr_t _ptr) { // 1107
	AlifInterpreter* interp = alifInterpreter_get();
	if (alifInterpreter_getFinalizing(interp) != nullptr or
		interp->stopTheWorld.worldStopped)
	{

		free_workItem(_ptr);
		return;
	}

	AlifThreadImpl* tstate = (AlifThreadImpl*)alifThread_get();
	LListNode* head = &tstate->memFreeQueue;

	class MemWorkChunk* buf = nullptr;
	if (!llist_empty(head)) {
		buf = LLIST_DATA(head->prev, MemWorkChunk, node);
		if (buf->wrIDx == WORK_ITEMS_PER_CHUNK) {
			buf = nullptr;
		}
	}

	if (buf == nullptr) {
		buf = (MemWorkChunk*)alifMem_dataAlloc(1 * sizeof(*buf));
		if (buf != nullptr) {
			llist_insertTail(head, &buf->node);
		}
	}

	if (buf == nullptr) {
		alifEval_stopTheWorld(tstate->base.interpreter);
		free_workItem(_ptr);
		alifEval_startTheWorld(tstate->base.interpreter);
		return;
	}

	uint64_t seq = alifQSBR_deferredAdvance(tstate->qsbr);
	buf->array[buf->wrIDx].ptr = _ptr;
	buf->array[buf->wrIDx].qsbrGoal = seq;
	buf->wrIDx++;

	if (buf->wrIDx == WORK_ITEMS_PER_CHUNK) {
		alifMem_processDelayed((AlifThread*)tstate);
	}
}

void alifMem_freeDelayed(void* _ptr) { // 1163
	free_delayed((AlifUSizeT)_ptr);

}

static class MemWorkChunk* work_queueFirst(class LListNode* _head) {  // 1177
	return LLIST_DATA(_head->next, class MemWorkChunk, node);
}

static void process_queue(class LListNode* _head, class QSBRThreadState* _qsbr,
	bool _keepEmpty) { // 1183
	while (!llist_empty(_head)) {
		class MemWorkChunk* buf = work_queueFirst(_head);

		while (buf->rdIDx < buf->wrIDx) {
			class MemWorkItem* item = &buf->array[buf->rdIDx];
			if (!alifQSBR_poll(_qsbr, item->qsbrGoal)) {
				return;
			}

			free_workItem(item->ptr);
			buf->rdIDx++;
		}

		if (_keepEmpty and buf->node.next == _head) {
			buf->rdIDx = buf->wrIDx = 0;
			return;
		}

		llist_remove(&buf->node);
		alifMem_dataFree(buf);
	}
}

static void process_interpQueue(class AlifMemInterpFreeQueue* _queue,
	class QSBRThreadState* _qsbr) {  // 1212
	if (!alifAtomic_loadIntRelaxed(&_queue->hasWork)) {
		return;
	}

	if (alifMutex_lockTimed(&_queue->mutex, 0, (AlifLockFlags_)0) == Alif_Lock_Acquired) {
		process_queue(&_queue->head, _qsbr, false);

		AlifIntT moreWork = !llist_empty(&_queue->head);
		alifAtomic_storeIntRelaxed(&_queue->hasWork, moreWork);

		alifMutex_unlock(&_queue->mutex);
	}
}

void alifMem_processDelayed(AlifThread* _thread) {  // 1231

	AlifInterpreter* interp = _thread->interpreter;
	AlifThreadImpl* tStateImpl = (AlifThreadImpl*)_thread;

	process_queue(&tStateImpl->memFreeQueue, tStateImpl->qsbr, true);

	process_interpQueue(&interp->memFreeQueue, tStateImpl->qsbr);
}
