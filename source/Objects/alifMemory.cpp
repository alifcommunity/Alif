#include "alif.h"
#include "alifCore_runtime.h"
#include "alifCore_alifMem.h"
#include "thread_nt.h"

/*

	هناك نوعان من الذاكرة الاول المسمى raw والثاني المسممى mem
	raw يستخدم ذاكرة منخفضة المستوى مباشرة من لغة c
	mem يسخدم الذاكرة رفيعة المستوى من مميزاتها
	   - تتبع التخصيصات و الغاء التخصيصات
	   - وايضا يتحقق من الاخطاء
	   - ويدعم تخصيصات الذاكرة المختلفة

*/


void* alifMem_raw_malloc(void* ctx, size_t size);
void* alifMem_raw_calloc(void* ctx, size_t nElement, size_t elSize);
void* alifMem_raw_realloc(void* ctx, void* ptr, size_t size);
void alifMem_raw_free(void* ctx, void* ptr);

void get_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator);
void set_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator);
void set_up_debug_hooks_domain_unlocked(AlifMemAllocateDomain domain);

static MemoryState _state; // قد لا يكون لها حاجة

inline MemoryState* get_state(void) {
	return &_state;
}

// raw memory ///////////////////////////////////////////////////////////////////////////////

/***************************************/
/* low-level allocator implementations */
/***************************************/

void* alifMem_raw_malloc(void* ctx, size_t size) {

	if (size == 0) {
		size = 1;
	}
	return malloc(size);
}

void* alifMem_raw_calloc(void* ctx, size_t nElement, size_t elSize) {


	if (nElement == 0 || elSize == 0) {
		nElement = 1;
		elSize = 1;
	}

	return calloc(nElement, elSize);
	
}

void* alifMem_raw_realloc(void* ctx, void* ptr, size_t size) {

	if (size == 0) {
		size = 1;
	}
	return realloc(ptr, size);
}

void alifMem_raw_free(void* ctx, void* ptr) {
	free(ptr);
}

#define MALLOC_ALLOC {nullptr, alifMem_raw_malloc, alifMem_raw_calloc, alifMem_raw_realloc, alifMem_raw_free}
#define ALIFRAW_ALLOC MALLOC_ALLOC

#ifdef WITH_ALIFMALLOC
void* object_malloc(void* ctx, size_t numberByte);
void* object_calloc(void* ctx, size_t nElement, size_t elSize);
void* object_realloc(void* ctx, void* ptr, size_t numberByte);
void  object_free(void* ctx, void* ptr);
#  define ALIFMALLOC_ALLOC {nullptr, object_malloc, object_calloc, object_realloc, object_free}
#  define ALIFOBJ_ALLOC ALIFMALLOC_ALLOC
#else
  #define ALIFOBJ_ALLOC MALLOC_ALLOC
#endif

#define ALIFMEM_ALLOC ALIFOBJ_ALLOC

void* alifMem_debug_raw_malloc(void* ctx, size_t numberByte);
void* alifMem_debug_raw_calloc(void* ctx, size_t nElement, size_t elSize);
void* alifMem_debug_raw_realloc(void* ctx, void* ptr, size_t nByte);
void  alifMem_debug_raw_free(void* ctx, void* ptr);

void* alifMem_malloc(void* ctx, size_t size);
void* alifMem_calloc(void* ctx, size_t nElement, size_t elSize);
void* alifMem_realloc(void* ctx, void* ptr, size_t newSize);
void alifMem_free(void* ctx, void* ptr);

#define ALIFDEBUGRAW_ALLOC {&alifRuntime.allocators.debug.raw, alifMem_debug_raw_alloc,  alifMem_debug_raw_calloc, alifMem_debug_raw_realloc, alifMem_debug_raw_free}

#define ALIFDEBUGMEM_ALLOC {&alifRuntime.allocators.debug.mem, alifMem_debug_raw_malloc, alifMem_debug_raw_calloc, alifMem_debug_raw_realloc, alifMem_debug_raw_free}

