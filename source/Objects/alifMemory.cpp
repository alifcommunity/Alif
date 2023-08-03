#include "alif.h"

/*

	هناك نوعان من الذاكرة الاول المسمى raw والثاني المسممى mem
	raw يستخدم ذاكرة منخفضة المستوى مباشرة من لغة c
	mem يسخدم الذاكرة رفيعة المستوى من مميزاتها
	   - تتبع التخصيصات و الغاء التخصيصات
	   - وايضا يتحقق من الاخطاء
	   - ويدعم تخصيصات الذاكرة المختلفة

*/

/* ______________ flags ______________ */

bool rawOrMem = false;
bool objectOrMemDelete = false; // false for mem , true for object

/* ______________ /flags ______________ */

void* AlifMem_debug_raw_alloc(MemoryState* _state, size_t useCalloc, size_t nByte);
void* AlifMem_debug_raw_realloc(MemoryState* _state, void* ptr, size_t nByte);
void  AlifMem_debug_raw_free(MemoryState* _state, void* ptr);
void* AlifMem_debug_raw_calloc(MemoryState* _state, size_t nElement, size_t elSize);

void* object_malloc(MemoryState* _state, size_t numberByte);
void* object_calloc(MemoryState* _state, size_t nElement, size_t elSize);
void  object_free(MemoryState* _state, void* ptr);
void* object_realloc(MemoryState* _state, void* ptr, size_t numberByte);

int malloc_free(MemoryState* _state, void* ptr);


static MemoryState state;


void
write_size_t(void* p, size_t n)
{
	uint8_t* q = (uint8_t*)p + 8 - 1;
	int i;

	for (i = 8; --i >= 0; --q) {
		*q = (uint8_t)(n & 0xff);
		n >>= 8;
	}
}

size_t
read_size_t(const void* p)
{
	const uint8_t* q = (const uint8_t*)p;
	size_t result = *q++;
	size_t i;

	for (i = 8; --i > 0; ++q)
		result = (result << 8) | *q;
	return result;
}

// raw memory ///////////////////////////////////////////////////////////////////////////////

/***************************************/
/* low-level allocator implementations */
/***************************************/

void* AlifMem_raw_malloc(size_t size) {

	if (size == 0) {
		size = 1;
	}
	return malloc(size);
}

void* AlifMem_raw_calloc(size_t nElement, size_t elSize) {


	if (nElement == 0 || elSize == 0) {
		nElement = 1;
		elSize = 1;
	}

	return calloc(nElement, elSize);

}

void* AlifMem_raw_realloc(void* ptr, size_t size) {

	if (size == 0) {
		size = 1;
	}
	return realloc(ptr, size);
}

void AlifMem_raw_free(void* ptr) {
	free(ptr);
}

void* raw_malloc(size_t size) {

	if (size > 9223372036854775807i64) {
		return NULL;
	}

	return AlifMem_raw_malloc(size);
}

void* raw_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	rawOrMem = false;

	return AlifMem_debug_raw_alloc(_state, 1, nElement * elSize);

}

void* raw_realloc(MemoryState* _state, void* ptr, size_t newSize) {

	rawOrMem = false;

	if (newSize > 9223372036854775807i64) {
		return NULL;
	}

	return AlifMem_debug_raw_realloc(_state, ptr, newSize);

}

void raw_free(MemoryState* _state, void* ptr) {

	objectOrMemDelete = false;

	AlifMem_debug_raw_free(_state, ptr);

}


// mem memory ///////////////////////////////////////////////////////////////////////////////

void* AlifMem_malloc(MemoryState* _state, size_t size) {

	rawOrMem = true;

	return AlifMem_debug_raw_alloc(_state, 0, size);
}

void* AlifMem_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	rawOrMem = true;

	return AlifMem_debug_raw_calloc(_state, nElement, elSize);
}

void* AlifMem_realloc(MemoryState* _state, void* ptr, size_t newSize) {

	rawOrMem = true;

	return AlifMem_debug_raw_realloc(_state, ptr, newSize);
}

void AlifMem_free(MemoryState* _state, void* ptr) {

	objectOrMemDelete = false;

	AlifMem_debug_raw_free(_state, ptr);
}


// object memory ///////////////////////////////////////////////////////////////////////////////

