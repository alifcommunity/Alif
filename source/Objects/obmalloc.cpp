

#include "alif.h"
#include "alifCore_code.h"
//#include "alifCore_object.h"  
#include "alifCore_obmalloc.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifMem.h"
#include "alifCore_alifState.h" 

//#include <stdlib.h>               // malloc()
#include <stdbool.h>


#undef  uint
#define uint alifMemUint

//extern void alifMem_dumpTraceback(int fd, const void* ptr);

//static void alifObject_debugDumpAddress(const void* p);
//static void alifMem_debugCheckAddress(const char* func, char apiID, const void* p);



static void setUp_debugHooksDomainUnlocked(AlifMemAllocatorDomain domain);
static void setUp_debugHooksUnlocked(void);
static void get_allocator_unlocked(AlifMemAllocatorDomain, AlifMemAllocatorEx*);
static void set_allocator_unlocked(AlifMemAllocatorDomain, AlifMemAllocatorEx*);


/***************************************/
/* low-level allocator implementations */
/***************************************/

/* the default raw allocator (wraps malloc) */


void* alifMem_rawMalloc(void* ALIF_UNUSED(ctx), size_t size)
{

	if (size == 0)
		size = 1;
	return malloc(size);
}





void* alifMem_rawCalloc(void* ALIF_UNUSED(ctx), size_t nelem, size_t elsize)
{
	if (nelem == 0 || elsize == 0) {
		nelem = 1;
		elsize = 1;
	}
	return calloc(nelem, elsize);
}






void* alifMem_rawRealloc(void* ALIF_UNUSED(ctx), void* ptr, size_t size)
{
	if (size == 0)
		size = 1;
	return realloc(ptr, size);
}


void alifMem_rawFree(void* ALIF_UNUSED(ctx), void* ptr)
{
	free(ptr);
}

#define MALLOC_ALLOC {nullptr, alifMem_rawMalloc, alifMem_rawCalloc, alifMem_rawRealloc, alifMem_rawFree}
#define ALIFRAW_ALLOC MALLOC_ALLOC





#ifdef WITH_ALIFMALLOC
void* _alifObject_malloc(void* ctx, size_t size);
void* _alifObject_calloc(void* ctx, size_t nElem, size_t elSize);
void  _alifObject_free(void* ctx, void* p);
void* _alifObject_realloc(void* ctx, void* ptr, size_t size);
#  define ALIFMALLOC_ALLOC {nullptr, _alifObject_malloc, _alifObject_calloc, _alifObject_realloc, _alifObject_free}
#  define ALIFOBJ_ALLOC ALIFMALLOC_ALLOC
#else
#define ALIFOBJ_ALLOC MALLOC_ALLOC
#endif

#define ALIFMEM_ALLOC ALIFOBJ_ALLOC





void* _alifMem_debugRawMalloc(void* ctx, size_t size);
void* _alifMem_debugRawCalloc(void* ctx, size_t nElem, size_t elSize);
void* _alifMem_debugRawRealloc(void* ctx, void* ptr, size_t size);
void  _alifMem_debugRawFree(void* ctx, void* ptr);

void* _alifMem_debugMalloc(void* ctx, size_t size);
void* _alifMem_debugCalloc(void* ctx, size_t nElem, size_t elSize);
void* _alifMem_debugRealloc(void* ctx, void* ptr, size_t size);
void _alifMem_debugFree(void* ctx, void* p);

#define ALIFDBGRAW_ALLOC \
    {&alifRuntime.allocators.debug.raw, _alifMem_debugRawMalloc, _alifMem_debugRawCalloc, _alifMem_debugRawRealloc, _alifMem_debugRawFree}
#define ALIFDBGMEM_ALLOC \
    {&alifRuntime.allocators.debug.mem, _alifMem_debugMalloc, _alifMem_debugCalloc, _alifMem_debugRealloc, _alifMem_debugFree}
#define ALIFDBGOBJ_ALLOC \
    {&alifRuntime.allocators.debug.obj, _alifMem_debugMalloc, _alifMem_debugCalloc, _alifMem_debugRealloc, _alifMem_debugFree}



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


/*******************************************/
/* end low-level allocator implementations */
/*******************************************/







#if defined(__has_feature)  /* Clang */
#  if __has_feature(address_sanitizer) /* is ASAN enabled? */
#    define ALIF_NO_SANITIZE_ADDRESS \
        __attribute__((no_sanitize("address")))
#  endif
#  if __has_feature(thread_sanitizer)  /* is TSAN enabled? */
#    define ALIF_NO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#  endif
#  if __has_feature(memory_sanitizer)  /* is MSAN enabled? */
#    define ALIF_NO_SANITIZE_MEMORY __attribute__((no_sanitize_memory))
#  endif
#elif defined(__GNUC__)
#  if defined(__SANITIZE_ADDRESS__)    /* GCC 4.8+, is ASAN enabled? */
#    define ALIF_NO_SANITIZE_ADDRESS \
        __attribute__((no_sanitize_address))
#  endif
// TSAN is supported since GCC 5.1, but __SANITIZE_THREAD__ macro
// is provided only since GCC 7.
#  if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1)
#    define ALIF_NO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#  endif
#endif

typedef class ObmallocState MemoryState;
// هذه تعاريف خاصة ب مكتشف الاخطاء في الذاكرة يسمى النظام ب sanitizers memory
// يعمل هذال النظام على اكتشاف اخطاء مثل كتابة عنوان فوق عنوان مكتوب مسبقا خاص بنظام الجهاز 
#ifndef ALIF_NO_SANITIZE_ADDRESS
#  define ALIF_NO_SANITIZE_ADDRESS
#endif
#ifndef ALIF_NO_SANITIZE_THREAD
#  define ALIF_NO_SANITIZE_THREAD
#endif
#ifndef ALIF_NO_SANITIZE_MEMORY
#  define ALIF_NO_SANITIZE_MEMORY
#endif
#define ALLOCATORS_MUTEX (alifRuntime.allocators.mutex)
#define ALIFMEM_RAW (alifRuntime.allocators.standard.raw)
#define ALIFMEM (alifRuntime.allocators.standard.mem)
#define ALIFOBJECT (alifRuntime.allocators.standard.obj)
#define ALIFMEM_DEBUG (alifRuntime.allocators.debug)
#define ALIFOBJECT_ARENA (alifRuntime.allocators.objArena)
/***************************/
/* managing the allocators */
/***************************/


int set_defaultAlloatorUnlocked(AlifMemAllocatorDomain _domain, int _debug, AlifMemAllocatorEx* _oldAlloc) {

	if (_oldAlloc != nullptr) {
		get_allocator_unlocked(_domain, _oldAlloc);
	}

	AlifMemAllocatorEx newAlloc;
	switch (_domain)
	{
	case AlifMem_Domain_Raw: newAlloc = ALIFRAW_ALLOC;
		break;
	case AlifMem_Domain_Mem: newAlloc = ALIFMEM_ALLOC;
		break;
	case AlifMem_Domain_Obj: newAlloc = ALIFOBJ_ALLOC;
		break;
	default:
		return -1;
	}

	get_allocator_unlocked(_domain, &newAlloc);

	if (_debug) {
		setUp_debugHooksDomainUnlocked(_domain);
	}
	return 0;
}