#define ALIFDEBUGMEM_ALLOC {&alifRuntime.allocators.debug.obj, alifMem_debug_raw_malloc, alifMem_debug_raw_calloc, alifMem_debug_raw_realloc, alifMem_debug_raw_free}


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

void* alifMem_arenaAlloc(void* ctx, size_t size) {

#ifdef MS_WINDOWS
	return VirtualAlloc(nullptr, size, 0x00001000 | 0x00002000, 0x04);
#elif defined(ARENA_USE_MMAP)
	void* ptr;
	ptr = mmap(nullptr, size, PORT_READ | PORT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED) {
		return nullptr;
	}
	return ptr;
#else
	return malloc(size);
#endif
}

void alifMem_arenaFree(void* ctx, void* ptr, size_t size)
{
#ifdef MS_WINDOWS
	VirtualFree(ptr, 0, 0x00008000);
#elif defined(ARENA_USE_MMAP)
	munmap(ptr, size);
#else
	free(ptr);
#endif
}

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

#define ALLOCATORS_MUTEX (alifRuntime.allocators.mutex)
#define ALIFMEM_RAW (alifRuntime.allocators.standard.raw)
#define ALIFMEM (alifRuntime.allocators.standard.mem)
#define ALIFOBJECT (alifRuntime.allocators.standard.obj)
#define ALIFMEM_DEBUG (alifRuntime.allocators.debug)

int set_default_alloator_unlocked(AlifMemAllocateDomain domain, int debug, AlifMemAllocatorExternal* oldAlloc) {

	if(oldAlloc != nullptr) {
		get_allocator_unlocked(domain, oldAlloc);
	}

	AlifMemAllocatorExternal newAlloc;
	switch (domain)
	{
	case ALIFMEM_DOMAIN_RAW: newAlloc = ALIFRAW_ALLOC;
		break;
	case ALIFMEM_DOMAIN_MEM: newAlloc = ALIFMEM_ALLOC;
		break;
	case ALIFMEM_DOMAIN_OBJ: newAlloc = ALIFOBJ_ALLOC;
		break;
	default:
		return -1;
	}

	get_allocator_unlocked(domain, &newAlloc);

	if (debug) {
		set_up_debug_hooks_domain_unlocked(domain);
	}
	return 0;
}


#ifdef Py_DEBUG
static const int alifDebug = 1;
#else
static const int alifDebug = 0;
#endif

int alifMem_setDefaultAllocator(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* oldAlloc) {

	if (ALLOCATORS_MUTEX == nullptr) {
		return set_default_alloator_unlocked(domain, alifDebug, oldAlloc);
	}
	alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
	int res = set_default_alloator_unlocked(domain, alifDebug, oldAlloc);
	alifThread_release_lock(ALLOCATORS_MUTEX);
	return res;

}