void* AlifObject_malloc(MemoryState* _state, size_t size) {

	rawOrMem = true;

	return AlifMem_debug_raw_alloc(_state, 0, size);

}

void AlifObject_free(MemoryState* _state, void* ptr) {

	if (ptr == NULL) {
		return;
	}

	objectOrMemDelete = true;

	if (!malloc_free(_state, ptr)) {

		AlifMem_debug_raw_free(_state, ptr);

	}

}

void* AlifObject_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	rawOrMem = false;

	return object_calloc(_state, nElement, elSize);

}

void* AlifObject_realloc(MemoryState* _state, void* ptr, size_t numberByte) {

	rawOrMem = false;

	return object_realloc(_state, ptr, numberByte);

}


////////////////////////////////////////////////////////////////////////////////////////


inline BlockMapDown* Block_map_get(MemoryState* _state, uint8_t* ptr, int create) {

	int i1 = (((uint8_t)(ptr)) >> ((((64 - 0) - 20 + 2) / 3) + (((64 - 0) - 20 + 2 + 20))) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1));

	if (_state->usage.BlockMapRoot.ptrs[i1] == NULL) {

		if (!create) {
			return NULL;
		}
		BlockMapMid* mid = (BlockMapMid*)AlifMem_debug_raw_alloc(_state, 1, sizeof(BlockMapMid));

		_state->usage.BlockMapRoot.ptrs[i1] = mid;
		_state->usage.BlockMapMidCount++;

	}
	int i2 = ((((uintptr_t)(ptr)) >> (((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3)) + 20)) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1));

	if (_state->usage.BlockMapRoot.ptrs[i1]->ptrs[i2] == NULL) {

		if (!create) {
			return NULL;
		}
		BlockMapDown* bot = (BlockMapDown*)AlifMem_debug_raw_alloc(_state, 1, sizeof(BlockMapDown));

		_state->usage.BlockMapRoot.ptrs[i1]->ptrs[i2] = bot;
		_state->usage.BlockMapMidCount++;
	}
	return _state->usage.BlockMapRoot.ptrs[i1]->ptrs[i2];

}

