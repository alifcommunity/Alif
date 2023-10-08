

#include "alif.h"
#include "alifCore_code.h"
//#include "alifCore_object.h"  
#include "alifCore_obmalloc.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifMem.h"
#include "alifCore_alifState.h" 
//#include <stdlib.h>               
#include <stdbool.h>


#undef  uint
#define uint alifMemUint

//extern void alifMem_dumpTraceback(int fd, const void* _ptr);

//static void alifObject_debugDumpAddress(const void* _p);
//static void alifMem_debugCheckAddress(const char* func, char apiID, const void* _p);



static void setUp_debugHooksDomainUnlocked(AlifMemAllocatorDomain);
static void setUp_debugHooksUnlocked(void);
static void get_allocator_unlocked(AlifMemAllocatorDomain, AlifMemAllocatorEx*);
static void set_allocator_unlocked(AlifMemAllocatorDomain, AlifMemAllocatorEx*);



/***************************************/
/* low-level _allocator implementations */
/***************************************/




void* _alifMem_rawMalloc(void* ALIF_UNUSED(_ctx), size_t _size)
{

	if (_size == 0)
		_size = 1;
	return malloc(_size);
}





void* _alifMem_rawCalloc(void* ALIF_UNUSED(_ctx), size_t _nElem, size_t _elSize)
{
	if (_nElem == 0 || _elSize == 0) {
		_nElem = 1;
		_elSize = 1;
	}
	return calloc(_nElem, _elSize);
}






void* _alifMem_rawRealloc(void* ALIF_UNUSED(_ctx), void* _ptr, size_t _size)
{
	if (_size == 0)
		_size = 1;
	return realloc(_ptr, _size);
}


void _alifMem_rawFree(void* ALIF_UNUSED(_ctx), void* _ptr)
{
	free(_ptr);
}

#define MALLOC_ALLOC {nullptr, _alifMem_rawMalloc, _alifMem_rawCalloc, _alifMem_rawRealloc, _alifMem_rawFree}
#define ALIFRAW_ALLOC MALLOC_ALLOC





#ifdef WITH_ALIFMALLOC
void* alifObject_malloc(void* , size_t );
void* alifObject_calloc(void* , size_t , size_t );
void  alifObject_free(void* , void* );
void* alifObject_realloc(void* , void* , size_t );
#  define ALIFMALLOC_ALLOC {nullptr, alifObject_malloc, alifObject_calloc, alifObject_realloc, alifObject_free}
#  define ALIFOBJ_ALLOC ALIFMALLOC_ALLOC
#else
#define ALIFOBJ_ALLOC MALLOC_ALLOC
#endif

#define ALIFMEM_ALLOC ALIFOBJ_ALLOC





void* alifMem_debugRawMalloc(void* ctx, size_t size);
void* alifMem_debugRawCalloc(void* ctx, size_t nElem, size_t elSize);
void* alifMem_debugRawRealloc(void* ctx, void* ptr, size_t size);
void  alifMem_debugRawFree(void* ctx, void* ptr);

void* alifMem_debugMalloc(void* ctx, size_t size);
void* alifMem_debugCalloc(void* ctx, size_t nElem, size_t elSize);
void* alifMem_debugRealloc(void* ctx, void* ptr, size_t size);
void alifMem_debugFree(void* ctx, void* p);

#define ALIFDBGRAW_ALLOC \
    {&alifRuntime.allocators.debug.raw, alifMem_debugRawMalloc, alifMem_debugRawCalloc, alifMem_debugRawRealloc, alifMem_debugRawFree}
#define ALIFDBGMEM_ALLOC \
    {&alifRuntime.allocators.debug.mem, alifMem_debugMalloc, alifMem_debugCalloc, alifMem_debugRealloc, alifMem_debugFree}
#define ALIFDBGOBJ_ALLOC \
    {&alifRuntime.allocators.debug.obj, alifMem_debugMalloc, alifMem_debugCalloc, alifMem_debugRealloc, alifMem_debugFree}



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


