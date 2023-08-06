#include "alif.h"
#include "alifCore_runtime.h"

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


int malloc_free(MemoryState* _state, void* ptr);
void get_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator);
void set_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator);

static MemoryState state;

inline MemoryState* get_state(void) {

	return &state;

}

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

void* AlifMem_raw_malloc(void* ctx, size_t size) {

	if (size == 0) {
		size = 1;
	}
	return malloc(size);
}

void* AlifMem_raw_calloc(void* ctx, size_t nElement, size_t elSize) {


	if (nElement == 0 || elSize == 0) {
		nElement = 1;
		elSize = 1;
	}

	return calloc(nElement, elSize);
	
}

void* AlifMem_raw_realloc(void* ctx, void* ptr, size_t size) {

	if (size == 0) {
		size = 1;
	}
	return realloc(ptr, size);
}

void AlifMem_raw_free(void* ctx, void* ptr) {
	free(ptr);
}

#define MALLOC_ALLOC {nullptr, AlifMem_raw_malloc, AlifMem_raw_calloc, AlifMem_raw_realloc, AlifMem_raw_free}
#define ALIFRAW_ALLOC MALLOC_ALLOC

#ifdef WITH_ALIFMALLOC
void* object_malloc(void* ctx, size_t numberByte);
void* object_calloc(void* ctx, size_t nElement, size_t elSize);
void* object_realloc(void* ctx, void* ptr, size_t numberByte);
void  object_free(void* ctx, void* ptr);
#  define ALIFMALLOC_ALLOC {nullptr, object_malloc, object_calloc, object_realloc, object_free}
#else
  #define ALIFOBJ_ALLOC MALLOC_ALLOC
#endif

void* AlifMem_debug_raw_alloc(MemoryState* _state, size_t useCalloc, size_t nByte);
void* AlifMem_debug_raw_calloc(MemoryState* _state, size_t nElement, size_t elSize);
void* AlifMem_debug_raw_realloc(MemoryState* _state, void* ptr, size_t nByte);
void  AlifMem_debug_raw_free(MemoryState* _state, void* ptr);

void* AlifMem_malloc(MemoryState* _state, size_t size);
void* AlifMem_calloc(MemoryState* _state, size_t nElement, size_t elSize);
void* AlifMem_realloc(MemoryState* _state, void* ptr, size_t newSize);
void AlifMem_free(MemoryState* _state, void* ptr);

#define ALIFDEBUGRAW_ALLOC {&runtime.allocators.debug.raw, AlifMem_debug_raw_alloc,  AlifMem_debug_raw_calloc, AlifMem_debug_raw_realloc, AlifMem_debug_raw_free}

#define ALIFDEBUGMEM_ALLOC {&runtime.allocators.debug.mem, AlifMem_malloc, AlifMem_calloc, AlifMem_realloc, AlifMem_free}

#define ALIFDEBUGOBJ_ALLOC {&runtime.allocators.debug.obj, AlifMem_malloc, AlifMem_calloc, AlifMem_realloc, AlifMem_free}


#ifdef WITH_ALIFMALLOC
#  ifdef MS_WINDOWS
#    include <windows.h>
#  elif defined(HAVE_MMAP)
#    include <sys/mman.h>
#    ifdef MAP_ANONYMOUS
#      define ARENAS_USE_MMAP
#    endif
#  endif
#endif

#if defined(__has_feature)  /* Clang */
#  if __has_feature(address_sanitizer) /* is ASAN enabled? */
#    define _ALIF_NO_SANITIZE_ADDRESS \
        __attribute__((no_sanitize("address")))
#  endif
#  if __has_feature(thread_sanitizer)  /* is TSAN enabled? */
#    define _ALIF_NO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#  endif
#  if __has_feature(memory_sanitizer)  /* is MSAN enabled? */
#    define _ALIF_NO_SANITIZE_MEMORY __attribute__((no_sanitize_memory))
#  endif
#elif defined(__GNUC__)
#  if defined(__SANITIZE_ADDRESS__)    /* GCC 4.8+, is ASAN enabled? */
#    define _ALIF_NO_SANITIZE_ADDRESS \
        __attribute__((no_sanitize_address))