#ifdef ALIF_DEBUG
static const int alifDebug = 1;
#else
static const int alifDebug = 0;
#endif

//int alifMem_setDefaultAllocator(AlifMemAllocatorDomain _domain, AlifMemAllocatorEx* _oldAlloc)
//{
//	if (ALLOCATORS_MUTEX == nullptr) {
//		return set_defaultAlloatorUnlocked(_domain, alifDebug, _oldAlloc);
//	}
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	int res = set_defaultAlloatorUnlocked(_domain, alifDebug, _oldAlloc);
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//	return res;
//
//}








































static int setUp_allocatorsUnlocked(AlifMemAllocatorName allocator)
{
	switch (allocator) {
	case AlifMem_Allocator_Not_Set:
		/* do nothing */
		break;

	case AlifMem_Allocator_Default:
		(void)set_defaultAlloatorUnlocked(AlifMem_Domain_Raw, alifDebug, nullptr);
		(void)set_defaultAlloatorUnlocked(AlifMem_Domain_Mem, alifDebug, nullptr);
		(void)set_defaultAlloatorUnlocked(AlifMem_Domain_Obj, alifDebug, nullptr);
		break;

	case AlifMem_Allocator_Debug:
		(void)set_defaultAlloatorUnlocked(AlifMem_Domain_Raw, 1, nullptr);
		(void)set_defaultAlloatorUnlocked(AlifMem_Domain_Mem, 1, nullptr);
		(void)set_defaultAlloatorUnlocked(AlifMem_Domain_Obj, 1, nullptr);
		break;

#ifdef WITH_ALIFMALLOC
	case AlifMem_Allocator_AlifMalloc:
	case AlifMem_Allocator_AlifMalloc_Debug:
	{
		AlifMemAllocatorEx mallocAlloc = MALLOC_ALLOC;
		set_allocator_unlocked(AlifMem_Domain_Raw, &mallocAlloc);

		AlifMemAllocatorEx alifMalloc = ALIFMALLOC_ALLOC;
		set_allocator_unlocked(AlifMem_Domain_Mem, &alifMalloc);
		set_allocator_unlocked(AlifMem_Domain_Obj, &alifMalloc);

		if (allocator == AlifMem_Allocator_AlifMalloc_Debug) {
			setUp_debugHooksUnlocked();
		}
		break;
	}
#endif

	case AlifMem_Allocator_Malloc:
	case AlifMem_Allocator_Malloc_Debug:
	{
		AlifMemAllocatorEx mallocAlloc = MALLOC_ALLOC;
		set_allocator_unlocked(AlifMem_Domain_Raw, &mallocAlloc);
		set_allocator_unlocked(AlifMem_Domain_Mem, &mallocAlloc);
		set_allocator_unlocked(AlifMem_Domain_Obj, &mallocAlloc);

		if (allocator == AlifMem_Allocator_Malloc_Debug) {
			setUp_debugHooksUnlocked();
		}
		break;
	}

	default:
		/* unknown allocator */
		return -1;
	}

	return 0;
}


//int alifMem_setupAllocators(AlifMemAllocatorName allocator)
//{
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	int res = setUp_allocatorsUnlocked(allocator);
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//	return res;
//}


static int alifMemAllocator_eq(AlifMemAllocatorEx* a, AlifMemAllocatorEx* b)
{
	return (memcmp(a, b, sizeof(AlifMemAllocatorEx)) == 0);
}


static const char* getCurrent_allocatorName_unlocked(void)
{
	AlifMemAllocatorEx mallocAlloc = MALLOC_ALLOC;
#ifdef WITH_ALIFMALLOC
	AlifMemAllocatorEx alifMalloc = ALIFMALLOC_ALLOC;
#endif

	if (alifMemAllocator_eq(&ALIFMEM_RAW, &mallocAlloc) &&
		alifMemAllocator_eq(&ALIFMEM, &mallocAlloc) &&
		alifMemAllocator_eq(&ALIFOBJECT, &mallocAlloc))
	{
		return "malloc";
	}
#ifdef WITH_ALIFMALLOC
	if (alifMemAllocator_eq(&ALIFMEM_RAW, &mallocAlloc) &&
		alifMemAllocator_eq(&ALIFMEM, &alifMalloc) &&
		alifMemAllocator_eq(&ALIFOBJECT, &alifMalloc))
	{
		return "alifmalloc";
	}
#endif

	AlifMemAllocatorEx dbg_raw = ALIFDBGRAW_ALLOC;
	AlifMemAllocatorEx dbg_mem = ALIFDBGMEM_ALLOC;
	AlifMemAllocatorEx dbg_obj = ALIFDBGOBJ_ALLOC;

	if (alifMemAllocator_eq(&ALIFMEM_RAW, &dbg_raw) &&
		alifMemAllocator_eq(&ALIFMEM, &dbg_mem) &&
		alifMemAllocator_eq(&ALIFOBJECT, &dbg_obj))
	{
		/* Debug hooks installed */
		if (alifMemAllocator_eq(&ALIFMEM_DEBUG.raw.alloc, &mallocAlloc) &&
			alifMemAllocator_eq(&ALIFMEM_DEBUG.mem.alloc, &mallocAlloc) &&
			alifMemAllocator_eq(&ALIFMEM_DEBUG.obj.alloc, &mallocAlloc))
		{
			return "malloc_debug";
		}
#ifdef WITH_ALIFMALLOC
		if (alifMemAllocator_eq(&ALIFMEM_DEBUG.raw.alloc, &mallocAlloc) &&
			alifMemAllocator_eq(&ALIFMEM_DEBUG.mem.alloc, &alifMalloc) &&
			alifMemAllocator_eq(&ALIFMEM_DEBUG.obj.alloc, &alifMalloc))
		{
			return "alifmalloc_debug";
		}
#endif
	}
	return nullptr;
}



//const char* alifMem_getCurrentAllocatorName()
//{
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	const char* name = getCurrent_allocatorName_unlocked();
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//	return name;
//}



#ifdef WITH_ALIFMALLOC
static int alifMem_debugEnabled()
{
	return (ALIFOBJECT.malloc == _alifMem_debugMalloc);
}



static int alifMem_alifMallocEnabled(void)
{
	if (alifMem_debugEnabled()) {
		return (ALIFMEM_DEBUG.obj.alloc.malloc == _alifObject_malloc);
	}
	else {
		return (ALIFOBJECT.malloc == _alifObject_malloc);
	}
}
#endif


