#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_State.h"
#include "AlifCore_ObjectStack.h"


extern AlifObjectStackChunk* alifObjectStackChunk_new();

AlifObjectStackChunk* alifObjectStackChunk_new() { // 11
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