#  endif
// TSAN is supported since GCC 5.1, but __SANITIZE_THREAD__ macro
// is provided only since GCC 7.
#  if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1)
#    define _ALIF_NO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#  endif
#endif

// هذه تعاريف خاصة ب مكتشف الاخطاء في الذاكرة يسمى النظام ب sanitizers memory
// يعمل هذال النظام على اكتشاف اخطاء مثل كتابة عنوان فوق عنوان مكتوب مسبقا خاص بنظام الجهاز 

#ifndef _ALIF_NO_SANITIZE_ADDRESS
#  define _ALIF_NO_SANITIZE_ADDRESS
#endif
#ifndef _ALIF_NO_SANITIZE_THREAD
#  define _ALIF_NO_SANITIZE_THREAD
#endif
#ifndef _ALIF_NO_SANITIZE_MEMORY
#  define _ALIF_NO_SANITIZE_MEMORY
#endif

#define ALIFMEM_RAW (runtime.allocators.standard.raw)
#define ALIFMEM (runtime.allocators.standard.mem)
#define ALIFOBJECT (runtime.allocators.standard.obj)
#define ALIFMEM_DEBUG (runtime.allocators.debug)

int set_default_alloator_unlocked(AlifMemAllocateDomain domain, int debug, AlifMemAllocatorExternal* oldAlloc) {

	if(oldAlloc != nullptr) {
		get_allocator_unlocked(domain, oldAlloc);
	}

	AlifMemAllocatorExternal newAlloc;
	switch (domain)
	{
	case PYMEM_DOMAIN_RAW: newAlloc = ALIFRAW_ALLOC;
		break;
	case PYMEM_DOMAIN_MEM: newAlloc = ALIFMALLOC_ALLOC;
		break;
	case PYMEM_DOMAIN_OBJ: newAlloc = ALIFMALLOC_ALLOC;
		break;
	default:
		return -1;
	}

	get_allocator_unlocked(domain, &newAlloc);

	if (debug) {
		
	}
	return 0;
}


#ifdef Py_DEBUG
static const int pydebug = 1;
#else
static const int pydebug = 0;
#endif

void get_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator)
{
	//switch (domain)
	//{
	//case PYMEM_DOMAIN_RAW: *allocator = ALIFMEM_RAW; break;
	//case PYMEM_DOMAIN_MEM: *allocator = ALIFMEM; break;
	//case PYMEM_DOMAIN_OBJ: *allocator = ALIFOBJECT; break;
	//default:
	//	break;
	//}

}


void set_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator) {

	//switch (domain)
	//{
	//case PYMEM_DOMAIN_RAW: ALIFMEM_RAW = *allocator; break;
	//case PYMEM_DOMAIN_MEM: ALIFMEM = *allocator; break;
	//case PYMEM_DOMAIN_OBJ: ALIFOBJECT = *allocator; break;
	//default:
	//	break;
	//}

}

// لم يتم اكمل funtion بعد 

void set_up_debug_hooks_domain_unlocked(AlifMemAllocateDomain domain)
{
	//AlifMemAllocatorExternal alloc;

	//if (domain == PYMEM_DOMAIN_RAW) {
	//	if (ALIFMEM_RAW.malloc == AlifMem_debug_raw_alloc) {
	//		return;
	//	}

	//	get_allocator_unlocked(domain, &ALIFMEM_DEBUG.raw.alloc);
	//	alloc.ctx = &ALIFMEM_DEBUG.raw;

	//	set_allocator_unlocked(domain, &alloc);
	//}
	//else if (domain == PYMEM_DOMAIN_MEM) {
	//	if (ALIFMEM.malloc == AlifMem_debug_raw_alloc) {
	//		return;
	//	}

	//	get_allocator_unlocked(domain, &ALIFMEM_DEBUG.mem.alloc);
	//	alloc.ctx = &ALIFMEM_DEBUG.mem;

	//	set_allocator_unlocked(domain, &alloc);
	//}
	//else if (domain == PYMEM_DOMAIN_OBJ) {
	//	if (ALIFOBJECT.malloc == AlifMem_debug_raw_alloc) {
	//		return;
	//	}

	//	get_allocator_unlocked(domain, &ALIFMEM_DEBUG.obj.alloc);
	//	alloc.ctx = &ALIFMEM_DEBUG.obj;

	//	set_allocator_unlocked(domain, &alloc);
	//}
}

