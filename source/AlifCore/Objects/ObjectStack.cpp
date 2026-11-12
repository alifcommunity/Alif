#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_State.h"
#include "AlifCore_ObjectStack.h"


extern AlifObjectStackChunk* _alifObjectStackChunk_new();
extern void _alifObjectStackChunk_free(AlifObjectStackChunk*); 

AlifObjectStackChunk* _alifObjectStackChunk_new() { // 11
	AlifObjectStackChunk* buf = (AlifObjectStackChunk*)ALIF_FREELIST_POP_MEM(objectStackChunks);
	if (buf == nullptr) {
		buf = (AlifObjectStackChunk*)alifMem_objAlloc(sizeof(AlifObjectStackChunk));
		if (buf == nullptr) {
			return nullptr;
		}
	}
	buf->prev = nullptr;
	buf->n_ = 0;
	return buf;
}



void _alifObjectStackChunk_free(AlifObjectStackChunk* _buf) { // 29
	ALIF_FREELIST_FREE(objectStackChunks, OBJECT_STACK_CHUNKS, _buf, alifMem_dataFree);
}


void _alifObjectStack_clear(AlifObjectStack* queue) { // 36
	while (queue->head != nullptr) {
		AlifObjectStackChunk* buf = queue->head;
		buf->n_ = 0;
		queue->head = buf->prev;
		_alifObjectStackChunk_free(buf);
	}
}