void set_up_debug_hooks_domain_unlocked(AlifMemAllocateDomain domain)
{
	AlifMemAllocatorExternal alloc;

	if (domain == ALIFMEM_DOMAIN_RAW) {
		if (ALIFMEM_RAW.malloc == alifMem_debug_raw_malloc) {
			return;
		}

		get_allocator_unlocked(domain, &ALIFMEM_DEBUG.raw.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.raw;
		alloc.malloc = alifMem_debug_raw_malloc;
		alloc.calloc = alifMem_debug_raw_calloc;
		alloc.realloc = alifMem_debug_raw_realloc;
		alloc.free = alifMem_debug_raw_free;
		set_allocator_unlocked(domain, &alloc);
	}
	else if (domain == ALIFMEM_DOMAIN_MEM) {
		if (ALIFMEM.malloc == alifMem_debug_raw_malloc) {
			return;
		}

		get_allocator_unlocked(domain, &ALIFMEM_DEBUG.mem.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.mem;
		alloc.malloc = alifMem_debug_raw_malloc;
		alloc.calloc = alifMem_debug_raw_calloc;
		alloc.realloc = alifMem_debug_raw_realloc;
		alloc.free = alifMem_debug_raw_free;
		set_allocator_unlocked(domain, &alloc);
	}
	else if (domain == ALIFMEM_DOMAIN_OBJ) {
		if (ALIFOBJECT.malloc == alifMem_debug_raw_malloc) {
			return;
		}

		get_allocator_unlocked(domain, &ALIFMEM_DEBUG.obj.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.obj;
		alloc.malloc = alifMem_debug_raw_malloc;
		alloc.calloc = alifMem_debug_raw_calloc;
		alloc.realloc = alifMem_debug_raw_realloc;
		alloc.free = alifMem_debug_raw_free;
		set_allocator_unlocked(domain, &alloc);
	}
}

void get_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator)
{
	switch (domain)
	{
	case ALIFMEM_DOMAIN_RAW: *allocator = ALIFMEM_RAW; break;
	case ALIFMEM_DOMAIN_MEM: *allocator = ALIFMEM; break;
	case ALIFMEM_DOMAIN_OBJ: *allocator = ALIFOBJECT; break;
	default:
		allocator->ctx = nullptr;
		allocator->malloc = nullptr;
		allocator->calloc = nullptr;
		allocator->realloc = nullptr;
		allocator->free = nullptr;
	}

}

void set_allocator_unlocked(AlifMemAllocateDomain domain, AlifMemAllocatorExternal* allocator) {

	switch (domain)
	{
	case ALIFMEM_DOMAIN_RAW: ALIFMEM_RAW = *allocator; break;
	case ALIFMEM_DOMAIN_MEM: ALIFMEM = *allocator; break;
	case ALIFMEM_DOMAIN_OBJ: ALIFOBJECT = *allocator; break;
	}

}

void alifMem_setAllocator(AlifMemAllocateDomain domain, AlifMemAllocatorExternal *allocator) {

	if (ALLOCATORS_MUTEX == nullptr) {
		set_allocator_unlocked(domain, allocator);
		return;
	}
	alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
	set_allocator_unlocked(domain, allocator);
	alifThread_release_lock(ALLOCATORS_MUTEX);
}

void* raw_malloc(size_t size) {

	if (size > 9223372036854775807i64) {
		return nullptr;
	}
	
	return ALIFMEM_RAW.malloc(ALIFMEM_RAW.ctx, size);
}

void* raw_calloc( size_t nElement, size_t elSize) {

	return ALIFMEM_RAW.calloc(ALIFMEM_RAW.ctx, nElement, elSize);

}

void* raw_realloc(void* ptr, size_t newSize) {

	if (newSize > 9223372036854775807i64) {
		return nullptr;
	}

	return ALIFMEM_RAW.realloc(ALIFMEM_RAW.ctx, ptr, newSize);

}

void raw_free(void* ptr) {

	ALIFMEM_RAW.free(ALIFMEM_RAW.ctx, ptr);

}


// mem memory ///////////////////////////////////////////////////////////////////////////////

void* alifMem_malloc(void* ctx, size_t size) {

	return ALIFMEM.malloc(ALIFMEM.ctx, size);
}

void* alifMem_calloc(void* ctx, size_t nElement, size_t elSize) {

	return ALIFMEM.calloc(ALIFMEM.ctx, nElement, elSize);
}

void* alifMem_realloc(void* ctx, void* ptr, size_t newSize) {

	return ALIFMEM.realloc(ALIFMEM.ctx, ptr, newSize);
}

void alifMem_free(void* ctx, void* ptr) {

	ALIFMEM.free(ALIFMEM.ctx, ptr);
}

/* ___________ alifMem functions ___________ */

// يجب مراجعة هذه الدالة لانه تم تعديلها تعديلات كبيرة
char* alifMem_rawStrDup(const char* str)
{
	size_t size = strlen(str) + 1;
	char* copy = (char*)alifMem_malloc(nullptr, size);
	if (copy == nullptr) {
		return nullptr;
	}
	memcpy(copy, str, size);
	return copy;
}


// object memory ///////////////////////////////////////////////////////////////////////////////

void* AlifObject_malloc(void* ctx, size_t size) {

	return ALIFOBJECT.malloc(ALIFOBJECT.ctx, size);

}

void* AlifObject_calloc(void* ctx, size_t nElement, size_t elSize) {

	return ALIFOBJECT.calloc(ALIFOBJECT.ctx, nElement, elSize);

}

void* AlifObject_realloc(void* ctx, void* ptr, size_t newSize) {

	return ALIFOBJECT.realloc(ALIFOBJECT.ctx, ptr, newSize);

}

void AlifObject_free(void* ctx, void* ptr) {

	ALIFOBJECT.free(ALIFOBJECT.ctx, ptr);

}

////////////////////////////////////////////////////////////////////////////////////////


inline BlockMapDown* Block_map_get(MemoryState* state, uint8_t* ptr, int create)
{
	int i1 = ((((uintptr_t)(ptr)) >> ((((64 - 0) - 20 + 2) / 3) + (((64 - 0) - 20 + 2 + 20))) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1)));

	if (state->usage.BlockMapRoot.ptrs[i1] == nullptr) {

		if (!create) {
			return nullptr;
		}
		BlockMapMid* mid = (BlockMapMid*)raw_calloc( 1, sizeof(BlockMapMid));

		state->usage.BlockMapRoot.ptrs[i1] = mid;
		state->usage.BlockMapMidCount++;

	}
	int i2 = ((((uintptr_t)(ptr)) >> (((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3)) + 20)) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1));

	if (state->usage.BlockMapRoot.ptrs[i1]->ptrs[i2] == nullptr) {

		if (!create) {
			return nullptr;
		}
		BlockMapDown* bot = (BlockMapDown*)raw_calloc(1, sizeof(BlockMapDown));

		state->usage.BlockMapRoot.ptrs[i1]->ptrs[i2] = bot;
		state->usage.BlockMapMidCount++;
	}
	return state->usage.BlockMapRoot.ptrs[i1]->ptrs[i2];

}