void* raw_malloc(size_t size) {

	if (size > 9223372036854775807i64) {
		return nullptr;
	}

	return AlifMem_raw_malloc(nullptr, size);
}

void* raw_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	rawOrMem = false;

	return AlifMem_debug_raw_alloc(_state, 1, nElement * elSize);

}

void* raw_realloc(MemoryState* _state, void* ptr, size_t newSize) {

	rawOrMem = false;

	if (newSize > 9223372036854775807i64) {
		return nullptr;
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

void* AlifObject_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	rawOrMem = false;

	return object_calloc(_state, nElement, elSize);

}

void* AlifObject_realloc(MemoryState* _state, void* ptr, size_t numberByte) {

	rawOrMem = false;

	return object_realloc(_state, ptr, numberByte);

}

void AlifObject_free(MemoryState* _state, void* ptr) {

	if (ptr == nullptr) {
		return;
	}

	objectOrMemDelete = true;

	if (!malloc_free(_state, ptr)) {

		AlifMem_debug_raw_free(_state, ptr);

	}

}


////////////////////////////////////////////////////////////////////////////////////////


inline BlockMapDown* Block_map_get(MemoryState* _state, uint8_t* ptr, int create) {

	int i1 = (((uintptr_t)(ptr)) >> ((((64 - 0) - 20 + 2) / 3) + (((64 - 0) - 20 + 2 + 20))) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1));

	if (_state->usage.BlockMapRoot.ptrs[i1] == nullptr) {

		if (!create) {
			return nullptr;
		}
		BlockMapMid* mid = (BlockMapMid*)AlifMem_debug_raw_alloc(_state, 1, sizeof(BlockMapMid));

		_state->usage.BlockMapRoot.ptrs[i1] = mid;
		_state->usage.BlockMapMidCount++;

	}
	int i2 = ((((uintptr_t)(ptr)) >> (((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3)) + 20)) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1));

	if (_state->usage.BlockMapRoot.ptrs[i1]->ptrs[i2] == nullptr) {

		if (!create) {
			return nullptr;
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
		if (botLo == nullptr) {
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

	BlockObject* blockObj{};
	blockObj = _state->mGmt.Blocks;

	Alignment->nextAlignment = blockObj->freeAlignments;
	blockObj->freeAlignments = Alignment;
	unsigned int numberFree = blockObj->numberFreeAlignments;

	BlockObject* lastNumberFree = state.mGmt.numberfreeAlignment[numberFree];
	if (lastNumberFree == blockObj) {
		BlockObject* Block = blockObj->prevBlock;
		state.mGmt.numberfreeAlignment[numberFree] = (Block != nullptr && Block->numberFreeAlignments == numberFree) ? Block : nullptr;
	}
	blockObj->numberFreeAlignments = ++numberFree;

	if (numberFree == blockObj->numberTotalAlignments && blockObj->nextBlock != nullptr) {
		if (blockObj->prevBlock == nullptr) {
			_state->mGmt.usableBlock = blockObj->nextBlock;
		}
		else {
			blockObj->prevBlock->nextBlock = blockObj->nextBlock;
		}

		if (blockObj->nextBlock != nullptr) {
			blockObj->nextBlock->prevBlock = blockObj->prevBlock;
		}

		blockObj->nextBlock = _state->mGmt.unusedBlock;
		_state->mGmt.unusedBlock = blockObj;


		Block_map_mark_used(_state, blockObj->address, 0);

		blockObj->address = 0;
		_state->mGmt.numberBlockCurrentlyAllocate;

		return;
	}

	if (numberFree == 1) {
		blockObj->nextBlock = _state->mGmt.usableBlock;
		blockObj->prevBlock = nullptr;
		if (_state->mGmt.usableBlock) {
			_state->mGmt.usableBlock->prevBlock = blockObj;
		}
		_state->mGmt.usableBlock = blockObj;
		if (_state->mGmt.numberfreeAlignment[1] == nullptr) {
			_state->mGmt.numberfreeAlignment[1] = blockObj;
		}
		return;
	}


	if (_state->mGmt.numberfreeAlignment[numberFree] == nullptr) {
		_state->mGmt.numberfreeAlignment[numberFree] = blockObj;
	}

	if (blockObj == lastNumberFree) {
		return;
	}

	if (blockObj->prevBlock != nullptr) {
		blockObj->prevBlock->nextBlock = blockObj->nextBlock;
	}
	else {
		_state->mGmt.usableBlock = blockObj->nextBlock;
	}


	blockObj->nextBlock->prevBlock = blockObj->prevBlock;

	blockObj->prevBlock = lastNumberFree;
	blockObj->nextBlock = lastNumberFree->nextBlock;
	if (blockObj->nextBlock != nullptr) {
		blockObj->nextBlock->prevBlock = blockObj;
	}
	lastNumberFree->nextBlock = blockObj;

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

	if (lastFree == nullptr) {
		insert_to_used_Alignment(_state, Alignment);
		return 1;
	}

	if (Alignment->ref.count != 0) {
		return 1;
	}

	insert_to_free_Alignment(_state, Alignment);
	return 1;

}

void object_free(void* ctx, void* ptr) {

	if (ptr == nullptr) {
		return;
	}

	MemoryState* state = get_state();

	if (!malloc_free(state, ptr)) {
		AlifMem_debug_raw_free(state, ptr);
		state->mGmt.rawAllocatedBlocks--;
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
			p = (uint8_t*)AlifMem_raw_calloc(nullptr, 1, total);
		}

	}
	else {

		if (rawOrMem) {
			p = (uint8_t*)object_malloc(nullptr, total);
		}
		else {
			p = (uint8_t*)AlifMem_raw_malloc(nullptr, total);
		}

	}
	if (p == nullptr) {
		return nullptr;
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

	if (ptr == nullptr) {
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
		AlifMem_raw_free(nullptr, q);
	}
}

void* AlifMem_debug_raw_calloc(MemoryState* _state, size_t nElement, size_t elSize) {

	size_t numberByte = nElement * elSize;

	return AlifMem_debug_raw_alloc(_state, 1, numberByte);

}

void* AlifMem_debug_raw_realloc(MemoryState* _state, void* ptr, size_t nByte) {

	if (ptr == nullptr) {
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
	r = (uint8_t*)AlifMem_raw_realloc(nullptr, head, total);

	if (r == nullptr) {
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

	if (r == nullptr) {
		return nullptr;
	}
	if (nByte > originalNByte) {
		memset(data + originalNByte, 0xCD, nByte - originalNByte);
	}

	return data;

}

BlockObject* new_Block(MemoryState* _state) {


	BlockObject* BlockObj = nullptr;
	unsigned int excess;
	void* address = nullptr;

	if ((_state)->mGmt.unusedBlock == nullptr) {

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
			(_state)->mGmt.Blocks[i].nextBlock = i < numberBlock - 1 ? &(_state)->mGmt.Blocks[i + 1] : nullptr;

		}

		(_state)->mGmt.unusedBlock = &(_state)->mGmt.Blocks[(_state)->mGmt.maxBlock];
		(_state)->mGmt.maxBlock = numberBlock;
	}

	BlockObj = _state->mGmt.unusedBlock;
	address = (void*)VirtualAlloc(nullptr, (1 << 20), 0x00001000 | 0x00002000, 0x04);

	if (address != nullptr) {
		Block_map_mark_used(_state, (uintptr_t)address, 1);
	}

	if (address == nullptr) {
		BlockObj->nextBlock = (_state)->mGmt.unusedBlock;
		(_state)->mGmt.unusedBlock = BlockObj;
		return nullptr;
	}

	BlockObj->address = (uintptr_t)address;

	++(_state)->mGmt.numberBlockCurrentlyAllocate;
	++(_state)->mGmt.numberBlockAllocated;
	if ((_state)->mGmt.numberBlockCurrentlyAllocate > (_state)->mGmt.numberBlockHighWater) {
		(_state)->mGmt.numberBlockHighWater = (_state)->mGmt.numberBlockCurrentlyAllocate;
	}
	BlockObj->freeAlignments = nullptr;


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
		*(uint8_t**)(Alignment->freeBlock) = nullptr;
		return;
	}

	AlignmentHeader* next;
	next = Alignment->nextAlignment;
	Alignment = Alignment->prevAlignment;
	next->prevAlignment = Alignment;
	Alignment->nextAlignment = next;

}

void* allocate_from_new_Alignment(MemoryState* _state, unsigned int size) {

	if ((_state)->mGmt.usableBlock == nullptr) {

		(_state)->mGmt.usableBlock = new_Block(_state);
		if ((_state)->mGmt.usableBlock == nullptr) {
			return nullptr;
		}
		(_state)->mGmt.usableBlock->nextBlock = (_state)->mGmt.usableBlock->prevBlock = nullptr;
		(_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments] = (_state)->mGmt.usableBlock;
	}
	if ((_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments] == (_state)->mGmt.usableBlock) {
		(_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments] = nullptr;
	}
	if ((_state)->mGmt.usableBlock->numberFreeAlignments > 1) {
		(_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableBlock->numberFreeAlignments - 1] = (_state)->mGmt.usableBlock;
	}

	AlignmentHeader* Alignment = (_state)->mGmt.usableBlock->freeAlignments;
	if (Alignment != nullptr) {
		(_state)->mGmt.usableBlock->freeAlignments = Alignment->nextAlignment;
		(_state)->mGmt.usableBlock->numberFreeAlignments--;
		if ((_state)->mGmt.usableBlock->numberFreeAlignments == 0) {
			(_state)->mGmt.usableBlock = (_state)->mGmt.usableBlock->nextBlock;
			if ((_state)->mGmt.usableBlock != nullptr) {
				(_state)->mGmt.usableBlock->prevBlock = nullptr;
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
			if ((_state)->mGmt.usableBlock != nullptr) {
				(_state)->mGmt.usableBlock->prevBlock = nullptr;
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
	*(uint8_t**)(Alignment->freeBlock) = nullptr;
	return bp;

}

void* malloc_alloc(MemoryState* _state, size_t nByte) {

	if (nByte > 512) {
		return nullptr;
	}

	unsigned int size = (unsigned int)(nByte - 1) >> 4;
	AlignmentHeader* Alignment = _state->alignments.used[size + size];
	uint8_t* bp;

	if (Alignment != Alignment->nextAlignment) {

		++Alignment->ref.count;
		bp = Alignment->freeBlock;

		if ((Alignment->freeBlock = *(uint8_t**)bp) == nullptr) {
			malloc_Alignment_extend(Alignment, size);
		}

	}
	else {
		bp = (uint8_t*)allocate_from_new_Alignment(_state, size);
	}
	return (void*)bp;
}

void* object_malloc(void* ctx, size_t numberByte) {

	MemoryState* state = get_state();

	void* ptr = malloc_alloc(state, numberByte);
	if (ptr != nullptr) {
		memset(ptr, 0, numberByte);
		return ptr;
	}
	ptr = AlifMem_raw_realloc(nullptr, ptr, numberByte);
	if (ptr != nullptr) {
		(state)->mGmt.rawAllocatedBlocks++;
	}
	return ptr;

}

void* object_calloc(void* ctx, size_t nElement, size_t elSize) {

	MemoryState* state = get_state();

	size_t numberByte = nElement * elSize;

	void* ptr = malloc_alloc(state, numberByte);
	if (ptr != nullptr) {
		memset(ptr, 0, numberByte);
		return ptr;
	}

	ptr = AlifMem_debug_raw_alloc(state, 1, nElement * elSize);
	if (ptr != nullptr) {
		state->mGmt.rawAllocatedBlocks++;
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

	bp = object_malloc(nullptr, numberByte);

	if (bp != nullptr) {
		memcpy(bp, ptr, size);
		object_free(_state, ptr);
	}
	*newPtr = bp;
	return 1;
}

void* object_realloc(void* ctx, void* ptr, size_t size) {

	void* ptr2;
	if (ptr == nullptr) {
		return object_malloc(nullptr, size);
	}

	MemoryState* state = get_state();

	if (malloc_realloc(state, &ptr2, ptr, size)) {
		return ptr2;
	}
	return AlifMem_debug_raw_realloc(state, ptr, size);

}