void* alifMem_arenaAlloc(void* _ctx, size_t _size) {

#ifdef MS_WINDOWS
	return VirtualAlloc(nullptr, _size, 0x00001000 | 0x00002000, 0x04);
#elif defined(ARENA_USE_MMAP)
	void* _ptr;
	_ptr = mmap(nullptr, _size, PORT_READ | PORT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (_ptr == MAP_FAILED) {
		return nullptr;
	}
	return _ptr;
#else
	return malloc(_size);
#endif
}




void alifMem_arenaFree(void* _ctx, void* _ptr, size_t _size)
{
#ifdef MS_WINDOWS
	VirtualFree(_ptr, 0, 0x00008000);
#elif defined(ARENA_USE_MMAP)
	munmap(_ptr, _size);
#else
	free(_ptr);
#endif
}


/*******************************************/
/* end low-level _allocator implementations */
/*******************************************/







#if defined(__has_feature) 
#  if __has_feature(address_sanitizer) 
#    define ALIFNO_SANITIZE_ADDRESS \
        __attribute__((no_sanitize("address")))
#  endif
#  if __has_feature(thread_sanitizer) 
#    define ALIFNO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#  endif
#  if __has_feature(memory_sanitizer) 
#    define ALIFNO_SANITIZE_MEMORY __attribute__((no_sanitize_memory))
#  endif
#elif defined(__GNUC__)
#  if defined(__SANITIZE_ADDRESS__)  
#    define ALIFNO_SANITIZE_ADDRESS \
        __attribute__((no_sanitize_address))
#  endif


#  if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1)
#    define ALIFNO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#  endif
#endif

typedef class ObmallocState MemoryState;
// هذه تعاريف خاصة ب مكتشف الاخطاء في الذاكرة يسمى النظام ب sanitizers memory
// يعمل هذال النظام على اكتشاف اخطاء مثل كتابة عنوان فوق عنوان مكتوب مسبقا خاص بنظام الجهاز 
#ifndef ALIFNO_SANITIZE_ADDRESS
#  define ALIFNO_SANITIZE_ADDRESS
#endif
#ifndef ALIFNO_SANITIZE_THREAD
#  define ALIFNO_SANITIZE_THREAD
#endif
#ifndef ALIFNO_SANITIZE_MEMORY
#  define ALIFNO_SANITIZE_MEMORY
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


int alifMem_setDefaultAllocator(AlifMemAllocatorDomain _domain, AlifMemAllocatorEx* _oldAlloc)
{
	if (ALLOCATORS_MUTEX == nullptr) {
		return set_defaultAlloatorUnlocked(_domain, alifDebug, _oldAlloc);
	}
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	int res = set_defaultAlloatorUnlocked(_domain, alifDebug, _oldAlloc);
	alifThread_releaseLock(ALLOCATORS_MUTEX);
	return res;

}







































static int setUp_allocatorsUnlocked(AlifMemAllocatorName _allocator)
{
	switch (_allocator) {
	case AlifMem_Allocator_NotSet:

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
	case AlifMem_Allocator_AlifMallocDebug:
	{
		AlifMemAllocatorEx mallocAlloc = MALLOC_ALLOC;
		set_allocator_unlocked(AlifMem_Domain_Raw, &mallocAlloc);

		AlifMemAllocatorEx alifMalloc = ALIFMALLOC_ALLOC;
		set_allocator_unlocked(AlifMem_Domain_Mem, &alifMalloc);
		set_allocator_unlocked(AlifMem_Domain_Obj, &alifMalloc);

		if (_allocator == AlifMem_Allocator_AlifMallocDebug) {
			setUp_debugHooksUnlocked();
		}
		break;
	}
#endif

	case AlifMem_Allocator_Malloc:
	case AlifMem_Allocator_MallocDebug:
	{
		AlifMemAllocatorEx mallocAlloc = MALLOC_ALLOC;
		set_allocator_unlocked(AlifMem_Domain_Raw, &mallocAlloc);
		set_allocator_unlocked(AlifMem_Domain_Mem, &mallocAlloc);
		set_allocator_unlocked(AlifMem_Domain_Obj, &mallocAlloc);

		if (_allocator == AlifMem_Allocator_MallocDebug) {
			setUp_debugHooksUnlocked();
		}
		break;
	}

	default:

		return -1;
	}

	return 0;
}


int alifMem_setupAllocators(AlifMemAllocatorName _allocator)
{
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	int res = setUp_allocatorsUnlocked(_allocator);
	alifThread_releaseLock(ALLOCATORS_MUTEX);
	return res;
}


static int alifMemAllocator_eq(AlifMemAllocatorEx* _a, AlifMemAllocatorEx* _b)
{
	return (memcmp(_a, _b, sizeof(AlifMemAllocatorEx)) == 0);
}


static const char* getCurrent_allocatorName_unlocked()
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



const char* alifMem_getCurrentAllocatorName()
{
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	const char* name = getCurrent_allocatorName_unlocked();
	alifThread_releaseLock(ALLOCATORS_MUTEX);
	return name;
}



#ifdef WITH_ALIFMALLOC
static int alifMem_debugEnabled()
{
	return (ALIFOBJECT.malloc == alifMem_debugMalloc);
}



static int alifMem_alifMallocEnabled()
{
	if (alifMem_debugEnabled()) {
		return (ALIFMEM_DEBUG.obj.alloc.malloc == alifObject_malloc);
	}
	else {
		return (ALIFOBJECT.malloc == alifObject_malloc);
	}
}
#endif


void setUp_debugHooksDomainUnlocked(AlifMemAllocatorDomain _domain)
{
	AlifMemAllocatorEx alloc;

	if (_domain == AlifMem_Domain_Raw) {
		if (ALIFMEM_RAW.malloc == alifMem_debugRawMalloc) {
			return;
		}

		get_allocator_unlocked(_domain, &ALIFMEM_DEBUG.raw.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.raw;
		alloc.malloc = alifMem_debugRawMalloc;
		alloc.calloc = alifMem_debugRawCalloc;
		alloc.realloc = alifMem_debugRawRealloc;
		alloc.free = alifMem_debugRawFree;
		set_allocator_unlocked(_domain, &alloc);
	}
	else if (_domain == AlifMem_Domain_Mem) {
		if (ALIFMEM.malloc == alifMem_debugRawMalloc) {
			return;
		}

		get_allocator_unlocked(_domain, &ALIFMEM_DEBUG.mem.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.mem;
		alloc.malloc = alifMem_debugRawMalloc;
		alloc.calloc = alifMem_debugRawCalloc;
		alloc.realloc = alifMem_debugRawRealloc;
		alloc.free = alifMem_debugRawFree;
		set_allocator_unlocked(_domain, &alloc);
	}
	else if (_domain == AlifMem_Domain_Obj) {
		if (ALIFOBJECT.malloc == alifMem_debugRawMalloc) {
			return;
		}

		get_allocator_unlocked(_domain, &ALIFMEM_DEBUG.obj.alloc);
		alloc.ctx = &ALIFMEM_DEBUG.obj;
		alloc.malloc = alifMem_debugRawMalloc;
		alloc.calloc = alifMem_debugRawCalloc;
		alloc.realloc = alifMem_debugRawRealloc;
		alloc.free = alifMem_debugRawFree;
		set_allocator_unlocked(_domain, &alloc);
	}
}



static void setUp_debugHooksUnlocked()
{
	setUp_debugHooksDomainUnlocked(AlifMem_Domain_Raw);
	setUp_debugHooksDomainUnlocked(AlifMem_Domain_Mem);
	setUp_debugHooksDomainUnlocked(AlifMem_Domain_Obj);
}



void alifMem_setupDebugHooks()
{
	if (ALLOCATORS_MUTEX == nullptr) {

		setUp_debugHooksUnlocked();
		return;
	}
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	setUp_debugHooksUnlocked();
	alifThread_releaseLock(ALLOCATORS_MUTEX);
}

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

static void set_allocator_unlocked(AlifMemAllocatorDomain _domain, AlifMemAllocatorEx* _allocator)
{

	switch (_domain)
	{
	case AlifMem_Domain_Raw: ALIFMEM_RAW = *_allocator; break;
	case AlifMem_Domain_Mem: ALIFMEM = *_allocator; break;
	case AlifMem_Domain_Obj: ALIFOBJECT = *_allocator; break;
	}

}


void alifMem_getAllocator(AlifMemAllocatorDomain _domain, AlifMemAllocatorEx* _allocator)
{
	if (ALLOCATORS_MUTEX == nullptr) {

		get_allocator_unlocked(_domain, _allocator);
		return;
	}
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	get_allocator_unlocked(_domain, _allocator);
	alifThread_releaseLock(ALLOCATORS_MUTEX);
}


void alifMem_setAllocator(AlifMemAllocatorDomain _domain, AlifMemAllocatorEx* _allocator)
{

	if (ALLOCATORS_MUTEX == nullptr) {
		set_allocator_unlocked(_domain, _allocator);
		return;
	}
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	set_allocator_unlocked(_domain, _allocator);
	alifThread_releaseLock(ALLOCATORS_MUTEX);
}


void alifObject_getArenaAllocator(AlifObjectArenaAllocator* _allocator)
{
	if (ALLOCATORS_MUTEX == nullptr) {

		*_allocator = ALIFOBJECT_ARENA;
		return;
	}
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	*_allocator = ALIFOBJECT_ARENA;
	alifThread_releaseLock(ALLOCATORS_MUTEX);
}


void alifObject_setArenaAllocator(AlifObjectArenaAllocator* _allocator)
{
	if (ALLOCATORS_MUTEX == nullptr) {

		ALIFOBJECT_ARENA = *_allocator;
		return;
	}
	(void)alifThread_acquireLock(ALLOCATORS_MUTEX, WAIT_LOCK);
	ALIFOBJECT_ARENA = *_allocator;
	alifThread_releaseLock(ALLOCATORS_MUTEX);
}





















void* alifObject_virtualAlloc(size_t _size)
{
	return ALIFOBJECT_ARENA.alloc(ALIFOBJECT_ARENA.ctx, _size);
}


void alifObject_virtualFree(void* _obj, size_t _size)
{
	ALIFOBJECT_ARENA.free(ALIFOBJECT_ARENA.ctx, _obj, _size);
}



/***********************/
/* the "raw" _allocator */
/***********************/


void* alifMem_rawMalloc(size_t _size)
{





	if (_size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFMEM_RAW.malloc(ALIFMEM_RAW.ctx, _size);
}


void* alifMem_rawCalloc(size_t _nElem, size_t _elSize)
{

	if (_elSize != 0 && _nElem > (size_t)ALIFSIZE_T_MAX / _elSize)
		return nullptr;
	return ALIFMEM_RAW.calloc(ALIFMEM_RAW.ctx, _nElem, _elSize);
}


void* alifMem_rawRealloc(void* _ptr, size_t _newSize)
{

	if (_newSize > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFMEM_RAW.realloc(ALIFMEM_RAW.ctx, _ptr, _newSize);
}


void alifMem_rawFree(void* _ptr)
{
	ALIFMEM_RAW.free(ALIFMEM_RAW.ctx, _ptr);
}


/***********************/
/* the "mem" _allocator */
/***********************/
// سيتم استخدام OBJECTSTAT_INCCOND لاحقا لكي يتم حساب عدد الكائنات المحجوزة داخل الذاكرة
void* alifMem_malloc(size_t _size)
{

	if (_size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	OBJECT_STATINC_COND(allocations512, _size < 512);
	OBJECT_STATINC_COND(allocations4k, _size >= 512 && _size < 4094);
	OBJECT_STATINC_COND(allocations_big, _size >= 4094);
	OBJECT_STATINC(allocations);
	return ALIFMEM.malloc(ALIFMEM.ctx, _size);
}


void* alifMem_calloc(size_t _nElem, size_t _elSize)
{

	if (_elSize != 0 && _nElem > (size_t)ALIFSIZE_T_MAX / _elSize)
		return nullptr;
	OBJECT_STATINC_COND(allocations512, _elSize < 512);
	OBJECT_STATINC_COND(allocations4k, _elSize >= 512 && _elSize < 4094);
	OBJECT_STATINC_COND(allocations_big, _elSize >= 4094);
	OBJECT_STATINC(allocations);
	return ALIFMEM.calloc(ALIFMEM.ctx, _nElem, _elSize);
}


void* alifMem_realloc(void* _ptr, size_t _newSize)
{

	if (_newSize > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFMEM.realloc(ALIFMEM.ctx, _ptr, _newSize);
}


void alifMem_free(void* _ptr)
{
	OBJECT_STATINC(frees);
	ALIFMEM.free(ALIFMEM.ctx, _ptr);
}



/***************************/
/* alifMem utility functions */
/***************************/

wchar_t* alifMem_rawWcsDup(const wchar_t* _str)
{


	size_t len = wcslen(_str);
	if (len > (size_t)ALIFSIZE_T_MAX / sizeof(wchar_t) - 1) {
		return nullptr;
	}

	size_t size = (len + 1) * sizeof(wchar_t);
	wchar_t* str2 = (wchar_t*)alifMem_rawMalloc(size);
	if (str2 == nullptr) {
		return nullptr;
	}

	memcpy(str2, _str, size);
	return str2;
}


char* alifMem_rawStrdup(const char* _str)
{

	size_t size = strlen(_str) + 1;
	char* copy = (char*)alifMem_rawMalloc(size);
	if (copy == nullptr) {
		return nullptr;
	}
	memcpy(copy, _str, size);
	return copy;
}


char* alifMem_strdup(const char* _str)
{

	size_t size = strlen(_str) + 1;
	char* copy = (char*)alifMem_malloc(size);
	if (copy == nullptr) {
		return nullptr;
	}
	memcpy(copy, _str, size);
	return copy;
}



/**************************/
/* the "object" _allocator */
/**************************/


void* alifObjectMalloc(size_t _size)
{

	if (_size > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	OBJECT_STATINC_COND(allocations512, _size < 512);
	OBJECT_STATINC_COND(allocations4k, _size >= 512 && _size < 4094);
	OBJECT_STATINC_COND(allocations_big, _size >= 4094);
	OBJECT_STATINC(allocations);
	return ALIFOBJECT.malloc(ALIFOBJECT.ctx, _size);
}


void* alifObjectCalloc(size_t _nElem, size_t _elSize)
{

	if (_elSize != 0 && _nElem > (size_t)ALIFSIZE_T_MAX / _elSize)
		return nullptr;
	OBJECT_STATINC_COND(allocations512, _elSize < 512);
	OBJECT_STATINC_COND(allocations4k, _elSize >= 512 && _elSize < 4094);
	OBJECT_STATINC_COND(allocations_big, _elSize >= 4094);
	OBJECT_STATINC(allocations);
	return ALIFOBJECT.calloc(ALIFOBJECT.ctx, _nElem, _elSize);
}


void* alif_Object_realloc(void* _ptr, size_t _newSize)
{

	if (_newSize > (size_t)ALIFSIZE_T_MAX)
		return nullptr;
	return ALIFOBJECT.realloc(ALIFOBJECT.ctx, _ptr, _newSize);
}


void alif_Object_free(void* _ptr)
{
	OBJECT_STATINC(frees);
	ALIFOBJECT.free(ALIFOBJECT.ctx, _ptr);
}




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


static int runningOnValgrind = -1;
#endif

typedef class ObmallocState OMState;

static inline int has_own_state(AlifInterpreterState* _interp)
{
	return (alif_isMainInterpreter(_interp) |
		!(_interp->featureFlags & ALIFRTFLAGS_USEMAIN_OBMALLOC) |
		alifisMainInterpreterFinalizing(_interp));
}

static inline OMState* get_state()
{
	AlifInterpreterState* interp = alifInterpreterState_get();
	if (!has_own_state(interp)) {
		interp = alifInterpreter_state_main();
	}
	return &interp->obmalloc;
}




#define USEDPOOLS (_state->pools.used)
#define ALLARENAS (_state->mGmt.arenas)
#define MAXARENAS (_state->mGmt.maxArenas)
#define UNUSED_ARENA_OBJECTS (_state->mGmt.unusedArenaObjects)
#define USABLE_ARENAS (_state->mGmt.usableArenas)
#define NFP2LASTA (_state->mGmt.nFP2Lasta)
#define NARENAS_CURRENTLY_ALLOCATED (_state->mGmt.nArenasCurrentlyAllocated)
#define NTIMES_ARENA_ALLOCATED (_state->mGmt.nTimesArenaAllocated)
#define NARENAS_HIGHWATER (_state->mGmt.nArenasHighwater)
#define RAW_ALLOCATED_BLOCKS (_state->mGmt.rawAllocatedBlocks)

AlifSizeT alifInterpreterState_getAllocatedBlocks(AlifInterpreterState* _interp)
{
#ifdef ALIF_DEBUG

#else
	if (!has_own_state(_interp)) {
		// error "المفسر لا يملك ذاكرة داخليه"
	}
#endif
	OMState* _state = &_interp->obmalloc;

	AlifSizeT n = RAW_ALLOCATED_BLOCKS;

	for (uint i = 0; i < MAXARENAS; ++i) {

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



void alifInterpreterState_finalizeAllocatedBlocks(AlifInterpreterState* _interp)
{
	if (has_own_state(_interp)) {
		AlifSizeT leaked = alifInterpreterState_getAllocatedBlocks(_interp);

		_interp->runtime->obmalloc.interpreterLeaks += leaked;
	}
}


static AlifSizeT getNumGlobal_allocatedBlocks(AlifRuntimeState*);





static AlifSizeT lastFinalLeaks = 0;

void alifFinalizeAllocatedBlocks(AlifRuntimeState* _runtime)
{
	lastFinalLeaks = getNumGlobal_allocatedBlocks(_runtime);
	_runtime->obmalloc.interpreterLeaks = 0;
}


static AlifSizeT getNumGlobal_allocatedBlocks(AlifRuntimeState* _runtime)
{
	AlifSizeT total = 0;
	if (alifRuntimeState_getFinalizing(_runtime) != nullptr) {
		AlifInterpreterState* _interp = alifInterpreter_state_main();
		if (_interp == nullptr) {





		}
		else {



			total += alifInterpreterState_getAllocatedBlocks(_interp);
		}
	}
	else {
		HEAD_LOCK(_runtime);
		AlifInterpreterState* _interp = alifInterpreterState_head();

#ifdef ALIF_DEBUG
		int gotMain = 0;
#endif
		for (; _interp != nullptr; _interp = alifInterpreterState_next(_interp)) {
#ifdef ALIF_DEBUG
			if (_interp == alifInterpreterState_main()) {

				gotMain = 1;

			}
#endif
			if (has_own_state(_interp)) {
				total += alifInterpreterState_getAllocatedBlocks(_interp);
			}
		}
		HEAD_UNLOCK(_runtime);
#ifdef ALIF_DEBUG

#endif
	}
	total += _runtime->obmalloc.interpreterLeaks;
	total += lastFinalLeaks;
	return total;
}


AlifSizeT alifGetGlobalAllocatedBlocks()
{
	return getNumGlobal_allocatedBlocks(&alifRuntime);
}

#if WITH_ALIFMALLOC_RADIXTREE



#define ARENA_MAP_ROOT (_state->usage.arenaMapRoot)
#ifdef USE_INTERIOR_NODES
#define ARENA_MAP_MIDCOUNT (_state->usage.arenaMapMidCount)
#define ARENA_MAP_BOTCOUNT (_state->usage.arenaMapBotCount)
#endif




static inline ALIF_ALWAYS_INLINE ArenaMapBotT* arena_map_get(OMState* _state, AlifMemBlock* _p, int _create)
{
#ifdef USE_INTERIOR_NODES


	int i1 = MAP_TOP_INDEX(_p);
	if (ARENA_MAP_ROOT.ptrs[i1] == nullptr) {
		if (!_create) {
			return nullptr;
		}
		ArenaMapMidT* n = (ArenaMapMidT*)alifMem_rawCalloc(1, sizeof(ArenaMapMidT));
		if (n == nullptr) {
			return nullptr;
		}
		ARENA_MAP_ROOT.ptrs[i1] = n;
		ARENA_MAP_MIDCOUNT++;
	}
	int i2 = MAP_MID_INDEX(_p);
	if (ARENA_MAP_ROOT.ptrs[i1]->ptrs[i2] == nullptr) {
		if (!_create) {
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



























static int arena_mapMark_used(OMState* _state, uintptr_t _arenaBase, int _isUsed)
{


	ArenaMapBotT* nHi = arena_map_get(
		_state, (AlifMemBlock*)_arenaBase, _isUsed);
	if (nHi == nullptr) {

		return 0;

	}
	int i3 = MAP_BOT_INDEX((AlifMemBlock*)_arenaBase);
	int32_t tail = (int32_t)(_arenaBase & ARENA_SIZE_MASK);
	if (tail == 0) {

		nHi->arenas[i3].tailHi = _isUsed ? -1 : 0;
	}
	else {







		nHi->arenas[i3].tailHi = _isUsed ? tail : 0;
		uintptr_t arenaBaseNext = _arenaBase + ARENA_SIZE;





		ArenaMapBotT* nLo = arena_map_get(
			_state, (AlifMemBlock*)arenaBaseNext, _isUsed);
		if (nLo == nullptr) {

			nHi->arenas[i3].tailHi = 0;
			return 0; /* failed to allocate space for node */
		}
		int i3_next = MAP_BOT_INDEX(arenaBaseNext);
		nLo->arenas[i3_next].tailLo = _isUsed ? tail : 0;
	}
	return 1;
}



static int arena_mapIs_used(OMState* _state, AlifMemBlock* _p)
{
	ArenaMapBotT* n = arena_map_get(_state, _p, 0);
	if (n == nullptr) {
		return 0;
	}
	int i3 = MAP_BOT_INDEX(_p);

	int32_t hi = n->arenas[i3].tailHi;
	int32_t lo = n->arenas[i3].tailLo;
	int32_t tail = (int32_t)(AS_UINT(_p) & ARENA_SIZE_MASK);
	return (tail < lo) || (tail >= hi && hi != 0);
}



#endif







static ArenaObject* new_arena(OMState* _state)
{
	ArenaObject* arenaObj;
	uint excess;      
	void* address;

	int debugStats = alifRuntime.obmalloc.dumpDebugStats;
	if (debugStats == -1) {
		//const char* opt = ALIF_GETENV("ALIFTHONMALLOCSTATS");
		//debugStats = (opt != nullptr && *opt != '\0');
		alifRuntime.obmalloc.dumpDebugStats = debugStats;
	}
	if (debugStats) {
		//alifObject_DebugMallocStats(stderr);
	}

	if (UNUSED_ARENA_OBJECTS == nullptr) {
		uint i;
		uint numArenas;
		size_t nBytes;




		numArenas = MAXARENAS ? MAXARENAS << 1 : INITIAL_ARENA_OBJECTS;
		if (numArenas <= MAXARENAS)
			return nullptr;                /* overflow */
#if SIZEOF_SIZE_T <= SIZEOF_INT
		if (numArenas > SIZE_MAX / sizeof(*ALLARENAS))
			return nullptr;                /* overflow */
#endif
		nBytes = numArenas * sizeof(*ALLARENAS);
		arenaObj = (class ArenaObject*)alifMem_rawRealloc(ALLARENAS, nBytes);
		if (arenaObj == nullptr)
			return nullptr;
		ALLARENAS = arenaObj;











		for (i = MAXARENAS; i < numArenas; ++i) {
			ALLARENAS[i].address = 0;              /* mark as unassociated */
			ALLARENAS[i].nextArena = i < numArenas - 1 ?
				&ALLARENAS[i + 1] : nullptr;
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
	if (address != nullptr) {
		if (!arena_mapMark_used(_state, (uintptr_t)address, 1)) {

			ALIFOBJECT_ARENA.free(ALIFOBJECT_ARENA.ctx, address, ARENA_SIZE);
			address = nullptr;
		}
	}
#endif
	if (address == nullptr) {



		arenaObj->nextArena = UNUSED_ARENA_OBJECTS;
		UNUSED_ARENA_OBJECTS = arenaObj;
		return nullptr;
	}
	arenaObj->address = (uintptr_t)address;

	++NARENAS_CURRENTLY_ALLOCATED;
	++NTIMES_ARENA_ALLOCATED;
	if (NARENAS_CURRENTLY_ALLOCATED > NARENAS_HIGHWATER)
		NARENAS_HIGHWATER = NARENAS_CURRENTLY_ALLOCATED;
	arenaObj->freePools = nullptr;


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



static bool address_in_range(OMState* state, void* p, PoolP ALIF_UNUSED(_pool))
{
	return arena_mapIs_used(state, (AlifMemBlock*)p);
}
#else












































































static bool ALIFNO_SANITIZE_ADDRESS
ALIFNO_SANITIZE_THREAD
ALIFNO_SANITIZE_MEMORY
address_in_range(OMState* _state, void* _p, poolp _pool)
{





	uint arenaIndex = *((volatile uint*)&_pool->arenaIndex);
	return arenaindex < maxArenas &&
		(uintptr_t)_p - ALLARENAS[arenaIndex].address < ARENA_SIZE &&
		ALLARENAS[arenaIndex].address != 0;
}

#endif 





static void alifMalloc_pool_extend(PoolP _pool, uint _size)
{
	if (UNLIKELY(_pool->nextOffset <= _pool->maxNextOffset)) {

		_pool->freeBlock = (AlifMemBlock*)_pool + _pool->nextOffset;
		_pool->nextOffset += INDEX2SIZE(_size);
		*(AlifMemBlock**)(_pool->freeBlock) = nullptr;
		return;
	}


	PoolP next;
	next = _pool->nextPool;
	_pool = _pool->prevPool;
	next->prevPool = _pool;
	_pool->nextPool = next;
}





static void* allocate_fromNew_pool(OMState* _state, uint _size)
{



	if (UNLIKELY(USABLE_ARENAS == nullptr)) {

#ifdef WITH_MEMORY_LIMITS
		if (NARENAS_CURRENTLY_ALLOCATED >= MAX_ARENAS) {
			return nullptr;
		}
#endif
		USABLE_ARENAS = new_arena(_state);
		if (USABLE_ARENAS == nullptr) {
			return nullptr;
		}
		USABLE_ARENAS->nextArena = USABLE_ARENAS->prevArena = nullptr;

		NFP2LASTA[USABLE_ARENAS->nFreePools] = USABLE_ARENAS;
	}








	if (NFP2LASTA[USABLE_ARENAS->nFreePools] == USABLE_ARENAS) {
		/* It's the last of this _size, so there won't be any. */
		NFP2LASTA[USABLE_ARENAS->nFreePools] = nullptr;
	}
	/* If any free pools will remain, it will be the new smallest. */
	if (USABLE_ARENAS->nFreePools > 1) {

		NFP2LASTA[USABLE_ARENAS->nFreePools - 1] = USABLE_ARENAS;
	}

	/* Try to get a cached free _pool. */
	PoolP pool = USABLE_ARENAS->freePools;
	if (LIKELY(pool != nullptr)) {
		/* Unlink from cached pools. */
		USABLE_ARENAS->freePools = pool->nextPool;
		USABLE_ARENAS->nFreePools--;
		if (UNLIKELY(USABLE_ARENAS->nFreePools == 0)) {
			/* Wholly allocated:  remove. */




			USABLE_ARENAS = USABLE_ARENAS->nextArena;
			if (USABLE_ARENAS != nullptr) {
				USABLE_ARENAS->prevArena = nullptr;

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
			if (USABLE_ARENAS != nullptr) {
				USABLE_ARENAS->prevArena = nullptr;

			}
		}
	}


	AlifMemBlock* bp;
	PoolP next = USEDPOOLS[_size + _size]; 
	pool->nextPool = next;
	pool->prevPool = next;
	next->nextPool = pool;
	next->prevPool = pool;
	pool->ref.count = 1;
	if (pool->szidx == _size) {




		bp = pool->freeBlock;

		pool->freeBlock = *(AlifMemBlock**)bp;
		return bp;
	}





	pool->szidx = _size;
	_size = INDEX2SIZE(_size);
	bp = (AlifMemBlock*)pool + POOL_OVERHEAD;
	pool->nextOffset = POOL_OVERHEAD + (_size << 1);
	pool->maxNextOffset = POOL_SIZE - _size;
	pool->freeBlock = bp + _size;
	*(AlifMemBlock**)(pool->freeBlock) = nullptr;
	return bp;
}











static inline void* alifMalloc_alloc(OMState* _state, void* ALIF_UNUSED(_ctx), size_t _nBytes)
{
#ifdef WITH_VALGRIND
	if (UNLIKELY(runningOnValgrind == -1)) {
		running_on_valgrind = RUNNING_ON_VALGRIND;
	}
	if (UNLIKELY(running_on_valgrind)) {
		return nullptr;
	}
#endif

	if (UNLIKELY(_nBytes == 0)) {
		return nullptr;
	}
	if (UNLIKELY(_nBytes > SMALL_REQUEST_THRESHOLD)) {
		return nullptr;
	}

	uint size = (uint)(_nBytes - 1) >> ALIGNMENT_SHIFT;
	PoolP pool = USEDPOOLS[size + size];
	AlifMemBlock* bp;

	if (LIKELY(pool != pool->nextPool)) {




		++pool->ref.count;
		bp = pool->freeBlock;


		if (UNLIKELY((pool->freeBlock = *(AlifMemBlock**)bp) == nullptr)) {

			alifMalloc_pool_extend(pool, size);
		}
	}
	else {



		bp = (AlifMemBlock*)allocate_fromNew_pool(_state, size);
	}

	return (void*)bp;
}



void* alifObject_malloc(void* ctx, size_t _nBytes)
{
	OMState* _state = get_state(); // هنا اضطررت لوضع _ قبل اسم المتغير ليتم التعرف عليه من قبل RAW_ALLOCATED_BLOCKS لان الاسم الممرر له _state
	void* ptr = alifMalloc_alloc(_state, ctx, _nBytes);
	if (LIKELY(ptr != nullptr)) {
		return ptr;
	}

	ptr = alifMem_rawMalloc(_nBytes);
	if (ptr != nullptr) {
		RAW_ALLOCATED_BLOCKS++;
	}
	return ptr;
}



void* alifObject_calloc(void* _ctx, size_t _nElem, size_t _elSize)
{

	size_t nBytes = _nElem * _elSize;

	OMState* _state = get_state();
	void* ptr = alifMalloc_alloc(_state, _ctx, nBytes);
	if (LIKELY(ptr != nullptr)) {
		memset(ptr, 0, nBytes);
		return ptr;
	}

	ptr = alifMem_rawCalloc(_nElem, _elSize);
	if (ptr != nullptr) {
		RAW_ALLOCATED_BLOCKS++;
	}
	return ptr;
}



static void insert_to_usedpool(OMState* _state, PoolP _pool)
{


	uint size = _pool->szidx;
	PoolP next = USEDPOOLS[size + size];
	PoolP prev = next->prevPool;


	_pool->nextPool = next;
	_pool->prevPool = prev;
	next->prevPool = _pool;
	prev->nextPool = _pool;
}


static void insert_to_freepool(OMState* _state, PoolP _pool)
{
	PoolP next = _pool->nextPool;
	PoolP prev = _pool->prevPool;
	next->prevPool = prev;
	prev->nextPool = next;




	ArenaObject* ao = &ALLARENAS[_pool->arenaIndex];
	_pool->nextPool = ao->freePools;
	ao->freePools = _pool;
	uint nf = ao->nFreePools;




	ArenaObject* lastnf = NFP2LASTA[nf];






	if (lastnf == ao) {  
		ArenaObject* p = ao->prevArena;
		NFP2LASTA[nf] = (p != nullptr && p->nFreePools == nf) ? p : nullptr;
	}
	ao->nFreePools = ++nf;

















	if (nf == ao->nTotalPools && ao->nextArena != nullptr) {










		if (ao->prevArena == nullptr) {
			USABLE_ARENAS = ao->nextArena;


		}














		ao->nextArena = UNUSED_ARENA_OBJECTS;
		UNUSED_ARENA_OBJECTS = ao;

#if WITH_ALIFMALLOC_RADIXTREE

		arena_mapMark_used(_state, ao->address, 0);
#endif


		ALIFOBJECT_ARENA.free(ALIFOBJECT_ARENA.ctx,
			(void*)ao->address, ARENA_SIZE);
		ao->address = 0;                        
		--NARENAS_CURRENTLY_ALLOCATED;

		return;
	}

	if (nf == 1) {





		ao->nextArena = USABLE_ARENAS;
		ao->prevArena = nullptr;
		if (USABLE_ARENAS)
			USABLE_ARENAS->prevArena = ao;
		USABLE_ARENAS = ao;

		if (NFP2LASTA[1] == nullptr) {
			NFP2LASTA[1] = ao;
		}

		return;
	}









	if (NFP2LASTA[nf] == nullptr) {
		NFP2LASTA[nf] = ao;
	} 

	if (ao == lastnf) {

		return;
	}










	if (ao->prevArena != nullptr) {


		ao->prevArena->nextArena = ao->nextArena;
	}
	else {


		USABLE_ARENAS = ao->nextArena;
	}
	ao->nextArena->prevArena = ao->prevArena;

	ao->prevArena = lastnf;
	ao->nextArena = lastnf->nextArena;
	if (ao->nextArena != nullptr) {
		ao->nextArena->prevArena = ao;
	}
	lastnf->nextArena = ao;






}






static inline int alifMalloc_free(OMState* _state, void* ALIF_UNUSED(_ctx), void* _p)
{


#ifdef WITH_VALGRIND
	if (UNLIKELY(runningOnValgrind > 0)) {
		return 0;
	}
#endif

	PoolP pool = POOL_ADDR(_p);
	if (UNLIKELY(!address_in_range(_state, _p, pool))) {
		return 0;
	}









	AlifMemBlock* lastFree = pool->freeBlock;
	*(AlifMemBlock**)_p = lastFree;
	pool->freeBlock = (AlifMemBlock*)_p;
	pool->ref.count--;

	if (UNLIKELY(lastFree == nullptr)) {






		insert_to_usedpool(_state, pool);
		return 1;
	}




	if (LIKELY(pool->ref.count != 0)) {

		return 1;
	}






	insert_to_freepool(_state, pool);
	return 1;
}



void alifObject_free(void* _ctx, void* _p)
{

	if (_p == nullptr) {
		return;
	}

	OMState* _state = get_state();
	if (UNLIKELY(!alifMalloc_free(_state, _ctx, _p))) {

		alifMem_rawFree(_p);
		RAW_ALLOCATED_BLOCKS--;
	}
}












static int alifMalloc_realloc(OMState* _state, void* _ctx,
	void** _newPtrP, void* _p, size_t _nBytes)
{
	void* bp;
	PoolP pool;
	size_t size;



#ifdef WITH_VALGRIND

	if (UNLIKELY(running_on_valgrind > 0)) {
		return 0;
	}
#endif

	pool = POOL_ADDR(_p);
	if (!address_in_range(_state, _p, pool)) {












		return 0;
	}


	size = INDEX2SIZE(pool->szidx);
	if (_nBytes <= size) {







		if (4 * _nBytes > 3 * size) {

			*_newPtrP = _p;
			return 1;
		}
		size = _nBytes;
	}

	bp = alifObject_malloc(_ctx, _nBytes);
	if (bp != nullptr) {
		memcpy(bp, _p, size);
		alifObject_free(_ctx, _p);
	}
	*_newPtrP = bp;
	return 1;
}



void* alifObject_realloc(void* _ctx, void* _ptr, size_t _nBytes)
{
	void* ptr2;

	if (_ptr == nullptr) {
		return alifObject_malloc(_ctx, _nBytes);
	}

	OMState* state = get_state();
	if (alifMalloc_realloc(state, _ctx, &ptr2, _ptr, _nBytes)) {
		return ptr2;
	}

	return alifMem_rawRealloc(_ptr, _nBytes);
}


#else   





AlifSizeT alifInterpreterState_getAllocatedBlocks(AlifInterpreterState* ALIF_UNUSED(_interp))
{
	return 0;
}

//AlifSizeT alifGetGlobalAllocatedBlocks()
//{
//	return 0;
//}

void alifInterpreterState_finalizeAllocatedBlocks(AlifInterpreterState* ALIF_UNUSED(_interp))
{
	return;
}

void alifFinalizeAllocatedBlocks(AlifRuntimeState* ALIF_UNUSED(_runtime))
{
	return;
}





#endif










#ifdef ALIFMEM_DEBUG_SERIALNO
static size_t serialno = 0;     




static void bumpserialno()
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



static size_t read_size_t(const void* _p)
{
	const uint8_t* q = (const uint8_t*)_p;
	size_t result = *q++;
	int i;

	for (i = SST; --i > 0; ++q)
		result = (result << 8) | *q;
	return result;
}





static void write_size_t(void* _p, size_t _n)
{
	uint8_t* q = (uint8_t*)_p + SST - 1;
	int i;

	for (i = SST; --i >= 0; --q) {
		*q = (uint8_t)(_n & 0xff);
		_n >>= 8;
	}
}































static void* alifMem_debugRawAlloc(int _useCalloc, void* _ctx, size_t _nBytes)
{
	DebugAllocApiT* api = (DebugAllocApiT*)_ctx;
	uint8_t* p;          
	uint8_t* data;       
	uint8_t* tail;       
	size_t total;        

	if (_nBytes > (size_t)ALIFSIZE_T_MAX - ALIFMEM_DEBUGEXTRA_BYTES) {

		return nullptr;
	}
	total = _nBytes + ALIFMEM_DEBUGEXTRA_BYTES;












	if (_useCalloc) {
		p = (uint8_t*)api->alloc.calloc(api->alloc.ctx, 1, total);
	}
	else {
		p = (uint8_t*)api->alloc.malloc(api->alloc.ctx, total);
	}
	if (p == nullptr) {
		return nullptr;
	}
	data = p + 2 * SST;

#ifdef ALIFMEM_DEBUG_SERIALNO
	bumpserialno();
#endif


	write_size_t(p, _nBytes);
	p[SST] = (uint8_t)api->apiID;
	memset(p + SST + 1, ALIFMEM_FORBIDDENBYTE, SST - 1);

	if (_nBytes > 0 && !_useCalloc) {
		memset(data, ALIFMEM_CLEANBYTE, _nBytes);
	}


	tail = data + _nBytes;
	memset(tail, ALIFMEM_FORBIDDENBYTE, SST);
#ifdef ALIFMEM_DEBUG_SERIALNO
	write_size_t(tail + SST, serialno);
#endif

	return data;
}


void* alifMem_debugRawMalloc(void* _ctx, size_t _nBytes)
{
	return alifMem_debugRawAlloc(0, _ctx, _nBytes);
}


void* alifMem_debugRawCalloc(void* _ctx, size_t _nElem, size_t _elSize)
{
	size_t nbytes;

	nbytes = _nElem * _elSize;
	return alifMem_debugRawAlloc(1, _ctx, nbytes);
}








void alifMem_debugRawFree(void* _ctx, void* _p)
{

	if (_p == nullptr) {
		return;
	}

	DebugAllocApiT* api = (DebugAllocApiT*)_ctx;
	uint8_t* q = (uint8_t*)_p - 2 * SST;  
	size_t nbytes;


	nbytes = read_size_t(q);
	nbytes += ALIFMEM_DEBUGEXTRA_BYTES;
	memset(q, ALIFMEM_DEADBYTE, nbytes);
	api->alloc.free(api->alloc.ctx, q);
}



void* alifMem_debugRawRealloc(void* _ctx, void* _p, size_t _nBytes)
{
	if (_p == nullptr) {
		return alifMem_debugRawAlloc(0, _ctx, _nBytes);
	}

	DebugAllocApiT* api = (DebugAllocApiT*)_ctx;
	uint8_t* head;        
	uint8_t* data;        
	uint8_t* r;
	uint8_t* tail;        
	size_t total;         
	size_t originalNbytes;
#define ERASED_SIZE 64
	uint8_t save[2 * ERASED_SIZE]; 



	data = (uint8_t*)_p;
	head = data - 2 * SST;
	originalNbytes = read_size_t(head);
	if (_nBytes > (size_t)ALIFSIZE_T_MAX - ALIFMEM_DEBUGEXTRA_BYTES) {

		return nullptr;
	}
	total = _nBytes + ALIFMEM_DEBUGEXTRA_BYTES;

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
	if (r == nullptr) {


		_nBytes = originalNbytes;
	}
	else {
		head = r;
#ifdef ALIFMEM_DEBUG_SERIALNO
		bumpserialno();
		blockSerialno = serialno;
#endif
	}
	data = head + 2 * SST;

	write_size_t(head, _nBytes);
	head[SST] = (uint8_t)api->apiID;
	memset(head + SST + 1, ALIFMEM_FORBIDDENBYTE, SST - 1);

	tail = data + _nBytes;
	memset(tail, ALIFMEM_FORBIDDENBYTE, SST);
#ifdef ALIFMEM_DEBUG_SERIALNO
	write_size_t(tail + SST, blockSerialno);
#endif


	if (originalNbytes <= sizeof(save)) {
		memcpy(data, save, ALIF_MIN(_nBytes, originalNbytes));
	}
	else {
		size_t i = originalNbytes - ERASED_SIZE;
		memcpy(data, save, ALIF_MIN(_nBytes, ERASED_SIZE));
		if (_nBytes > i) {
			memcpy(data + i, &save[ERASED_SIZE],
				ALIF_MIN(_nBytes - i, ERASED_SIZE));
		}
	}

	if (r == nullptr) {
		return nullptr;
	}

	if (_nBytes > originalNbytes) {

		memset(data + originalNbytes, ALIFMEM_CLEANBYTE,
			_nBytes - originalNbytes);
	}

	return data;
}












void* alifMem_debugMalloc(void* _ctx, size_t _nBytes)
{

	return alifMem_debugRawMalloc(_ctx, _nBytes);
}


void* alifMem_debugCalloc(void* _ctx, size_t _nElem, size_t _elSize)
{

	return alifMem_debugRawCalloc(_ctx, _nElem, _elSize);
}



void alifMem_debugFree(void* _ctx, void* _ptr)
{

	alifMem_debugRawFree(_ctx, _ptr);
}



void* alifMem_debugRealloc(void* _ctx, void* _ptr, size_t _nBytes)
{

	return alifMem_debugRawRealloc(_ctx, _ptr, _nBytes);
}