void Block_map_mark_used(MemoryState* _state, uintptr_t BlockBase, int isUsed) {


	BlockMapDown* bot = Block_map_get(_state, (uint8_t*)BlockBase, isUsed);

	int i3 = ((((uintptr_t)((uint8_t*)BlockBase)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));
	int32_t tail = (int32_t)(BlockBase & ((1 << 20) - 1));
	if (tail == 0) {
		bot->Block[i3].tailHi = isUsed ? -1 : 0;
	}
	else {
		bot->Block[i3].tailHi = isUsed ? tail : 0;
		uintptr_t BlockBaseNext = BlockBase + (1 << 20);
		BlockMapDown* botLo = Block_map_get(_state, (uint8_t*)BlockBaseNext, isUsed);
		if (botLo == NULL) {
			bot->Block[i3].tailHi = 0;
			return;
		}
		int i3Next = ((((uintptr_t)(BlockBaseNext)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));
		botLo->Block[i3Next].tailLo = isUsed ? tail : 0;
	}

}

int Block_map_is_used(MemoryState* _state, uint8_t* ptr) {

	BlockMapDown* mid = Block_map_get(_state, ptr, 0);

	int i3 = ((((uintptr_t)((uint8_t*)ptr)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));

	int32_t hi = mid->Block[i3].tailHi;
	int32_t lo = mid->Block[i3].tailLo;
	int32_t tail = (int32_t)((uintptr_t)(ptr) & ((1 << 20) - 1));
	return (tail < lo) || (tail >= hi && hi != 0);

}

void insert_to_used_Alignment(MemoryState* _state, AlignmentHeader* Alignment) {

	unsigned int size = Alignment->sizeIndex;
	AlignmentHeader* next = _state->alignments.used[size * 2];
	AlignmentHeader* prev = next->prevAlignment;

	Alignment->nextAlignment = next;
	Alignment->prevAlignment = prev;
	next->prevAlignment = Alignment;
	prev->nextAlignment = Alignment;

}

void insert_to_free_Alignment(MemoryState* _state, AlignmentHeader* Alignment) {

	AlignmentHeader* next = Alignment->nextAlignment;
	AlignmentHeader* prev = Alignment->prevAlignment;
	next->prevAlignment = prev;
	prev->nextAlignment = next;

	BlockObject* BlockObj = _state->mGmt.Blocks;

	Alignment->nextAlignment = BlockObj->freeAlignments;
	BlockObj->freeAlignments = Alignment;
	unsigned int numberFree = BlockObj->numberFreeAlignments;

	BlockObject* lastNumberFree = state.mGmt.numberfreeAlignment[numberFree];
	if (lastNumberFree == BlockObj) {
		BlockObject* Block = BlockObj->prevBlock;
		state.mGmt.numberfreeAlignment[numberFree] = (Block != NULL && Block->numberFreeAlignments == numberFree) ? Block : NULL;
	}
	BlockObj->numberFreeAlignments = ++numberFree;

	if (numberFree == BlockObj->numberTotalAlignments && BlockObj->nextBlock != NULL) {
		if (BlockObj->prevBlock == NULL) {
			_state->mGmt.usableBlock = BlockObj->nextBlock;
		}
		else {
			BlockObj->prevBlock->nextBlock = BlockObj->nextBlock;
		}

		if (BlockObj->nextBlock != NULL) {
			BlockObj->nextBlock->prevBlock = BlockObj->prevBlock;
		}

		BlockObj->nextBlock = _state->mGmt.unusedBlock;
		_state->mGmt.unusedBlock = BlockObj;


		Block_map_mark_used(_state, BlockObj->address, 0);

		BlockObj->address = 0;
		--_state->mGmt.numberBlockCurrentlyAllocate;

		return;
	}

	if (numberFree == 1) {
		BlockObj->nextBlock = _state->mGmt.usableBlock;
		BlockObj->prevBlock = NULL;
		if (_state->mGmt.usableBlock) {
			_state->mGmt.usableBlock->prevBlock = BlockObj;
		}
		_state->mGmt.usableBlock = BlockObj;
		if (_state->mGmt.numberfreeAlignment[1] == NULL) {
			_state->mGmt.numberfreeAlignment[1] = BlockObj;
		}
		return;
	}


	if (_state->mGmt.numberfreeAlignment[numberFree] == NULL) {
		_state->mGmt.numberfreeAlignment[numberFree] = BlockObj;
	}

	if (BlockObj == lastNumberFree) {
		return;
	}

	if (BlockObj->prevBlock != NULL) {
		BlockObj->prevBlock->nextBlock = BlockObj->nextBlock;
	}
	else {
		_state->mGmt.usableBlock = BlockObj->nextBlock;
	}

	BlockObj->nextBlock->prevBlock = BlockObj->prevBlock;

	BlockObj->prevBlock = lastNumberFree;
	BlockObj->nextBlock = lastNumberFree->nextBlock;
	if (BlockObj->nextBlock != NULL) {
		BlockObj->nextBlock->prevBlock = BlockObj;
	}
	lastNumberFree->nextBlock = BlockObj;

}

bool address_in_range(MemoryState* _state, void* ptr) {
	return Block_map_is_used(_state, (uint8_t*)ptr);
}

int malloc_free(MemoryState* _state, void* ptr) {

	AlignmentHeader* Alignment = ((AlignmentHeader*)((void*)((uintptr_t)((ptr)) & ~(uintptr_t)(((1 << 14)) - 1))));

	if (!address_in_range(_state, ptr)) {
		return 0;
	}

	uint8_t* lastFree = Alignment->freeBlock;
	*(uint8_t**)ptr = lastFree;
	Alignment->freeBlock = (uint8_t*)ptr;
	Alignment->ref.count--;

	if (lastFree == NULL) {
		insert_to_used_Alignment(_state, Alignment);
		return 1;
	}

	if (Alignment->ref.count != 0) {
		return 1;
	}

	insert_to_free_Alignment(_state, Alignment);
	return 1;

}

void object_free(MemoryState* _state, void* ptr) {



	if (ptr == NULL) {
		return;
	}

	if (!malloc_free(_state, ptr)) {
		AlifMem_debug_raw_free(_state, ptr);
		_state->mGmt.rawAllocatedBlocks--;
	}

}

void* AlifMem_debug_raw_alloc(MemoryState* _state, size_t useCalloc, size_t nByte) {

	uint8_t* p;
	uint8_t* data;
	uint8_t* tail;
	size_t total;

	total = nByte + 24;

	if (useCalloc) {
		if (rawOrMem) {
			p = (uint8_t*)object_calloc(_state, 1, total);
		}
		else {
			p = (uint8_t*)AlifMem_raw_calloc(1, total);
		}

	}
	else {

		if (rawOrMem) {
			p = (uint8_t*)object_malloc(_state, total);
		}
		else {
			p = (uint8_t*)AlifMem_raw_malloc(total);
		}

	}
	if (p == NULL) {
		return NULL;
	}
	data = p + 2 * 8;

	write_size_t(p, nByte);

	memset(p + 8 + 1, 0xFD, 8 - 1);

	if (nByte > 0 && !useCalloc) {
		memset(data, 0xCD, nByte);
	}

	tail = data + nByte;
	memset(tail, 0xFD, 8);

	return data;
}

void AlifMem_debug_raw_free(MemoryState* _state, void* ptr) {

	if (ptr == NULL) {
		return;
	}

	uint8_t* q = (uint8_t*)ptr - 2 * 8;
	size_t numberByte;
	numberByte = read_size_t(q);
	numberByte += 24;
	memset(q, 0xDD, numberByte);
	if (objectOrMemDelete) {
		object_free(_state, q);
	}
	else {
		AlifMem_raw_free(q);
	}
}

void* AlifMem_debug_raw_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	size_t numberByte = nElement * elSize;

	return AlifMem_debug_raw_alloc(_state, 1, numberByte);

}

void* AlifMem_debug_raw_realloc(MemoryState* _state, void* ptr, size_t nByte) {

	if (ptr == NULL) {
		return AlifMem_debug_raw_alloc(_state, 0, nByte);
	}

	uint8_t* head;
	uint8_t* data;
	uint8_t* r;
	uint8_t* tail;
	size_t total;
	size_t originalNByte;
	uint8_t save[2 * 64];

	data = (uint8_t*)ptr;
	head = data - 2 * 8;
	originalNByte = read_size_t(head);
	total = nByte + 24;

	tail = data + originalNByte;
	if (originalNByte <= sizeof(save)) {
		memcpy(save, data, originalNByte);
		memset(data - 2 * 8, 0xDD, originalNByte + 24);
	}
	else {
		memcpy(save, data, 64);
		memset(head, 0xDD, 64 + 2 * 8);
		memcpy(&save[2 * 64], tail - 2 * 64, 2 * 64);
		memset(tail - 64, 0xDD, 64 + 24 - 2 * 8);
	}
	r = (uint8_t*)AlifMem_raw_realloc(head, total);

	if (r == NULL) {
		nByte = originalNByte;
	}
	else {
		head = r;
	}

	data = head + 2 * 8;

	write_size_t(head, nByte);
	memset(head + 8 + 1, 0xFD, 8 - 1);

	if (originalNByte <= sizeof(save)) {
		memcpy(data, save, nByte > originalNByte ? originalNByte : nByte);
	}
	else {
		size_t i = originalNByte - 64;
		memcpy(data, save, nByte > 64 ? 64 : nByte);
		if (nByte > i) {
			memcpy(data + i, &save[64], nByte - i > 64 ? 64 : nByte);
		}
	}

	if (r == NULL) {
		return NULL;
	}
	if (nByte > originalNByte) {
		memset(data + originalNByte, 0xCD, nByte - originalNByte);
	}

	return data;

}

BlockObject* new_Block(MemoryState* _state) {


	BlockObject* BlockObj = NULL;
	unsigned int excess;
	void* address = nullptr;

	if ((_state)->mGmt.unusedBlock == NULL) {

		unsigned int i;
		unsigned int numberBlock;
		size_t nByte;

		numberBlock = (_state)->mGmt.maxBlock ? (_state)->mGmt.maxBlock << 1 : 16;

		nByte = numberBlock * sizeof(*(_state)->mGmt.Blocks);
		BlockObj = (BlockObject*)raw_realloc(_state, _state->mGmt.Blocks, nByte);
		(_state)->mGmt.Blocks = BlockObj;

		for (i = (_state)->mGmt.maxBlock; i < numberBlock; ++i)
		{
			(_state)->mGmt.Blocks[i].address = 0;
			(_state)->mGmt.Blocks[i].nextBlock = i < numberBlock - 1 ? &(_state)->mGmt.Blocks[i + 1] : NULL;

		}

		(_state)->mGmt.unusedBlock = &(_state)->mGmt.Blocks[(_state)->mGmt.maxBlock];
		(_state)->mGmt.maxBlock = numberBlock;
	}

	BlockObj = _state->mGmt.unusedBlock;
	address = (void*)VirtualAlloc(NULL, (1 << 20), 0x00001000 | 0x00002000, 0x04);

	if (address != nullptr) {
		Block_map_mark_used(_state, (uintptr_t)address, 1);
	}

	if (address == NULL) {
		BlockObj->nextBlock = (_state)->mGmt.unusedBlock;
		(_state)->mGmt.unusedBlock = BlockObj;
		return NULL;
	}

	BlockObj->address = (uintptr_t)address;

	++(_state)->mGmt.numberBlockCurrentlyAllocate;
	++(_state)->mGmt.numberBlockAllocated;
	if ((_state)->mGmt.numberBlockCurrentlyAllocate > (_state)->mGmt.numberBlockHighWater) {
		(_state)->mGmt.numberBlockHighWater = (_state)->mGmt.numberBlockCurrentlyAllocate;
	}
	BlockObj->freeAlignments = NULL;


	BlockObj->alignmentAddress = (uint8_t*)BlockObj->address;
	BlockObj->numberFreeAlignments = ((1 << 20) / (1 << 14));
	excess = (unsigned int)(BlockObj->address & ((1 << 14) - 1));
	if (excess != 0) {
		--BlockObj->numberFreeAlignments;
		BlockObj->alignmentAddress += ((1 << 14) - excess);
	}
	BlockObj->numberTotalAlignments = BlockObj->numberFreeAlignments;
	return BlockObj;

}

void malloc_Alignment_extend(AlignmentHeader* Alignment, unsigned int size) {

	if (Alignment->nextOffset <= Alignment->maxNextOffset) {

		Alignment->freeBlock = (uint8_t*)Alignment + Alignment->nextOffset;
		Alignment->nextOffset += (((unsigned int)(size)+1) << 4);
		*(uint8_t**)(Alignment->freeBlock) = NULL;
		return;
	}

	AlignmentHeader* next;
	next = Alignment->nextAlignment;
	Alignment = Alignment->prevAlignment;
	next->prevAlignment = Alignment;
	Alignment->nextAlignment = next;

}

void* allocate_from_new_Alignment(MemoryState* _state, unsigned int size) {

	if ((_state)->mGmt.usableBlock == NULL) {

		(_state)->mGmt.usableBlock = new_Block(_state);
		if ((_state)->mGmt.usableBlock == NULL) {
			return NULL;
		}
		(_state)->mGmt.usableBlock->nextBlock = (_state)->mGmt.usableBlock->prevBlock = NULL;
		(_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments] = (_state)->mGmt.usableBlock;
	}
	if ((_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments] == (_state)->mGmt.usableBlock) {
		(_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments] = NULL;
	}
	if ((_state)->mGmt.usableBlock->numberFreeAlignments > 1) {
		(_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments - 1] = (_state)->mGmt.usableBlock;
	}

	AlignmentHeader* Alignment = (_state)->mGmt.usableBlock->freeAlignments;
	if (Alignment != NULL) {
		(_state)->mGmt.usableBlock->freeAlignments = Alignment->nextAlignment;
		(_state)->mGmt.usableBlock->numberFreeAlignments--;
		if ((_state)->mGmt.usableBlock->numberFreeAlignments == 0) {
			(_state)->mGmt.usableBlock = (_state)->mGmt.usableBlock->nextBlock;
			if ((_state)->mGmt.usableBlock != NULL) {
				(_state)->mGmt.usableBlock->prevBlock = NULL;
			}
		}
	}
	else {
		Alignment = (AlignmentHeader*)_state->mGmt.usableBlock->alignmentAddress;
		Alignment->BlockIndex = (unsigned int)(_state->mGmt.usableBlock - _state->mGmt.Blocks);
		Alignment->sizeIndex = 0xffff;
		(_state)->mGmt.usableBlock->alignmentAddress += (1 << 14);
		--(_state)->mGmt.usableBlock->numberFreeAlignments;

		if ((_state)->mGmt.usableBlock->numberFreeAlignments == 0) {
			(_state)->mGmt.usableBlock = (_state)->mGmt.usableBlock->nextBlock;
			if ((_state)->mGmt.usableBlock != NULL) {
				(_state)->mGmt.usableBlock->prevBlock = NULL;
			}
		}
	}

	uint8_t* bp;
	AlignmentHeader* next = (_state)->alignments.used[size + size];
	Alignment->nextAlignment = next;
	Alignment->prevAlignment = next;
	next->nextAlignment = Alignment;
	next->prevAlignment = Alignment;
	Alignment->ref.count = 1;
	if (Alignment->sizeIndex == size) {
		bp = Alignment->freeBlock;
		Alignment->freeBlock = *(uint8_t**)bp;
		return bp;
	}

	Alignment->sizeIndex = size;
	size = (((unsigned int)(size)+1) << 4);
	bp = (uint8_t*)Alignment + SIZE_ROUND_UP(sizeof(AlignmentHeader), 16));
	Alignment->nextOffset = SIZE_ROUND_UP(sizeof(AlignmentHeader), 16)) + (size << 1);
	Alignment->maxNextOffset = (1 << 14) - size;
	Alignment->freeBlock = bp + size;
	*(uint8_t**)(Alignment->freeBlock) = NULL;
	return bp;

}