void setUp_debugHooksDomainUnlocked(AlifMemAllocatorDomain domain)
{
	AlifMemAllocatorEx alloc;

	if (domain == AlifMem_Domain_Raw) {
		if (ALIFMEM_RAW.malloc == _alifMem_debugRawMalloc) {
			return;
		}

		get_allocator_unlocked(domain, &ALIFMEM_DEBUG.raw.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.raw;
		alloc.malloc = _alifMem_debugRawMalloc;
		alloc.calloc = _alifMem_debugRawCalloc;
		alloc.realloc = _alifMem_debugRawRealloc;
		alloc.free = _alifMem_debugRawFree;
		set_allocator_unlocked(domain, &alloc);
	}
	else if (domain == AlifMem_Domain_Mem) {
		if (ALIFMEM.malloc == _alifMem_debugRawMalloc) {
			return;
		}

		get_allocator_unlocked(domain, &ALIFMEM_DEBUG.mem.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.mem;
		alloc.malloc = _alifMem_debugRawMalloc;
		alloc.calloc = _alifMem_debugRawCalloc;
		alloc.realloc = _alifMem_debugRawRealloc;
		alloc.free = _alifMem_debugRawFree;
		set_allocator_unlocked(domain, &alloc);
	}
	else if (domain == AlifMem_Domain_Obj) {
		if (ALIFOBJECT.malloc == _alifMem_debugRawMalloc) {
			return;
		}

		get_allocator_unlocked(domain, &ALIFMEM_DEBUG.obj.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.obj;
		alloc.malloc = _alifMem_debugRawMalloc;
		alloc.calloc = _alifMem_debugRawCalloc;
		alloc.realloc = _alifMem_debugRawRealloc;
		alloc.free = _alifMem_debugRawFree;
		set_allocator_unlocked(domain, &alloc);
	}
}



static void setUp_debugHooksUnlocked()
{
	setUp_debugHooksDomainUnlocked(AlifMem_Domain_Raw);
	setUp_debugHooksDomainUnlocked(AlifMem_Domain_Mem);
	setUp_debugHooksDomainUnlocked(AlifMem_Domain_Obj);
}



//void alifMem_setupDebugHooks()
//{
//	if (ALLOCATORS_MUTEX == nullptr) {
//
//		setUp_debugHooksUnlocked();
//		return;
//	}
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	setUp_debugHooksUnlocked();
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//}

static void get_allocator_unlocked(AlifMemAllocatorDomain _domain, AlifMemAllocatorEx* _allocator)
{
	switch (_domain)
	{
	case AlifMem_Domain_Raw: *_allocator = ALIFMEM_RAW; break;
	case AlifMem_Domain_Mem: *_allocator = ALIFMEM; break;
	case AlifMem_Domain_Obj: *_allocator = ALIFOBJECT; break;
	default:
		_allocator->ctx = nullptr;
		_allocator->malloc = nullptr;
		_allocator->calloc = nullptr;
		_allocator->realloc = nullptr;
		_allocator->free = nullptr;
	}

}

static void set_allocator_unlocked(AlifMemAllocatorDomain domain, AlifMemAllocatorEx* allocator)
{

	switch (domain)
	{
	case AlifMem_Domain_Raw: ALIFMEM_RAW = *allocator; break;
	case AlifMem_Domain_Mem: ALIFMEM = *allocator; break;
	case AlifMem_Domain_Obj: ALIFOBJECT = *allocator; break;
	}

}


//void alifMem_getAllocator(AlifMemAllocatorDomain domain, AlifMemAllocatorEx* allocator)
//{
//	if (ALLOCATORS_MUTEX == nullptr) {
//
//		get_allocator_unlocked(domain, allocator);
//		return;
//	}
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	get_allocator_unlocked(domain, allocator);
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//}


//void alifMem_setAllocator(AlifMemAllocatorDomain domain, AlifMemAllocatorEx* allocator)
//{
//
//	if (ALLOCATORS_MUTEX == nullptr) {
//		set_allocator_unlocked(domain, allocator);
//		return;
//	}
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	set_allocator_unlocked(domain, allocator);
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//}


//void alifObject_getArenaAllocator(AlifObjectArenaAllocator* allocator)
//{
//	if (ALLOCATORS_MUTEX == nullptr) {
//
//		*allocator = ALIFOBJECT_ARENA;
//		return;
//	}
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	*allocator = ALIFOBJECT_ARENA;
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//}


//void alifObject_setArenaAllocator(AlifObjectArenaAllocator* allocator)
//{
//	if (ALLOCATORS_MUTEX == nullptr) {
//
//		ALIFOBJECT_ARENA = *allocator;
//		return;
//	}
//	(void)alifThread_acquire_lock(ALLOCATORS_MUTEX, WAIT_LOCK);
//	ALIFOBJECT_ARENA = *allocator;
//	alifThread_release_lock(ALLOCATORS_MUTEX);
//}





















void* alifObject_virtualAlloc(size_t size)
{
	return ALIFOBJECT_ARENA.alloc(ALIFOBJECT_ARENA.ctx, size);
}


void alifObject_virtualFree(void* obj, size_t size)
{
	ALIFOBJECT_ARENA.free(ALIFOBJECT_ARENA.ctx, obj, size);
}



/***********************/
/* the "raw" allocator */
/***********************/


void* alifMem_rawMalloc(size_t size)
{





	if (size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFMEM_RAW.malloc(ALIFMEM_RAW.ctx, size);
}


void* alifMem_rawCalloc(size_t nElem, size_t elSize)
{

	if (elSize != 0 && nElem > (size_t)ALIFSIZE_T_MAX / elSize)
		return nullptr;
	return ALIFMEM_RAW.calloc(ALIFMEM_RAW.ctx, nElem, elSize);
}


void* alifMem_rawRealloc(void* ptr, size_t newSize)
{

	if (newSize > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFMEM_RAW.realloc(ALIFMEM_RAW.ctx, ptr, newSize);
}


void alifMem_rawFree(void* ptr)
{
	ALIFMEM_RAW.free(ALIFMEM_RAW.ctx, ptr);
}


/***********************/
/* the "mem" allocator */
/***********************/
// سيتم استخدام OBJECTSTAT_INCCOND لاحقا لكي يتم حساب عدد الكائنات المحجوزة داخل الذاكرة
void* alifMem_malloc(size_t size)
{

	if (size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	OBJECT_STAT_INC_COND(allocations512, size < 512);
	OBJECT_STAT_INC_COND(allocations4k, size >= 512 && size < 4094);
	OBJECT_STAT_INC_COND(allocations_big, size >= 4094);
	OBJECT_STAT_INC(allocations);
	return ALIFMEM.malloc(ALIFMEM.ctx, size);
}


void* alifMem_calloc(size_t nElem, size_t elSize)
{

	if (elSize != 0 && nElem > (size_t)ALIFSIZE_T_MAX / elSize)
		return nullptr;
	OBJECT_STAT_INC_COND(allocations512, elSize < 512);
	OBJECT_STAT_INC_COND(allocations4k, elSize >= 512 && elSize < 4094);
	OBJECT_STAT_INC_COND(allocations_big, elSize >= 4094);
	OBJECT_STAT_INC(allocations);
	return ALIFMEM.calloc(ALIFMEM.ctx, nElem, elSize);
}


void* alifMem_realloc(void* ptr, size_t new_size)
{

	if (new_size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFMEM.realloc(ALIFMEM.ctx, ptr, new_size);
}


void alifMem_free(void* ptr)
{
	//OBJECTSTAT_INC(frees);
	ALIFMEM.free(ALIFMEM.ctx, ptr);
}



/***************************/
/* alifMem utility functions */
/***************************/

wchar_t* alifMem_rawWcsdup(const wchar_t* str)
{


	size_t len = wcslen(str);
	if (len > (size_t)ALIFSIZE_T_MAX / sizeof(wchar_t) - 1) {
		return nullptr;
	}

	size_t size = (len + 1) * sizeof(wchar_t);
	wchar_t* str2 = (wchar_t*)alifMem_rawMalloc(size);
	if (str2 == nullptr) {
		return nullptr;
	}

	memcpy(str2, str, size);
	return str2;
}


char* alifMem_rawStrdup(const char* str)
{

	size_t size = strlen(str) + 1;
	char* copy = (char*)alifMem_rawMalloc(size);
	if (copy == nullptr) {
		return nullptr;
	}
	memcpy(copy, str, size);
	return copy;
}


char* alifMem_strdup(const char* str)
{

	size_t size = strlen(str) + 1;
	char* copy = (char*)alifMem_malloc(size);
	if (copy == nullptr) {
		return nullptr;
	}
	memcpy(copy, str, size);
	return copy;
}



/**************************/
/* the "object" allocator */
/**************************/


void* alifObject_malloc(size_t size)
{

	if (size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	OBJECT_STAT_INC_COND(allocations512, size < 512);
	OBJECT_STAT_INC_COND(allocations4k, size >= 512 && size < 4094);
	OBJECT_STAT_INC_COND(allocations_big, size >= 4094);
	OBJECT_STAT_INC(allocations);
	return ALIFOBJECT.malloc(ALIFOBJECT.ctx, size);
}


void* alifObject_calloc(size_t nelem, size_t elsize)
{

	if (elsize != 0 && nelem > (size_t)ALIFSIZE_T_MAX / elsize)
		return nullptr;
	OBJECT_STAT_INC_COND(allocations512, elSize < 512);
	OBJECT_STAT_INC_COND(allocations4k, elSize >= 512 && elSize < 4094);
	OBJECT_STAT_INC_COND(allocations_big, elSize >= 4094);
	OBJECT_STAT_INC(allocations);
	return ALIFOBJECT.calloc(ALIFOBJECT.ctx, nelem, elsize);
}


void* alifObject_realloc(void* ptr, size_t new_size)
{

	if (new_size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFOBJECT.realloc(ALIFOBJECT.ctx, ptr, new_size);
}


void alifObject_free(void* ptr)
{
	//OBJECTSTAT_INC(frees);
	ALIFOBJECT.free(ALIFOBJECT.ctx, ptr);
}


/* If we're using GCC, use __builtin_expect() to reduce overhead of
   the valgrind checks */
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#  define UNLIKELY(value) __builtin_expect((value), 0)
#  define LIKELY(value) __builtin_expect((value), 1)
#else
#  define UNLIKELY(value) (value)
#  define LIKELY(value) (value)
#endif

#ifdef WITH_ALIFMALLOC

#ifdef WITH_VALGRIND
#include <valgrind/valgrind.h>

   /* -1 indicates that we haven't checked that we're running on valgrind yet. */
static int runningOnValgrind = -1;
#endif

typedef class ObmallocState OMState;

static inline int has_own_state(AlifInterpreterState* interp)
{
	return (alifIsMainInterpreter(interp) |
		!(interp->featureFlags & ALIFRTFLAGS_USEMAIN_OBMALLOC) |
		alifisMainInterpreterFinalizing(interp));
}

static inline OMState* get_state(void)
{
	AlifInterpreterState* interp = alifInterpreterState_get();
	if (!has_own_state(interp)) {
		interp = _alifInterpreterState_main();
	}
	return &interp->obmalloc;
}



// These macros all rely on a local "state" variable.
#define USEDPOOLS (state->pools.used)
#define ALLARENAS (state->mGmt.arenas)
#define MAXARENAS (state->mGmt.maxArenas)
#define UNUSED_ARENA_OBJECTS (state->mGmt.unusedArenaObjects)
#define USABLE_ARENAS (state->mGmt.usableArenas)
#define NFP2LASTA (state->mGmt.nFP2Lasta)
#define NARENAS_CURRENTLY_ALLOCATED (state->mGmt.nArenasCurrentlyAllocated)
#define NTIMES_ARENA_ALLOCATED (state->mGmt.nTimesArenaAllocated)
#define NARENAS_HIGHWATER (state->mGmt.nArenasHighwater)
#define RAW_ALLOCATED_BLOCKS (state->mGmt.rawAllocatedBlocks)

AlifSizeT alifInterpreterState_getAllocatedBlocks(AlifInterpreterState* interp)
{
#ifdef ALIF_DEBUG

#else
	if (!has_own_state(interp)) {
		// error "المفسر لا يملك ذاكرة داخليه"
	}
#endif
	OMState* state = &interp->obmalloc;

	AlifSizeT n = RAW_ALLOCATED_BLOCKS;
	/* add up allocated blocks for used pools */
	for (uint i = 0; i < MAXARENAS; ++i) {
		/* Skip arenas which are not allocated. */
		if (ALLARENAS[i].address == 0) {
			continue;
		}

		uintptr_t base = (uintptr_t)ALIFALIGN_UP(ALLARENAS[i].address, POOL_SIZE);



		for (; base < (uintptr_t)ALLARENAS[i].poolAddress; base += POOL_SIZE) {
			PoolP p = (PoolP)base;
			n += p->ref.count;
		}
	}
	return n;
}



void alifInterpreterState_finalizeAllocatedBlocks(AlifInterpreterState* interp)
{
	if (has_own_state(interp)) {
		AlifSizeT leaked = alifInterpreterState_getAllocatedBlocks(interp);

		interp->runtime->obmalloc.interpreterLeaks += leaked;
	}
}


//static AlifSizeT getNumGlobal_allocatedBlocks(AlifRuntimeState*);
//
//
//
//
//
//static AlifSizeT lastFinalLeaks = 0;
//
//void alifFinalizeAllocatedBlocks(AlifRuntimeState* runtime)
//{
//	lastFinalLeaks = getNumGlobal_allocatedBlocks(runtime);
//	runtime->obmalloc.interpreterLeaks = 0;
//}
//
//
//static AlifSizeT getNumGlobal_allocatedBlocks(AlifRuntimeState* runtime)
//{
//	AlifSizeT total = 0;
//	if (alifRuntimeState_getFinalizing(runtime) != nullptr) {
//		AlifInterpreterState* interp = _alifInterpreterState_main();
//		if (interp == nullptr) {
//
//
//
//
//
//		}
//		else {
//
//
//
//			total += alifInterpreterState_getAllocatedBlocks(interp);
//		}
//	}
//	else {
//		HEAD_LOCK(runtime);
//		AlifInterpreterState* interp = alifInterpreterState_head();
//
//#ifdef ALIF_DEBUG
//		int gotMain = 0;
//#endif
//		for (; interp != nullptr; interp = alifInterpreterState_next(interp)) {
//#ifdef ALIF_DEBUG
//			if (interp == alifInterpreterState_main()) {
//
//				gotMain = 1;
//
//			}
//#endif
//			if (has_own_state(interp)) {
//				total += alifInterpreterState_getAllocatedBlocks(interp);
//			}
//		}
//		HEAD_UNLOCK(runtime);
//#ifdef ALIF_DEBUG
//
//#endif
//	}
//	total += runtime->obmalloc.interpreterLeaks;
//	total += lastFinalLeaks;
//	return total;
//}


//AlifSizeT alifGetGlobalAllocatedBlocks(void)
//{
//	return getNumGlobal_allocatedBlocks(&alifRuntime);
//}

#if WITH_ALIFMALLOC_RADIXTREE
/*==========================================================================*/
/* radix tree for tracking arena usage. */

#define ARENA_MAP_ROOT (state->usage.arenaMapRoot)
#ifdef USE_INTERIOR_NODES
#define ARENA_MAP_MIDCOUNT (state->usage.arenaMapMidCount)
#define ARENA_MAP_BOTCOUNT (state->usage.arenaMapBotCount)
#endif




static inline ALIF_ALWAYS_INLINE ArenaMapBotT* arena_map_get(OMState* state, AlifMemBlock* p, int create)
{
#ifdef USE_INTERIOR_NODES
	/* sanity check that IGNORE_BITS is correct */

	int i1 = MAP_TOP_INDEX(p);
	if (ARENA_MAP_ROOT.ptrs[i1] == nullptr) {
		if (!create) {
			return nullptr;
		}
		ArenaMapMidT* n = (ArenaMapMidT*)alifMem_rawCalloc(1, sizeof(ArenaMapMidT));
		if (n == nullptr) {
			return nullptr;
		}
		ARENA_MAP_ROOT.ptrs[i1] = n;
		ARENA_MAP_MIDCOUNT++;
	}
	int i2 = MAP_MID_INDEX(p);
	if (ARENA_MAP_ROOT.ptrs[i1]->ptrs[i2] == nullptr) {
		if (!create) {
			return nullptr;
		}
		ArenaMapBotT* n = (ArenaMapBotT*)alifMem_rawCalloc(1, sizeof(ArenaMapBotT));
		if (n == nullptr) {
			return nullptr;
		}
		ARENA_MAP_ROOT.ptrs[i1]->ptrs[i2] = n;
		ARENA_MAP_BOTCOUNT++;
	}
	return ARENA_MAP_ROOT.ptrs[i1]->ptrs[i2];
#else
	return &ARENA_MAP_ROOT;
#endif
}



























static int arena_mapMark_used(OMState* state, uintptr_t arenaBase, int isUsed)
{


	ArenaMapBotT* nHi = arena_map_get(
		state, (AlifMemBlock*)arenaBase, isUsed);
	if (nHi == NULL) {

		return 0;

	}
	int i3 = MAP_BOT_INDEX((AlifMemBlock*)arenaBase);
	int32_t tail = (int32_t)(arenaBase & ARENA_SIZE_MASK);
	if (tail == 0) {

		nHi->arenas[i3].tailHi = isUsed ? -1 : 0;
	}
	else {







		nHi->arenas[i3].tailHi = isUsed ? tail : 0;
		uintptr_t arenaBaseNext = arenaBase + ARENA_SIZE;





		ArenaMapBotT* nLo = arena_map_get(
			state, (AlifMemBlock*)arenaBaseNext, isUsed);
		if (nLo == NULL) {

			nHi->arenas[i3].tailHi = 0;
			return 0; /* failed to allocate space for node */
		}
		int i3_next = MAP_BOT_INDEX(arenaBaseNext);
		nLo->arenas[i3_next].tailLo = isUsed ? tail : 0;
	}
	return 1;
}



static int arena_mapIs_used(OMState* state, AlifMemBlock* p)
{
	ArenaMapBotT* n = arena_map_get(state, p, 0);
	if (n == NULL) {
		return 0;
	}
	int i3 = MAP_BOT_INDEX(p);

	int32_t hi = n->arenas[i3].tailHi;
	int32_t lo = n->arenas[i3].tailLo;
	int32_t tail = (int32_t)(AS_UINT(p) & ARENA_SIZE_MASK);
	return (tail < lo) || (tail >= hi && hi != 0);
}



#endif







static ArenaObject* new_arena(OMState* state)
{
	ArenaObject* arenaObj;
	uint excess;      
	void* address;

	int debugStats = alifRuntime.obmalloc.dumpDebugStats;
	if (debugStats == -1) {
		//const char* opt = ALIF_GETENV("ALIFTHONMALLOCSTATS");
		//debugStats = (opt != NULL && *opt != '\0');
		alifRuntime.obmalloc.dumpDebugStats = debugStats;
	}
	if (debugStats) {
		//alifObject_DebugMallocStats(stderr);
	}

	if (UNUSED_ARENA_OBJECTS == NULL) {
		uint i;
		uint numArenas;
		size_t nBytes;




		numArenas = MAXARENAS ? MAXARENAS << 1 : INITIAL_ARENA_OBJECTS;
		if (numArenas <= MAXARENAS)
			return NULL;                /* overflow */
#if SIZEOF_SIZE_T <= SIZEOF_INT
		if (numArenas > SIZE_MAX / sizeof(*ALLARENAS))
			return NULL;                /* overflow */
#endif
		nBytes = numArenas * sizeof(*ALLARENAS);
		arenaObj = (class ArenaObject*)alifMem_rawRealloc(ALLARENAS, nBytes);
		if (arenaObj == NULL)
			return NULL;
		ALLARENAS = arenaObj;











		for (i = MAXARENAS; i < numArenas; ++i) {
			ALLARENAS[i].address = 0;              /* mark as unassociated */
			ALLARENAS[i].nextArena = i < numArenas - 1 ?
				&ALLARENAS[i + 1] : NULL;
		}

		/* Update globals. */
		UNUSED_ARENA_OBJECTS = &ALLARENAS[MAXARENAS];
		MAXARENAS = numArenas;
	}

	/* Take the next available arena object off the head of the list. */

	arenaObj = UNUSED_ARENA_OBJECTS;
	UNUSED_ARENA_OBJECTS = arenaObj->nextArena;

	address = ALIFOBJECT_ARENA.alloc(ALIFOBJECT_ARENA.ctx, ARENA_SIZE);
#if WITH_ALIFMALLOC_RADIXTREE
	if (address != NULL) {
		if (!arena_mapMark_used(state, (uintptr_t)address, 1)) {

			ALIFOBJECT_ARENA.free(ALIFOBJECT_ARENA.ctx, address, ARENA_SIZE);
			address = NULL;
		}
	}
#endif
	if (address == NULL) {



		arenaObj->nextArena = UNUSED_ARENA_OBJECTS;
		UNUSED_ARENA_OBJECTS = arenaObj;
		return NULL;
	}
	arenaObj->address = (uintptr_t)address;

	++NARENAS_CURRENTLY_ALLOCATED;
	++NTIMES_ARENA_ALLOCATED;
	if (NARENAS_CURRENTLY_ALLOCATED > NARENAS_HIGHWATER)
		NARENAS_HIGHWATER = NARENAS_CURRENTLY_ALLOCATED;
	arenaObj->freePools = NULL;


	arenaObj->poolAddress = (AlifMemBlock*)arenaObj->address;
	arenaObj->nFreePools = MAX_POOLS_INARENA;
	excess = (uint)(arenaObj->address & POOL_SIZE_MASK);
	if (excess != 0) {
		--arenaObj->nFreePools;
		arenaObj->poolAddress += POOL_SIZE - excess;
	}
	arenaObj->nTotalPools = arenaObj->nFreePools;

	return arenaObj;
}




#if WITH_ALIFMALLOC_RADIXTREE



static bool address_in_range(OMState* state, void* p, PoolP ALIF_UNUSED(pool))
{
	return arena_mapIs_used(state, (AlifMemBlock*)p);
}
#else












































































static bool ALIFNO_SANITIZE_ADDRESS
ALIFNO_SANITIZE_THREAD
ALIFNO_SANITIZE_MEMORY
address_in_range(OMState* state, void* p, poolp pool)
{





	uint arenaIndex = *((volatile uint*)&pool->arenaIndex);
	return arenaindex < maxArenas &&
		(uintptr_t)p - ALLARENAS[arenaIndex].address < ARENA_SIZE &&
		ALLARENAS[arenaIndex].address != 0;
}

#endif 





static void alifMalloc_pool_extend(PoolP pool, uint size)
{
	if (UNLIKELY(pool->nextOffset <= pool->maxNextOffset)) {
		/* There is room for another block. */
		pool->freeBlock = (AlifMemBlock*)pool + pool->nextOffset;
		pool->nextOffset += INDEX2SIZE(size);
		*(AlifMemBlock**)(pool->freeBlock) = NULL;
		return;
	}

	/* Pool is full, unlink from used pools. */
	PoolP next;
	next = pool->nextPool;
	pool = pool->prevPool;
	next->prevPool = pool;
	pool->nextPool = next;
}





static void* allocate_fromNew_pool(OMState* state, uint size)
{



	if (UNLIKELY(USABLE_ARENAS == NULL)) {

#ifdef WITH_MEMORY_LIMITS
		if (NARENAS_CURRENTLY_ALLOCATED >= MAX_ARENAS) {
			return NULL;
		}
#endif
		USABLE_ARENAS = new_arena(state);
		if (USABLE_ARENAS == NULL) {
			return NULL;
		}
		USABLE_ARENAS->nextArena = USABLE_ARENAS->prevArena = NULL;

		NFP2LASTA[USABLE_ARENAS->nFreePools] = USABLE_ARENAS;
	}








	if (NFP2LASTA[USABLE_ARENAS->nFreePools] == USABLE_ARENAS) {
		/* It's the last of this size, so there won't be any. */
		NFP2LASTA[USABLE_ARENAS->nFreePools] = NULL;
	}
	/* If any free pools will remain, it will be the new smallest. */
	if (USABLE_ARENAS->nFreePools > 1) {

		NFP2LASTA[USABLE_ARENAS->nFreePools - 1] = USABLE_ARENAS;
	}

	/* Try to get a cached free pool. */
	PoolP pool = USABLE_ARENAS->freePools;
	if (LIKELY(pool != NULL)) {
		/* Unlink from cached pools. */
		USABLE_ARENAS->freePools = pool->nextPool;
		USABLE_ARENAS->nFreePools--;
		if (UNLIKELY(USABLE_ARENAS->nFreePools == 0)) {
			/* Wholly allocated:  remove. */




			USABLE_ARENAS = USABLE_ARENAS->nextArena;
			if (USABLE_ARENAS != NULL) {
				USABLE_ARENAS->prevArena = NULL;

			}
		}











	}
	else {



		pool = (PoolP)USABLE_ARENAS->poolAddress;


		pool->arenaIndex = (uint)(USABLE_ARENAS - ALLARENAS);

		pool->szidx = DUMMY_SIZE_IDX;
		USABLE_ARENAS->poolAddress += POOL_SIZE;
		--USABLE_ARENAS->nFreePools;

		if (USABLE_ARENAS->nFreePools == 0) {



			USABLE_ARENAS = USABLE_ARENAS->nextArena;
			if (USABLE_ARENAS != NULL) {
				USABLE_ARENAS->prevArena = NULL;

			}
		}
	}


	AlifMemBlock* bp;
	PoolP next = USEDPOOLS[size + size]; /* == prev */
	pool->nextPool = next;
	pool->prevPool = next;
	next->nextPool = pool;
	next->prevPool = pool;
	pool->ref.count = 1;
	if (pool->szidx == size) {




		bp = pool->freeBlock;

		pool->freeBlock = *(AlifMemBlock**)bp;
		return bp;
	}





	pool->szidx = size;
	size = INDEX2SIZE(size);
	bp = (AlifMemBlock*)pool + POOL_OVERHEAD;
	pool->nextOffset = POOL_OVERHEAD + (size << 1);
	pool->maxNextOffset = POOL_SIZE - size;
	pool->freeBlock = bp + size;
	*(AlifMemBlock**)(pool->freeBlock) = NULL;
	return bp;
}











static inline void* alifMalloc_alloc(OMState* state, void* ALIF_UNUSED(ctx), size_t nBytes)
{
#ifdef WITH_VALGRIND
	if (UNLIKELY(runningOnValgrind == -1)) {
		running_on_valgrind = RUNNING_ON_VALGRIND;
	}
	if (UNLIKELY(running_on_valgrind)) {
		return NULL;
	}
#endif

	if (UNLIKELY(nBytes == 0)) {
		return NULL;
	}
	if (UNLIKELY(nBytes > SMALL_REQUEST_THRESHOLD)) {
		return NULL;
	}

	uint size = (uint)(nBytes - 1) >> ALIGNMENT_SHIFT;
	PoolP pool = USEDPOOLS[size + size];
	AlifMemBlock* bp;

	if (LIKELY(pool != pool->nextPool)) {
		/*
		 * There is a used pool for this size class.
		 * Pick up the head block of its free list.
		 */
		++pool->ref.count;
		bp = pool->freeBlock;


		if (UNLIKELY((pool->freeBlock = *(AlifMemBlock**)bp) == NULL)) {
			// Reached the end of the free list, try to extend it.
			alifMalloc_pool_extend(pool, size);
		}
	}
	else {
		/* There isn't a pool of the right size class immediately
		 * available:  use a free pool.
		 */
		bp = (AlifMemBlock*)allocate_fromNew_pool(state, size);
	}

	return (void*)bp;
}



void* _alifObject_malloc(void* ctx, size_t nBytes)
{
	OMState* state = get_state();
	void* ptr = alifMalloc_alloc(state, ctx, nBytes);
	if (LIKELY(ptr != NULL)) {
		return ptr;
	}

	ptr = alifMem_rawMalloc(nBytes);
	if (ptr != NULL) {
		RAW_ALLOCATED_BLOCKS++;
	}
	return ptr;
}



void* _alifObject_calloc(void* ctx, size_t nelem, size_t elsize)
{

	size_t nBytes = nelem * elsize;

	OMState* state = get_state();
	void* ptr = alifMalloc_alloc(state, ctx, nBytes);
	if (LIKELY(ptr != NULL)) {
		memset(ptr, 0, nBytes);
		return ptr;
	}

	ptr = alifMem_rawCalloc(nelem, elsize);
	if (ptr != NULL) {
		RAW_ALLOCATED_BLOCKS++;
	}
	return ptr;
}



static void insert_to_usedpool(OMState* state, PoolP pool)
{


	uint size = pool->szidx;
	PoolP next = USEDPOOLS[size + size];
	PoolP prev = next->prevPool;

	/* insert pool before next:   prev <-> pool <-> next */
	pool->nextPool = next;
	pool->prevPool = prev;
	next->prevPool = pool;
	prev->nextPool = pool;
}


static void insert_to_freepool(OMState* state, PoolP pool)
{
	PoolP next = pool->nextPool;
	PoolP prev = pool->prevPool;
	next->prevPool = prev;
	prev->nextPool = next;




	ArenaObject* ao = &ALLARENAS[pool->arenaIndex];
	pool->nextPool = ao->freePools;
	ao->freePools = pool;
	uint nf = ao->nFreePools;




	ArenaObject* lastnf = NFP2LASTA[nf];






	if (lastnf == ao) {  
		ArenaObject* p = ao->prevArena;
		NFP2LASTA[nf] = (p != NULL && p->nFreePools == nf) ? p : NULL;
	}
	ao->nFreePools = ++nf;

















	if (nf == ao->nTotalPools && ao->nextArena != NULL) {










		if (ao->prevArena == NULL) {
			USABLE_ARENAS = ao->nextArena;


		}














		ao->nextArena = UNUSED_ARENA_OBJECTS;
		UNUSED_ARENA_OBJECTS = ao;

#if WITH_ALIFMALLOC_RADIXTREE

		arena_mapMark_used(state, ao->address, 0);
#endif


		ALIFOBJECT_ARENA.free(ALIFOBJECT_ARENA.ctx,
			(void*)ao->address, ARENA_SIZE);
		ao->address = 0;                        
		--NARENAS_CURRENTLY_ALLOCATED;

		return;
	}

	if (nf == 1) {





		ao->nextArena = USABLE_ARENAS;
		ao->prevArena = NULL;
		if (USABLE_ARENAS)
			USABLE_ARENAS->prevArena = ao;
		USABLE_ARENAS = ao;

		if (NFP2LASTA[1] == NULL) {
			NFP2LASTA[1] = ao;
		}

		return;
	}









	if (NFP2LASTA[nf] == NULL) {
		NFP2LASTA[nf] = ao;
	} 

	if (ao == lastnf) {

		return;
	}










	if (ao->prevArena != NULL) {


		ao->prevArena->nextArena = ao->nextArena;
	}
	else {


		USABLE_ARENAS = ao->nextArena;
	}
	ao->nextArena->prevArena = ao->prevArena;

	ao->prevArena = lastnf;
	ao->nextArena = lastnf->nextArena;
	if (ao->nextArena != NULL) {
		ao->nextArena->prevArena = ao;
	}
	lastnf->nextArena = ao;






}






static inline int alifMalloc_free(OMState* state, void* ALIF_UNUSED(ctx), void* p)
{


#ifdef WITH_VALGRIND
	if (UNLIKELY(running_on_valgrind > 0)) {
		return 0;
	}
#endif

	PoolP pool = POOL_ADDR(p);
	if (UNLIKELY(!address_in_range(state, p, pool))) {
		return 0;
	}









	AlifMemBlock* lastFree = pool->freeBlock;
	*(AlifMemBlock**)p = lastFree;
	pool->freeBlock = (AlifMemBlock*)p;
	pool->ref.count--;

	if (UNLIKELY(lastFree == NULL)) {






		insert_to_usedpool(state, pool);
		return 1;
	}




	if (LIKELY(pool->ref.count != 0)) {

		return 1;
	}






	insert_to_freepool(state, pool);
	return 1;
}



void _alifObject_free(void* ctx, void* p)
{

	if (p == NULL) {
		return;
	}

	OMState* state = get_state();
	if (UNLIKELY(!alifMalloc_free(state, ctx, p))) {

		alifMem_rawFree(p);
		RAW_ALLOCATED_BLOCKS--;
	}
}












static int alifMalloc_realloc(OMState* state, void* ctx,
	void** newptr_p, void* p, size_t nBytes)
{
	void* bp;
	PoolP pool;
	size_t size;



#ifdef WITH_VALGRIND

	if (UNLIKELY(running_on_valgrind > 0)) {
		return 0;
	}
#endif

	pool = POOL_ADDR(p);
	if (!address_in_range(state, p, pool)) {












		return 0;
	}


	size = INDEX2SIZE(pool->szidx);
	if (nBytes <= size) {







		if (4 * nBytes > 3 * size) {

			*newptr_p = p;
			return 1;
		}
		size = nBytes;
	}

	bp = _alifObject_malloc(ctx, nBytes);
	if (bp != NULL) {
		memcpy(bp, p, size);
		_alifObject_free(ctx, p);
	}
	*newptr_p = bp;
	return 1;
}



void* _alifObject_realloc(void* ctx, void* ptr, size_t nBytes)
{
	void* ptr2;

	if (ptr == NULL) {
		return _alifObject_malloc(ctx, nBytes);
	}

	OMState* state = get_state();
	if (alifMalloc_realloc(state, ctx, &ptr2, ptr, nBytes)) {
		return ptr2;
	}

	return alifMem_rawRealloc(ptr, nBytes);
}


#else   /* ! WITH_ALIFMALLOC */





AlifSizeT alifInterpreterState_getAllocatedBlocks(AlifInterpreterState* ALIF_UNUSED(interp))
{
	return 0;
}

//AlifSizeT alifGetGlobalAllocatedBlocks(void)
//{
//	return 0;
//}

void alifInterpreterState_finalizeAllocatedBlocks(AlifInterpreterState* ALIF_UNUSED(interp))
{
	return;
}

void alifFinalizeAllocatedBlocks(AlifRuntimeState* ALIF_UNUSED(runtime))
{
	return;
}





#endif










#ifdef ALIFMEM_DEBUG_SERIALNO
static size_t serialno = 0;     




static void bumpserialno(void)
{
	++serialno;
}
#endif

#define SST SIZEOF_SIZE_T

#ifdef ALIFMEM_DEBUG_SERIALNO
#  define ALIFMEM_DEBUGEXTRA_BYTES 4 * SST
#else
#  define ALIFMEM_DEBUGEXTRA_BYTES 3 * SST
#endif



static size_t read_size_t(const void* p)
{
	const uint8_t* q = (const uint8_t*)p;
	size_t result = *q++;
	int i;

	for (i = SST; --i > 0; ++q)
		result = (result << 8) | *q;
	return result;
}





static void write_size_t(void* p, size_t n)
{
	uint8_t* q = (uint8_t*)p + SST - 1;
	int i;

	for (i = SST; --i >= 0; --q) {
		*q = (uint8_t)(n & 0xff);
		n >>= 8;
	}
}































static void* alifMem_debugRawAlloc(int useCalloc, void* ctx, size_t nBytes)
{
	DebugAllocApiT* api = (DebugAllocApiT*)ctx;
	uint8_t* p;          
	uint8_t* data;       
	uint8_t* tail;       
	size_t total;        

	if (nBytes > (size_t)ALIFSIZE_T_MAX - ALIFMEM_DEBUGEXTRA_BYTES) {

		return NULL;
	}
	total = nBytes + ALIFMEM_DEBUGEXTRA_BYTES;












	if (useCalloc) {
		p = (uint8_t*)api->alloc.calloc(api->alloc.ctx, 1, total);
	}
	else {
		p = (uint8_t*)api->alloc.malloc(api->alloc.ctx, total);
	}
	if (p == NULL) {
		return NULL;
	}
	data = p + 2 * SST;

#ifdef ALIFMEM_DEBUG_SERIALNO
	bumpserialno();
#endif


	write_size_t(p, nBytes);
	p[SST] = (uint8_t)api->apiID;
	memset(p + SST + 1, ALIFMEM_FORBIDDENBYTE, SST - 1);

	if (nBytes > 0 && !useCalloc) {
		memset(data, ALIFMEM_CLEANBYTE, nBytes);
	}


	tail = data + nBytes;
	memset(tail, ALIFMEM_FORBIDDENBYTE, SST);
#ifdef PYMEM_DEBUG_SERIALNO
	write_size_t(tail + SST, serialno);
#endif

	return data;
}


void* _alifMem_debugRawMalloc(void* ctx, size_t nbytes)
{
	return alifMem_debugRawAlloc(0, ctx, nbytes);
}


void* _alifMem_debugRawCalloc(void* ctx, size_t nelem, size_t elsize)
{
	size_t nbytes;

	nbytes = nelem * elsize;
	return alifMem_debugRawAlloc(1, ctx, nbytes);
}








void _alifMem_debugRawFree(void* ctx, void* p)
{

	if (p == NULL) {
		return;
	}

	DebugAllocApiT* api = (DebugAllocApiT*)ctx;
	uint8_t* q = (uint8_t*)p - 2 * SST;  
	size_t nbytes;


	nbytes = read_size_t(q);
	nbytes += ALIFMEM_DEBUGEXTRA_BYTES;
	memset(q, ALIFMEM_DEADBYTE, nbytes);
	api->alloc.free(api->alloc.ctx, q);
}



void* _alifMem_debugRawRealloc(void* ctx, void* p, size_t nbytes)
{
	if (p == NULL) {
		return alifMem_debugRawAlloc(0, ctx, nbytes);
	}

	DebugAllocApiT* api = (DebugAllocApiT*)ctx;
	uint8_t* head;        
	uint8_t* data;        
	uint8_t* r;
	uint8_t* tail;        
	size_t total;         
	size_t originalNbytes;
#define ERASED_SIZE 64
	uint8_t save[2 * ERASED_SIZE]; 



	data = (uint8_t*)p;
	head = data - 2 * SST;
	originalNbytes = read_size_t(head);
	if (nbytes > (size_t)ALIFSIZE_T_MAX - ALIFMEM_DEBUGEXTRA_BYTES) {

		return NULL;
	}
	total = nbytes + ALIFMEM_DEBUGEXTRA_BYTES;

	tail = data + originalNbytes;
#ifdef ALIFMEM_DEBUG_SERIALNO
	size_t blockSerialno = read_size_t(tail + SST);
#endif



	if (originalNbytes <= sizeof(save)) {
		memcpy(save, data, originalNbytes);
		memset(data - 2 * SST, ALIFMEM_DEADBYTE,
			originalNbytes + ALIFMEM_DEBUGEXTRA_BYTES);
	}
	else {
		memcpy(save, data, ERASED_SIZE);
		memset(head, ALIFMEM_DEADBYTE, ERASED_SIZE + 2 * SST);
		memcpy(&save[ERASED_SIZE], tail - ERASED_SIZE, ERASED_SIZE);
		memset(tail - ERASED_SIZE, ALIFMEM_DEADBYTE,
			ERASED_SIZE + ALIFMEM_DEBUGEXTRA_BYTES - 2 * SST);
	}


	r = (uint8_t*)api->alloc.realloc(api->alloc.ctx, head, total);
	if (r == NULL) {


		nbytes = originalNbytes;
	}
	else {
		head = r;
#ifdef ALIFMEM_DEBUG_SERIALNO
		bumpserialno();
		blockSerialno = serialno;
#endif
	}
	data = head + 2 * SST;

	write_size_t(head, nbytes);
	head[SST] = (uint8_t)api->apiID;
	memset(head + SST + 1, ALIFMEM_FORBIDDENBYTE, SST - 1);

	tail = data + nbytes;
	memset(tail, ALIFMEM_FORBIDDENBYTE, SST);
#ifdef ALIFMEM_DEBUG_SERIALNO
	write_size_t(tail + SST, blockSerialno);
#endif


	if (originalNbytes <= sizeof(save)) {
		memcpy(data, save, ALIF_MIN(nbytes, originalNbytes));
	}
	else {
		size_t i = originalNbytes - ERASED_SIZE;
		memcpy(data, save, ALIF_MIN(nbytes, ERASED_SIZE));
		if (nbytes > i) {
			memcpy(data + i, &save[ERASED_SIZE],
				ALIF_MIN(nbytes - i, ERASED_SIZE));
		}
	}

	if (r == NULL) {
		return NULL;
	}

	if (nbytes > originalNbytes) {

		memset(data + originalNbytes, ALIFMEM_CLEANBYTE,
			nbytes - originalNbytes);
	}

	return data;
}












void* _alifMem_debugMalloc(void* ctx, size_t nbytes)
{

	return _alifMem_debugRawMalloc(ctx, nbytes);
}


void* _alifMem_debugCalloc(void* ctx, size_t nElem, size_t elSize)
{

	return _alifMem_debugRawCalloc(ctx, nElem, elSize);
}



void _alifMem_debugFree(void* ctx, void* ptr)
{

	_alifMem_debugRawFree(ctx, ptr);
}



void* _alifMem_debugRealloc(void* ctx, void* ptr, size_t nBytes)
{

	return _alifMem_debugRawRealloc(ctx, ptr, nBytes);
}