void Block_map_mark_used(MemoryState* state, uintptr_t BlockBase, int isUsed) {

	BlockMapDown* bot = Block_map_get(state, (uint8_t*)BlockBase, isUsed);

	int i3 = ((((uintptr_t)((uint8_t*)BlockBase)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));
	int32_t tail = (int32_t)(BlockBase & ((1 << 20) - 1));
	if (tail == 0) {
		bot->Block[i3].tailHi = isUsed ? -1 : 0;
	}
	else {
		bot->Block[i3].tailHi = isUsed ? tail : 0;
		uintptr_t BlockBaseNext = BlockBase + (1 << 20);
		BlockMapDown* botLo = Block_map_get(state, (uint8_t*)BlockBaseNext, isUsed);
		if (botLo == nullptr) {
			bot->Block[i3].tailHi = 0;
			return;
		}
		int i3Next = ((((uintptr_t)(BlockBaseNext)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));
		botLo->Block[i3Next].tailLo = isUsed ? tail : 0;
	}

}

int Block_map_is_used(MemoryState* state, uint8_t* ptr) {

	BlockMapDown* mid = Block_map_get(state, ptr, 0);

	int i3 = ((((uintptr_t)((uint8_t*)ptr)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));

	int32_t hi = mid->Block[i3].tailHi;
	int32_t lo = mid->Block[i3].tailLo;
	int32_t tail = (int32_t)((uintptr_t)(ptr) & ((1 << 20) - 1));
	return (tail < lo) || (tail >= hi && hi != 0);

}

void insert_to_usedAlignment(MemoryState* _state, AlignmentHeader* Alignment) {

	size_t size = Alignment->sizeIndex;
	AlignmentHeader* next = _state->alignments.used[size * 2];
	AlignmentHeader* prev = next->prevAlignment;

	Alignment->nextAlignment = next;
	Alignment->prevAlignment = prev;
	next->prevAlignment = Alignment;
	prev->nextAlignment = Alignment;

}

void insert_to_freeAlignment(MemoryState* state, AlignmentHeader* Alignment) {

	AlignmentHeader* next = Alignment->nextAlignment;
	AlignmentHeader* prev = Alignment->prevAlignment;
	next->prevAlignment = prev;
	prev->nextAlignment = next;

	BlockObject* blockObj{};
	blockObj = state->mGmt.Blocks;

	Alignment->nextAlignment = blockObj->freeAlignments;
	blockObj->freeAlignments = Alignment;
	unsigned int numberFree = blockObj->numberFreeAlignments;

	BlockObject* lastNumberFree = state->mGmt.numberfreeAlignment[numberFree];
	if (lastNumberFree == blockObj) {
		BlockObject* Block = blockObj->prevBlock;
		state->mGmt.numberfreeAlignment[numberFree] = (Block != nullptr && Block->numberFreeAlignments == numberFree) ? Block : nullptr;
	}
	blockObj->numberFreeAlignments = ++numberFree;

	if (numberFree == blockObj->numberTotalAlignments && blockObj->nextBlock != nullptr) {
		if (blockObj->prevBlock == nullptr) {
			state->mGmt.usableBlock = blockObj->nextBlock;
		}
		else {
			blockObj->prevBlock->nextBlock = blockObj->nextBlock;
		}

		if (blockObj->nextBlock != nullptr) {
			blockObj->nextBlock->prevBlock = blockObj->prevBlock;
		}

		blockObj->nextBlock = state->mGmt.unusedBlock;
		state->mGmt.unusedBlock = blockObj;


		Block_map_mark_used(state, blockObj->address, 0);

		blockObj->address = 0;
		state->mGmt.numberBlockCurrentlyAllocate;

		return;
	}

	if (numberFree == 1) {
		blockObj->nextBlock = state->mGmt.usableBlock;
		blockObj->prevBlock = nullptr;
		if (state->mGmt.usableBlock) {
			state->mGmt.usableBlock->prevBlock = blockObj;
		}
		state->mGmt.usableBlock = blockObj;
		if (state->mGmt.numberfreeAlignment[1] == nullptr) {
			state->mGmt.numberfreeAlignment[1] = blockObj;
		}
		return;
	}


	if (state->mGmt.numberfreeAlignment[numberFree] == nullptr) {
		state->mGmt.numberfreeAlignment[numberFree] = blockObj;
	}

	if (blockObj == lastNumberFree) {
		return;
	}

	if (blockObj->prevBlock != nullptr) {
		blockObj->prevBlock->nextBlock = blockObj->nextBlock;
	}
	else {
		state->mGmt.usableBlock = blockObj->nextBlock;
	}


	blockObj->nextBlock->prevBlock = blockObj->prevBlock;

	blockObj->prevBlock = lastNumberFree;
	blockObj->nextBlock = lastNumberFree->nextBlock;
	if (blockObj->nextBlock != nullptr) {
		blockObj->nextBlock->prevBlock = blockObj;
	}
	lastNumberFree->nextBlock = blockObj;

}

bool address_in_range(MemoryState* state, void* ptr) {
	return Block_map_is_used(state, (uint8_t*)ptr);
}

int malloc_free(MemoryState* state, void* ptr) {

	AlignmentHeader* Alignment = ((AlignmentHeader*)((void*)((uintptr_t)((ptr)) & ~(uintptr_t)(((1 << 14)) - 1))));

	if (!address_in_range(state, ptr)) {
		return 0;
	}

	uint8_t* lastFree = Alignment->freeBlock;
	*(uint8_t**)ptr = lastFree;
	Alignment->freeBlock = (uint8_t*)ptr;
	Alignment->ref.count--;

	if (lastFree == nullptr) {
		insert_to_usedAlignment(state, Alignment);
		return 1;
	}

	if (Alignment->ref.count != 0) {
		return 1;
	}

	insert_to_freeAlignment(state, Alignment);
	return 1;

}

void object_free(void* ctx, void* ptr) {

	if (ptr == nullptr) {
		return;
	}

	MemoryState* state = get_state();

	if (!malloc_free(state, ptr)) {
		raw_free( ptr);
		state->mGmt.rawAllocatedBlocks--;
	}

}

BlockObject* new_Block(MemoryState* state) {


	BlockObject* BlockObj = nullptr;
	unsigned int excess;
	void* address = nullptr;

	if ((state)->mGmt.unusedBlock == nullptr) {

		unsigned int i;
		unsigned int numberBlock;
		size_t nByte;

		numberBlock = (state)->mGmt.maxBlock ? (state)->mGmt.maxBlock << 1 : 16;

		nByte = numberBlock * sizeof(*(state)->mGmt.Blocks);
		BlockObj = (BlockObject*)raw_realloc(state->mGmt.Blocks, nByte);
		(state)->mGmt.Blocks = BlockObj;

		for (i = (state)->mGmt.maxBlock; i < numberBlock; ++i)
		{
			(state)->mGmt.Blocks[i].address = 0;
			(state)->mGmt.Blocks[i].nextBlock = i < numberBlock - 1 ? &(state)->mGmt.Blocks[i + 1] : nullptr;

		}

		(state)->mGmt.unusedBlock = &(state)->mGmt.Blocks[(state)->mGmt.maxBlock];
		(state)->mGmt.maxBlock = numberBlock;
	}

	BlockObj = state->mGmt.unusedBlock;
	address = (void*)VirtualAlloc(nullptr, (1 << 20), 0x00001000 | 0x00002000, 0x04);

	if (address != nullptr) {
		Block_map_mark_used(state, (uintptr_t)address, 1);
	}

	if (address == nullptr) {
		BlockObj->nextBlock = (state)->mGmt.unusedBlock;
		(state)->mGmt.unusedBlock = BlockObj;
		return nullptr;
	}

	BlockObj->address = (uintptr_t)address;

	++(state)->mGmt.numberBlockCurrentlyAllocate;
	++(state)->mGmt.numberBlockAllocated;
	if ((state)->mGmt.numberBlockCurrentlyAllocate > (state)->mGmt.numberBlockHighWater) {
		(state)->mGmt.numberBlockHighWater = (state)->mGmt.numberBlockCurrentlyAllocate;
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

void malloc_Alignment_extend(AlignmentHeader* Alignment, size_t _size) {

	if (Alignment->nextOffset <= Alignment->maxNextOffset) {

		Alignment->freeBlock = (uint8_t*)Alignment + Alignment->nextOffset;
		Alignment->nextOffset += (((_size)+1) << 4);
		*(uint8_t**)(Alignment->freeBlock) = nullptr;
		return;
	}

	AlignmentHeader* next;
	next = Alignment->nextAlignment;
	Alignment = Alignment->prevAlignment;
	next->prevAlignment = Alignment;
	Alignment->nextAlignment = next;

}

void* allocate_From_newAlignment(MemoryState* state, size_t _size) {

	if ((state)->mGmt.usableBlock == nullptr) {

		(state)->mGmt.usableBlock = new_Block(state);
		if ((state)->mGmt.usableBlock == nullptr) {
			return nullptr;
		}
		(state)->mGmt.usableBlock->nextBlock = (state)->mGmt.usableBlock->prevBlock = nullptr;
		(state)->mGmt.numberfreeAlignment[(state)->mGmt.usableBlock->numberFreeAlignments] = (state)->mGmt.usableBlock;
	}
	if ((state)->mGmt.numberfreeAlignment[(state)->mGmt.usableBlock->numberFreeAlignments] == (state)->mGmt.usableBlock) {
		(state)->mGmt.numberfreeAlignment[(state)->mGmt.usableBlock->numberFreeAlignments] = nullptr;
	}
	if ((state)->mGmt.usableBlock->numberFreeAlignments > 1) {
		(state)->mGmt.numberfreeAlignment[(state)->mGmt.usableBlock->numberFreeAlignments - 1] = (state)->mGmt.usableBlock;
	}

	AlignmentHeader* Alignment = (state)->mGmt.usableBlock->freeAlignments;
	if (Alignment != nullptr) {
		(state)->mGmt.usableBlock->freeAlignments = Alignment->nextAlignment;
		(state)->mGmt.usableBlock->numberFreeAlignments--;
		if ((state)->mGmt.usableBlock->numberFreeAlignments == 0) {
			(state)->mGmt.usableBlock = (state)->mGmt.usableBlock->nextBlock;
			if ((state)->mGmt.usableBlock != nullptr) {
				(state)->mGmt.usableBlock->prevBlock = nullptr;
			}
		}
	}
	else {
		Alignment = (AlignmentHeader*)state->mGmt.usableBlock->alignmentAddress;
		Alignment->BlockIndex = (unsigned int)(state->mGmt.usableBlock - state->mGmt.Blocks);
		Alignment->sizeIndex = 0xffff;
		(state)->mGmt.usableBlock->alignmentAddress += (1 << 14);
		--(state)->mGmt.usableBlock->numberFreeAlignments;

		if ((state)->mGmt.usableBlock->numberFreeAlignments == 0) {
			(state)->mGmt.usableBlock = (state)->mGmt.usableBlock->nextBlock;
			if ((state)->mGmt.usableBlock != nullptr) {
				(state)->mGmt.usableBlock->prevBlock = nullptr;
			}
		}
	}

	uint8_t* bp;
	AlignmentHeader* next = (state)->alignments.used[_size + _size];
	Alignment->nextAlignment = next;
	Alignment->prevAlignment = next;
	next->nextAlignment = Alignment;
	next->prevAlignment = Alignment;
	Alignment->ref.count = 1;
	if (Alignment->sizeIndex == _size) {
		bp = Alignment->freeBlock;
		Alignment->freeBlock = *(uint8_t**)bp;
		return bp;
	}

	Alignment->sizeIndex = _size;
	_size = (((_size)+1) << 4);
	bp = (uint8_t*)Alignment + SIZE_ROUND_UP(sizeof(AlignmentHeader), 16));
	Alignment->nextOffset = SIZE_ROUND_UP(sizeof(AlignmentHeader), 16)) + (_size << 1);
	Alignment->maxNextOffset = (1 << 14) - _size;
	Alignment->freeBlock = bp + _size;
	*(uint8_t**)(Alignment->freeBlock) = nullptr;
	return bp;

}

void* malloc_alloc(MemoryState* state, size_t nByte) {

	if (nByte > 512) {
		return nullptr;
	}

	unsigned int size = (unsigned int)(nByte - 1) >> 4;
	AlignmentHeader* Alignment = state->alignments.used[size + size];
	uint8_t* bp;

	if (Alignment != Alignment->nextAlignment) {

		++Alignment->ref.count;
		bp = Alignment->freeBlock;

		if ((Alignment->freeBlock = *(uint8_t**)bp) == nullptr) {
			malloc_Alignment_extend(Alignment, size);
		}

	}
	else {
		bp = (uint8_t*)allocate_From_newAlignment(state, size);
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
	ptr = alifMem_raw_realloc(nullptr, ptr, numberByte);
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

	ptr = raw_calloc(1, nElement * elSize);
	if (ptr != nullptr) {
		state->mGmt.rawAllocatedBlocks++;
	}
	return ptr;

}

int malloc_realloc(MemoryState* state, void* ctx, void** newPtr, void* ptr, size_t numberByte) {

	void* bp;
	AlignmentHeader* Alignment;
	size_t size;

	Alignment = ((AlignmentHeader*)((void*)((uintptr_t)((ptr)) & ~(uintptr_t)(((1 << 14)) - 1))));

	if (!address_in_range(state, ptr)) {
		return 0;
	}

	size = ((((size_t)Alignment->sizeIndex) + 1) << 4);

	if (numberByte <= size) {
		if (4 * numberByte > 3 * size) {
			*newPtr = ptr;
			return 1;
		}
		size = numberByte;
	}

	bp = object_malloc(ctx, numberByte);

	if (bp != nullptr) {
		memcpy(bp, ptr, size);
		object_free(ctx, ptr);
	}
	*newPtr = bp;
	return 1;
}

void* object_realloc(void* ctx, void* ptr, size_t size) {

	void* ptr2;
	if (ptr == nullptr) {
		return object_malloc(ctx, size);
	}

	MemoryState* state = get_state();

	if (malloc_realloc(state, ctx, &ptr2, ptr, size)) {
		return ptr2;
	}
	return raw_realloc(ptr, size);

}

void write_size_t(void* p, size_t n)
{
	uint8_t* q = (uint8_t*)p + 8 - 1;
	int i;

	for (i = 8; --i >= 0; --q) {
		*q = (uint8_t)(n & 0xff);
		n >>= 8;
	}
}

size_t read_size_t(const void* p)
{
	const uint8_t* q = (const uint8_t*)p;
	size_t result = *q++;
	size_t i;

	for (i = 8; --i > 0; ++q)
		result = (result << 8) | *q;
	return result;
}

void* alifMem_debug_raw_alloc(int useCalloc, void* ctx, size_t nByte) {

	DebugAllocateAPI* api = (DebugAllocateAPI*)ctx;
	uint8_t* p;
	uint8_t* data;
	uint8_t* tail;
	size_t total;

	if (nByte > (size_t)9223372036854775807i64 - 24) {
		return nullptr;
	}

	total = nByte + 24;

	if (useCalloc) {
		p = (uint8_t*)api->alloc.calloc(api->alloc.ctx, 1, total);
	}
	else {
		p = (uint8_t*)api->alloc.malloc(api->alloc.ctx, total);
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

void* alifMem_debug_raw_malloc(void* ctx, size_t numberByte) {
	return alifMem_debug_raw_alloc(0, ctx, numberByte);
}

void* alifMem_debug_raw_calloc(void* ctx, size_t nElement, size_t elSize) {

	size_t numberByte = nElement * elSize;

	return alifMem_debug_raw_alloc(1, ctx, numberByte);

}


void alifMem_debug_raw_free(void* ctx, void* ptr) {

	if (ptr == nullptr) {
		return;
	}

	DebugAllocateAPI* api = (DebugAllocateAPI*)ctx;
	uint8_t* q = (uint8_t*)ptr - 2 * 8;
	size_t numberByte;

	numberByte = read_size_t(q);
	numberByte += 24;
	memset(q, 0xDD, numberByte);
	api->alloc.free(api->alloc.ctx, q);

}

void* alifMem_debug_raw_realloc(void* ctx, void* ptr, size_t nByte) {

	if (ptr == nullptr) {
		return alifMem_debug_raw_alloc(0, ctx, nByte);
	}

	DebugAllocateAPI* api = (DebugAllocateAPI*)ctx;
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
	if (nByte > (size_t)9223372036854775807i64 - 24) {
		return nullptr;
	}
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
	r = (uint8_t*)api->alloc.realloc(api->alloc.ctx, head, total);

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