void* malloc_alloc(MemoryState* _state, size_t nByte) {

	if (nByte > 512) {
		return NULL;
	}

	unsigned int size = (nByte - 1) >> 4;
	AlignmentHeader* Alignment = _state->alignments.used[size + size];
	uint8_t* bp;

	if (Alignment != Alignment->nextAlignment) {

		++Alignment->ref.count;
		bp = Alignment->freeBlock;

		if ((Alignment->freeBlock = *(uint8_t**)bp) == NULL) {
			malloc_Alignment_extend(Alignment, size);
		}

	}
	else {
		bp = (uint8_t*)allocate_from_new_Alignment(_state, size);
	}
	return (void*)bp;
}

void* object_malloc(MemoryState* _state, size_t numberByte) {

	void* ptr = malloc_alloc(_state, numberByte);
	if (ptr != NULL) {
		memset(ptr, 0, numberByte);
		return ptr;
	}
	ptr = AlifMem_raw_realloc(ptr, numberByte);
	if (ptr != NULL) {
		(_state)->mGmt.rawAllocatedBlocks++;
	}
	return ptr;

}

void* object_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	size_t numberByte = nElement * elSize;

	void* ptr = malloc_alloc(_state, numberByte);
	if (ptr != NULL) {
		memset(ptr, 0, numberByte);
		return ptr;
	}

	ptr = AlifMem_debug_raw_alloc(_state, 1, nElement * elSize);
	if (ptr != NULL) {
		_state->mGmt.rawAllocatedBlocks++;
	}
	return ptr;

}

int malloc_realloc(MemoryState* _state, void** newPtr, void* ptr, size_t numberByte) {

	void* bp;
	AlignmentHeader* Alignment;
	size_t size;

	Alignment = ((AlignmentHeader*)((void*)((uintptr_t)((ptr)) & ~(uintptr_t)(((1 << 14)) - 1))));

	if (!address_in_range(_state, ptr)) {
		return 0;
	}

	size = (((unsigned int)(Alignment->sizeIndex) + 1) << 4);

	if (numberByte <= size) {
		if (4 * numberByte > 3 * size) {
			*newPtr = ptr;
			return 1;
		}
		size = numberByte;
	}

	bp = object_malloc(_state, numberByte);

	if (bp != NULL) {
		memcpy(bp, ptr, size);
		object_free(_state, ptr);
	}
	*newPtr = bp;
	return 1;
}

void* object_realloc(MemoryState* _state, void* ptr, size_t numberByte) {

	void* ptr2;
	if (ptr == NULL) {
		object_malloc(_state, numberByte);
	}

	if (malloc_realloc(_state, &ptr2, ptr, numberByte)) {
		return ptr2;
	}
	return AlifMem_debug_raw_realloc(_state, ptr, numberByte);

}
